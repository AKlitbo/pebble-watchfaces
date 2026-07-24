/**
 * @file clockstr.spec.c
 * @brief Host tests for the "HH:MM" reader.
 *
 * The phone sends sunrise and sunset as text, so this is where a string the phone chose becomes an
 * hour the face draws. Getting it wrong shows up as a sun panel counting down to the wrong moment,
 * or as "--" where a time should be. The phone is not trusted, so what it refuses matters as much
 * as what it reads.
 */
#include "unity.h"

#include "clock/clockstr.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief The everyday case, the shape the phone actually sends. */
void test_reads_a_plain_time(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("06:30", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(6, h);
    TEST_ASSERT_EQUAL_INT(30, m);
}

/** @brief Midnight is a real time, and a zero hour must not read as an empty one. */
void test_reads_midnight(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("00:00", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(0, h);
    TEST_ASSERT_EQUAL_INT(0, m);
}

/** @brief The last minute of the day is in range, so the top of the range must not be off by one. */
void test_reads_the_last_minute_of_the_day(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("23:59", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(23, h);
    TEST_ASSERT_EQUAL_INT(59, m);
}

/** @brief A single digit hour reads fine, which is worth knowing since the phone pads and this does not care. */
void test_accepts_a_one_digit_hour(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("6:30", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(6, h);
    TEST_ASSERT_EQUAL_INT(30, m);
}

/**
 * @brief Only the two digits after the colon are the minute and the rest is ignored.
 *
 * This is the surprising one. Nothing checks what follows the minute, so a longer string is read
 * rather than refused. That is what lets a "06:30:00" through unbothered, and it is also why
 * "12:305" is half twelve rather than nonsense.
 */
void test_ignores_anything_past_the_minute(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("12:305", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(12, h);
    TEST_ASSERT_EQUAL_INT(30, m);
}

/**
 * @brief A run of digits where the hour goes must be refused on sight and not added up.
 *
 * Adding them up is the problem. The 0-23 test cannot save a number that has already overflowed
 * getting there, and a signed overflow is undefined rather than merely a wrong hour. Every caller
 * on this face hands over a buffer of eight bytes at most, which is what keeps it out of reach,
 * but that bound belongs to the caller and does not travel with the function.
 *
 * @note This one documents the case and does not pin it. Take the digit bound away and it still
 *   passes, because the overflow it then walks into is undefined and lands, this time, on
 *   something the 0-23 test happens to refuse. That is the nature of the thing: undefined
 *   behaviour cannot be tested, only avoided. test_refuses_a_padded_hour is the one that fails
 *   when the bound goes.
 */
void test_refuses_an_hour_of_many_digits(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("99999999999999999999:30", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief Two digits is a real hour, so the bound must not refuse the everyday case. */
void test_accepts_a_two_digit_hour(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("23:30", &h, &m);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(23, h);
}

/**
 * @brief Padding an hour out with zeroes is not a time the phone sends, so it is refused.
 *
 * This is what actually holds the digit bound in place. Three digits that still add up to a real
 * hour walk straight past the 0-23 test, so without the bound this reads as half six and the bound
 * has nothing else stopping it from being deleted as dead weight.
 */
void test_refuses_a_padded_hour(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("006:30", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief An hour past the clock is not a time, whatever the digits say. */
void test_refuses_an_hour_out_of_range(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("24:00", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief Sixty minutes is the next hour and not a minute, so it must be refused. */
void test_refuses_a_minute_out_of_range(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("12:60", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief A missing hour is not a midnight, or an empty reading would draw as 00:30. */
void test_refuses_a_missing_hour(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse(":30", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief The minute needs both its digits, so a short tail is refused rather than read as 3. */
void test_refuses_a_one_digit_minute(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("12:3", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief No colon means no time. */
void test_refuses_a_string_with_no_colon(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("1230", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief An empty string is the phone saying it has nothing, and must read as nothing. */
void test_refuses_an_empty_string(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief Letters where digits belong are refused rather than read as their byte values. */
void test_refuses_letters_in_the_hour(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse("ab:30", &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief A refused string must leave the caller's values alone rather than half write them. */
void test_leaves_the_outputs_alone_when_it_refuses(void)
{
    int h = 7;
    int m = 7;

    bool result = clockstr_parse("nonsense", &h, &m);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(7, h);
    TEST_ASSERT_EQUAL_INT(7, m);
}

/** @brief A null string is the phone saying nothing, and must read as nothing rather than crash. */
void test_refuses_a_null_string(void)
{
    int h = -1;
    int m = -1;

    bool result = clockstr_parse(NULL, &h, &m);

    TEST_ASSERT_FALSE(result);
}

/** @brief The minutes form folds the same read into one number: 06:30 is 390 past midnight. */
void test_minutes_reads_a_plain_time(void)
{
    int result = clockstr_minutes("06:30");

    TEST_ASSERT_EQUAL_INT(390, result);
}

/** @brief Midnight is zero minutes, not the -1 that means nothing. */
void test_minutes_reads_midnight_as_zero(void)
{
    int result = clockstr_minutes("00:00");

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A string that does not read as a time gives -1 for the caller to treat as no data. */
void test_minutes_refuses_with_negative_one(void)
{
    int result = clockstr_minutes("");

    TEST_ASSERT_EQUAL_INT(-1, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_reads_a_plain_time);
    RUN_TEST(test_reads_midnight);
    RUN_TEST(test_reads_the_last_minute_of_the_day);
    RUN_TEST(test_accepts_a_one_digit_hour);
    RUN_TEST(test_accepts_a_two_digit_hour);
    RUN_TEST(test_ignores_anything_past_the_minute);
    RUN_TEST(test_refuses_an_hour_of_many_digits);
    RUN_TEST(test_refuses_a_padded_hour);
    RUN_TEST(test_refuses_an_hour_out_of_range);
    RUN_TEST(test_refuses_a_minute_out_of_range);
    RUN_TEST(test_refuses_a_missing_hour);
    RUN_TEST(test_refuses_a_one_digit_minute);
    RUN_TEST(test_refuses_a_string_with_no_colon);
    RUN_TEST(test_refuses_an_empty_string);
    RUN_TEST(test_refuses_a_null_string);
    RUN_TEST(test_refuses_letters_in_the_hour);
    RUN_TEST(test_leaves_the_outputs_alone_when_it_refuses);
    RUN_TEST(test_minutes_reads_a_plain_time);
    RUN_TEST(test_minutes_reads_midnight_as_zero);
    RUN_TEST(test_minutes_refuses_with_negative_one);

    return UNITY_END();
}
