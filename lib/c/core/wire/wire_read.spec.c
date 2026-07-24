/**
 * @file wire_read.spec.c
 * @brief Host tests for reading the values the phone packs into a message.
 *
 * This is the width table, and it is the densest patch of trouble in the codebase: three separate
 * findings landed in it, twice over, because the same fourteen lines were written out in two files
 * and only one copy ever got fixed. The bug each time was reading four bytes off a value that only
 * had one, and picking up whatever was packed behind it.
 *
 * So every width is pinned at both signs, and so is the refusal of a width no sender may use. The
 * buffers below carry a sentinel behind the value on purpose: a reader that overruns lands on it,
 * which is exactly what the watch would do to the next value along.
 */
#include "unity.h"

#include "wire/wire_read.h"

void setUp(void) {}
void tearDown(void) {}

// a value followed by bytes that are not part of it. a reader that takes more than the width says
// picks these up, the same way it would pick up the next value in a real message
#define TRAILING 0xAA, 0xBB, 0xCC, 0xDD

/**
 * @brief One byte read signed, with rubbish behind it that must not be read.
 *
 * This is the exact shape of the finding: a temperature of 23 sent as a single byte. Take four
 * bytes and the answer is millions, which the display then clamps to something almost plausible.
 */
void test_one_byte_signed_stops_at_one_byte(void)
{
    const uint8_t bytes[] = { 23, TRAILING };

    int32_t result = wire_int_or(true, 1, bytes, -1);

    TEST_ASSERT_EQUAL_INT32(23, result);
}

/** @brief A byte's top bit is the sign when it is signed, so 0xFF is -1 and not 255. */
void test_one_byte_signed_reads_negative(void)
{
    const uint8_t bytes[] = { 0xFF, TRAILING };

    int32_t result = wire_int_or(true, 1, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(-1, result);
}

/** @brief The same byte unsigned is 255, which is the only thing the signedness decides here. */
void test_one_byte_unsigned_reads_the_top_of_the_range(void)
{
    const uint8_t bytes[] = { 0xFF, TRAILING };

    int32_t result = wire_int_or(false, 1, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(255, result);
}

/** @brief Two bytes signed, low first, with rubbish behind that must not be read. */
void test_two_bytes_signed_stops_at_two_bytes(void)
{
    const uint8_t bytes[] = { 0x83, 0xFF, TRAILING };  // -125

    int32_t result = wire_int_or(true, 2, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(-125, result);
}

/**
 * @brief Two bytes unsigned must land on 0 to 65535 and not go negative at the top.
 *
 * The pair is read signed and the sign taken back off, so this is the case that catches the
 * taking-off going missing.
 */
void test_two_bytes_unsigned_does_not_go_negative(void)
{
    const uint8_t bytes[] = { 0xFF, 0xFF, TRAILING };

    int32_t result = wire_int_or(false, 2, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(65535, result);
}

/** @brief Four bytes signed is the everyday case and fills the whole width. */
void test_four_bytes_signed(void)
{
    const uint8_t bytes[] = { 0x9C, 0xFF, 0xFF, 0xFF };  // -100

    int32_t result = wire_int_or(true, 4, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(-100, result);
}

/** @brief Four bytes fill the width either way so the bits come back the same. */
void test_four_bytes_unsigned_keeps_the_bits(void)
{
    const uint8_t bytes[] = { 0xFF, 0xFF, 0xFF, 0xFF };

    int32_t result = wire_int_or(false, 4, bytes, 0);

    TEST_ASSERT_EQUAL_INT32(-1, result);
}

/**
 * @brief A width no sender may use is refused rather than guessed at.
 *
 * Three bytes is not a width the phone can send. Reading it anyway means picking a size, and every
 * size is wrong, so the caller keeps what it had instead.
 */
void test_an_odd_width_is_refused(void)
{
    const uint8_t bytes[] = { 1, 2, 3, TRAILING };

    int32_t result = wire_int_or(true, 3, bytes, -999);

    TEST_ASSERT_EQUAL_INT32(-999, result);
}

/** @brief A width of zero carries no value at all, so there is nothing to read. */
void test_a_zero_width_is_refused(void)
{
    const uint8_t bytes[] = { 1, TRAILING };

    int32_t result = wire_int_or(true, 0, bytes, -999);

    TEST_ASSERT_EQUAL_INT32(-999, result);
}

/** @brief No value means the fallback, so a caller may pass one straight through. */
void test_no_value_is_refused(void)
{
    int32_t result = wire_int_or(true, 4, NULL, -999);

    TEST_ASSERT_EQUAL_INT32(-999, result);
}

/** @brief A terminated string is safe to read, the case everything else is measured against. */
void test_a_terminated_string_is_readable(void)
{
    const char s[] = "SUNNY";

    bool result = wire_cstring_terminated(s, sizeof(s));  // 6, terminator included

    TEST_ASSERT_TRUE(result);
}

/**
 * @brief A string whose terminator is not inside its own length must be refused.
 *
 * Reading it runs on past the end hunting a zero byte, through whatever the sender packed behind
 * it, or off the end of the message entirely.
 */
void test_an_unterminated_string_is_refused(void)
{
    const char s[] = { 'S', 'U', 'N', 'N' };

    bool result = wire_cstring_terminated(s, 4);

    TEST_ASSERT_FALSE(result);
}

/** @brief The terminator has to be the last byte and not merely somewhere about. */
void test_a_terminator_past_the_length_is_refused(void)
{
    const char s[] = { 'S', 'U', 'N', '\0' };

    bool result = wire_cstring_terminated(s, 3);  // the length stops before the terminator

    TEST_ASSERT_FALSE(result);
}

/** @brief A length of zero carries nothing, terminator included, so there is nothing to read. */
void test_an_empty_string_is_refused(void)
{
    const char s[] = "";

    bool result = wire_cstring_terminated(s, 0);

    TEST_ASSERT_FALSE(result);
}

/** @brief A string of nothing but a terminator is a real empty string and is readable. */
void test_a_string_of_just_a_terminator_is_readable(void)
{
    const char s[] = "";

    bool result = wire_cstring_terminated(s, 1);

    TEST_ASSERT_TRUE(result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_one_byte_signed_stops_at_one_byte);
    RUN_TEST(test_one_byte_signed_reads_negative);
    RUN_TEST(test_one_byte_unsigned_reads_the_top_of_the_range);
    RUN_TEST(test_two_bytes_signed_stops_at_two_bytes);
    RUN_TEST(test_two_bytes_unsigned_does_not_go_negative);
    RUN_TEST(test_four_bytes_signed);
    RUN_TEST(test_four_bytes_unsigned_keeps_the_bits);
    RUN_TEST(test_an_odd_width_is_refused);
    RUN_TEST(test_a_zero_width_is_refused);
    RUN_TEST(test_no_value_is_refused);
    RUN_TEST(test_a_terminated_string_is_readable);
    RUN_TEST(test_an_unterminated_string_is_refused);
    RUN_TEST(test_a_terminator_past_the_length_is_refused);
    RUN_TEST(test_an_empty_string_is_refused);
    RUN_TEST(test_a_string_of_just_a_terminator_is_readable);

    return UNITY_END();
}
