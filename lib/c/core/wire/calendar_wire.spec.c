/**
 * @file calendar_wire.spec.c
 * @brief Host tests for unpacking the agenda off the wire.
 *
 * Untrusted bytes going into fixed buffers, with every length in the run being the phone's word
 * rather than ours. The three things worth pinning: a count past the array is pinned and does not
 * walk off the end, a length that reaches past the message is refused, and a message that stops
 * halfway leaves the caller's agenda alone instead of half replacing it.
 */
#include "unity.h"

#include "wire/calendar_wire.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static uint16_t put_u32(uint8_t *buffer, uint16_t offset, int32_t value)
{
    buffer[offset++] = value & 0xFF;
    buffer[offset++] = (value >> 8) & 0xFF;
    buffer[offset++] = (value >> 16) & 0xFF;
    buffer[offset++] = (value >> 24) & 0xFF;
    return offset;
}

// build one wire event: [start i32 LE][end i32 LE][flags][titleLen][title][locLen][loc]
static uint16_t put_event(uint8_t *buffer, uint16_t offset, int32_t start, int32_t end,
                          bool all_day, const char *title, const char *location)
{
    offset = put_u32(buffer, offset, start);
    offset = put_u32(buffer, offset, end);
    buffer[offset++] = all_day ? 1 : 0;

    uint8_t title_len = (uint8_t)strlen(title);
    buffer[offset++] = title_len;
    memcpy(buffer + offset, title, title_len);
    offset += title_len;

    uint8_t loc_len = (uint8_t)strlen(location);
    buffer[offset++] = loc_len;
    memcpy(buffer + offset, location, loc_len);
    offset += loc_len;
    return offset;
}

/** @brief A clean event must unpack fully or the agenda shows a blank row. */
void test_reads_a_clean_event(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_event(buffer, len, 1700000000, 1700003600, false, "Standup", "Room 4");

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(1, out.count);
    TEST_ASSERT_EQUAL_INT64(1700000000, out.event[0].start);
    TEST_ASSERT_EQUAL_INT64(1700003600, out.event[0].end);
    TEST_ASSERT_FALSE(out.event[0].all_day);
    TEST_ASSERT_EQUAL_STRING("Standup", out.event[0].title);
    TEST_ASSERT_EQUAL_STRING("Room 4", out.event[0].location);
}

/** @brief The all-day flag rides bit 0, and a set bit must read as set. */
void test_reads_the_all_day_flag(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_event(buffer, len, 1700000000, 1700086400, true, "Leave", "");

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(out.event[0].all_day);
}

/**
 * @brief A count past what the strip holds must be pinned to it.
 *
 * The count byte is the phone's word for how many events follow, and the array holds six. Without
 * the pin the fill walks off the end of it and into whatever the caller keeps next door.
 *
 * The buffer has to carry a full strip of real events to catch this. Stop the bytes short and the
 * reader refuses on the bounds check whether or not the count was pinned, which looks like a pass
 * and proves nothing.
 */
void test_pins_a_count_past_the_array(void)
{
    uint8_t buffer[256];
    uint16_t len = 0;
    buffer[len++] = 200;
    for (int i = 0; i < CALENDAR_MAX_SLOTS; i++)
    {
        char title[8];
        snprintf(title, sizeof(title), "E%d", i);
        len = put_event(buffer, len, 1700000000 + i, 1700003600 + i, false, title, "");
    }

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(CALENDAR_MAX_SLOTS, out.count);
    TEST_ASSERT_EQUAL_STRING("E5", out.event[CALENDAR_MAX_SLOTS - 1].title);
}

/** @brief A message that stops inside an event's fixed part is refused. */
void test_refuses_a_message_that_stops_mid_event(void)
{
    // claims two events but the bytes stop inside the first one's header
    uint8_t buffer[6] = { 2, 0x00, 0x11, 0x22, 0x33, 0x44 };

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_FALSE(result);
}

/** @brief A title reaching past the message is refused rather than read out of bounds. */
void test_refuses_a_title_past_the_message(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_u32(buffer, len, 1700000000);
    len = put_u32(buffer, len, 1700003600);
    buffer[len++] = 0;   // flags
    buffer[len++] = 40;  // titleLen lies: 40 bytes claimed
    buffer[len++] = 'H';
    buffer[len++] = 'i';

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/** @brief A location reaching past the message is refused the same way a title is. */
void test_refuses_a_location_past_the_message(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_u32(buffer, len, 1700000000);
    len = put_u32(buffer, len, 1700003600);
    buffer[len++] = 0;   // flags
    buffer[len++] = 2;   // titleLen
    buffer[len++] = 'H';
    buffer[len++] = 'i';
    buffer[len++] = 20;  // locLen lies: 20 bytes claimed
    buffer[len++] = 'X';
    buffer[len++] = 'Y';

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief A refused message must leave the caller's agenda untouched.
 *
 * This is what lets the store keep its last good reading. Half filling the caller's strip and then
 * refusing would blank the panel on a bad message, which is the thing the whole build-then-commit
 * shape exists to stop.
 */
void test_leaves_the_agenda_alone_when_it_refuses(void)
{
    CalendarStrip out = {0};
    out.count = 3;
    strcpy(out.event[0].title, "Keep me");

    uint8_t buffer[6] = { 2, 0x00, 0x11, 0x22, 0x33, 0x44 };
    bool result = calendar_wire_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_UINT8(3, out.count);
    TEST_ASSERT_EQUAL_STRING("Keep me", out.event[0].title);
}

/** @brief An over-long title takes what fits and stays terminated rather than running over. */
void test_truncates_an_overlong_title(void)
{
    uint8_t buffer[128];
    uint16_t len = 0;
    buffer[len++] = 1;
    len = put_event(buffer, len, 1700000000, 1700003600, false,
                    "A title far longer than the buffer has room for", "");

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT(CAL_TITLE_LEN - 1, strlen(out.event[0].title));
}

/** @brief An empty agenda is a real reading: the phone saying there is nothing on. */
void test_reads_an_empty_agenda(void)
{
    uint8_t buffer[1] = { 0 };

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(0, out.count);
}

/** @brief No bytes at all is not an empty agenda, it is no message, so it is refused. */
void test_refuses_an_empty_message(void)
{
    uint8_t buffer[1] = { 0 };

    CalendarStrip out;
    bool result = calendar_wire_decode(buffer, 0, &out);

    TEST_ASSERT_FALSE(result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_reads_a_clean_event);
    RUN_TEST(test_reads_the_all_day_flag);
    RUN_TEST(test_pins_a_count_past_the_array);
    RUN_TEST(test_refuses_a_message_that_stops_mid_event);
    RUN_TEST(test_refuses_a_title_past_the_message);
    RUN_TEST(test_refuses_a_location_past_the_message);
    RUN_TEST(test_leaves_the_agenda_alone_when_it_refuses);
    RUN_TEST(test_truncates_an_overlong_title);
    RUN_TEST(test_reads_an_empty_agenda);
    RUN_TEST(test_refuses_an_empty_message);

    return UNITY_END();
}
