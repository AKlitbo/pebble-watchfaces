/**
 * @file weather_wire.spec.c
 * @brief Host tests for unpacking the forecast strips off the wire.
 *
 * Untrusted bytes going into fixed arrays. These two fill the caller's strip as they go rather
 * than handing over a finished copy, which is only safe because the one length check up top
 * settles the whole run, so that check is what most of this pins. The rest is the count being
 * held to what the array holds, and the caller's strip surviving a message that gets turned away.
 */
#include "unity.h"

#include "wire/weather_wire.h"

void setUp(void) {}
void tearDown(void) {}

// build one hourly column: [code][temp int16 LE]
static uint16_t put_hour(uint8_t *buffer, uint16_t offset, uint8_t code, int16_t temp)
{
    buffer[offset++] = code;
    buffer[offset++] = temp & 0xFF;
    buffer[offset++] = (temp >> 8) & 0xFF;
    return offset;
}

// build one daily column: [code][max int16 LE][min int16 LE]
static uint16_t put_day(uint8_t *buffer, uint16_t offset, uint8_t code, int16_t hi, int16_t lo)
{
    buffer[offset++] = code;
    buffer[offset++] = hi & 0xFF;
    buffer[offset++] = (hi >> 8) & 0xFF;
    buffer[offset++] = lo & 0xFF;
    buffer[offset++] = (lo >> 8) & 0xFF;
    return offset;
}

/** @brief A clean hourly strip must unpack whole or the forecast row draws blanks. */
void test_reads_a_clean_hourly_strip(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 2;   // count
    buffer[len++] = 14;  // base_hour
    buffer[len++] = 3;   // step_hours
    len = put_hour(buffer, len, 1, 21);
    len = put_hour(buffer, len, 7, 19);

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(2, out.count);
    TEST_ASSERT_EQUAL_UINT8(14, out.base_hour);
    TEST_ASSERT_EQUAL_UINT8(3, out.step_hours);
    TEST_ASSERT_EQUAL_UINT8(1, out.col[0].code);
    TEST_ASSERT_EQUAL_INT16(21, out.col[0].temp);
    TEST_ASSERT_EQUAL_INT16(19, out.col[1].temp);
}

/** @brief A temperature below zero must stay below zero and not read as a large positive. */
void test_reads_a_negative_hourly_temp(void)
{
    uint8_t buffer[16];
    uint16_t len = 0;
    buffer[len++] = 1;
    buffer[len++] = 0;
    buffer[len++] = 1;
    len = put_hour(buffer, len, 3, -12);

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT16(-12, out.col[0].temp);
}

/** @brief A count past what the strip holds must be pinned to it, or the fill runs off the end. */
void test_pins_an_hourly_count_past_the_array(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 200;
    buffer[len++] = 0;
    buffer[len++] = 1;
    for (int i = 0; i < WEATHER_FORECAST_COLS; i++)
    {
        len = put_hour(buffer, len, (uint8_t)i, (int16_t)(10 + i));
    }

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(WEATHER_FORECAST_COLS, out.count);
    TEST_ASSERT_EQUAL_INT16(10 + WEATHER_FORECAST_COLS - 1, out.col[WEATHER_FORECAST_COLS - 1].temp);
}

/**
 * @brief A strip claiming more columns than it carries is refused whole.
 *
 * This is the check the whole shape rests on. Fill-as-you-go is only safe because nothing past
 * here can come up short, so a message that lies about its count has to be caught right here or
 * the loop reads off the end of the message.
 */
void test_refuses_an_hourly_strip_that_is_short(void)
{
    uint8_t buffer[16];
    uint16_t len = 0;
    buffer[len++] = 4;  // says four columns
    buffer[len++] = 0;
    buffer[len++] = 1;
    len = put_hour(buffer, len, 1, 21);  // sends one

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/** @brief A message too short to even hold the header is refused. */
void test_refuses_an_hourly_message_with_no_header(void)
{
    uint8_t buffer[2] = { 1, 0 };

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief A refused message must leave the caller's strip untouched.
 *
 * The store hands its own live strip straight in, so anything written before a refusal lands on
 * the row the watch is showing. Everything that can turn a message away happens before the first
 * byte is written, and this is what says so.
 */
void test_leaves_the_hourly_strip_alone_when_it_refuses(void)
{
    WeatherHourly out = {0};
    out.count = 5;
    out.base_hour = 9;
    out.col[0].temp = 17;

    uint8_t buffer[8];
    uint16_t len = 0;
    buffer[len++] = 4;  // lies
    buffer[len++] = 22;
    buffer[len++] = 1;
    len = put_hour(buffer, len, 1, 99);

    bool result = weather_hourly_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_UINT8(5, out.count);
    TEST_ASSERT_EQUAL_UINT8(9, out.base_hour);
    TEST_ASSERT_EQUAL_INT16(17, out.col[0].temp);
}

/** @brief An empty hourly strip is a real reading: the phone has no forecast to give. */
void test_reads_an_empty_hourly_strip(void)
{
    uint8_t buffer[3] = { 0, 0, 0 };

    WeatherHourly out;
    bool result = weather_hourly_decode(buffer, sizeof(buffer), &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(0, out.count);
}

/** @brief A clean daily strip must unpack whole, highs and lows both. */
void test_reads_a_clean_daily_strip(void)
{
    uint8_t buffer[32];
    uint16_t len = 0;
    buffer[len++] = 2;  // count
    buffer[len++] = 3;  // base_weekday
    len = put_day(buffer, len, 1, 24, 12);
    len = put_day(buffer, len, 7, 19, 8);

    WeatherDaily out;
    bool result = weather_daily_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(2, out.count);
    TEST_ASSERT_EQUAL_UINT8(3, out.base_weekday);
    TEST_ASSERT_EQUAL_INT16(24, out.col[0].temp_max);
    TEST_ASSERT_EQUAL_INT16(12, out.col[0].temp_min);
    TEST_ASSERT_EQUAL_INT16(8, out.col[1].temp_min);
}

/** @brief The high and the low must not swap, which a spec is the only way to notice. */
void test_reads_a_daily_low_below_zero(void)
{
    uint8_t buffer[16];
    uint16_t len = 0;
    buffer[len++] = 1;
    buffer[len++] = 0;
    len = put_day(buffer, len, 1, 3, -9);

    WeatherDaily out;
    bool result = weather_daily_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT16(3, out.col[0].temp_max);
    TEST_ASSERT_EQUAL_INT16(-9, out.col[0].temp_min);
}

/** @brief A count past what the strip holds must be pinned to it. */
void test_pins_a_daily_count_past_the_array(void)
{
    uint8_t buffer[64];
    uint16_t len = 0;
    buffer[len++] = 200;
    buffer[len++] = 0;
    for (int i = 0; i < WEATHER_FORECAST_COLS; i++)
    {
        len = put_day(buffer, len, (uint8_t)i, (int16_t)(20 + i), (int16_t)i);
    }

    WeatherDaily out;
    bool result = weather_daily_decode(buffer, len, &out);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(WEATHER_FORECAST_COLS, out.count);
}

/** @brief A daily strip claiming more days than it carries is refused whole. */
void test_refuses_a_daily_strip_that_is_short(void)
{
    uint8_t buffer[16];
    uint16_t len = 0;
    buffer[len++] = 3;  // says three
    buffer[len++] = 0;
    len = put_day(buffer, len, 1, 24, 12);  // sends one

    WeatherDaily out;
    bool result = weather_daily_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
}

/** @brief A refused daily message must leave the caller's strip untouched, same as the hourly. */
void test_leaves_the_daily_strip_alone_when_it_refuses(void)
{
    WeatherDaily out = {0};
    out.count = 7;
    out.col[0].temp_max = 30;

    uint8_t buffer[8];
    uint16_t len = 0;
    buffer[len++] = 3;  // lies
    buffer[len++] = 1;
    len = put_day(buffer, len, 1, 99, 99);

    bool result = weather_daily_decode(buffer, len, &out);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_UINT8(7, out.count);
    TEST_ASSERT_EQUAL_INT16(30, out.col[0].temp_max);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_reads_a_clean_hourly_strip);
    RUN_TEST(test_reads_a_negative_hourly_temp);
    RUN_TEST(test_pins_an_hourly_count_past_the_array);
    RUN_TEST(test_refuses_an_hourly_strip_that_is_short);
    RUN_TEST(test_refuses_an_hourly_message_with_no_header);
    RUN_TEST(test_leaves_the_hourly_strip_alone_when_it_refuses);
    RUN_TEST(test_reads_an_empty_hourly_strip);
    RUN_TEST(test_reads_a_clean_daily_strip);
    RUN_TEST(test_reads_a_daily_low_below_zero);
    RUN_TEST(test_pins_a_daily_count_past_the_array);
    RUN_TEST(test_refuses_a_daily_strip_that_is_short);
    RUN_TEST(test_leaves_the_daily_strip_alone_when_it_refuses);

    return UNITY_END();
}
