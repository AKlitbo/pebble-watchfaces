/**
 * @file time_progress_calc.spec.c
 * @brief Host tests for the progress panel maths.
 *
 * A bar that reads 100% before the period is over, or that runs on the wrong length of year or
 * the wrong first day of the week, is wrong all day and looks fine in review. The boundaries
 * are what these tests pin down.
 *
 * Run from the repo root:
 *   cc -std=c11 -Wall -Wextra -Ilib/c/spec/unity -Iwatchfaces/mosaic/core/c/modules/time \
 *      watchfaces/mosaic/core/c/modules/time/time_progress_calc.spec.c \
 *      watchfaces/mosaic/core/c/modules/time/time_progress_calc.c lib/c/spec/unity/unity.c && ./a.out
 */
#include "unity.h"

#include "time_progress_calc.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief The very start of an hour reads nothing done. */
void test_hour_start_is_0(void)
{
    TEST_ASSERT_EQUAL_INT(0, progress_hour_pct(0, 0));
}

/** @brief Half past is halfway through the hour. */
void test_hour_half_past_is_50(void)
{
    TEST_ASSERT_EQUAL_INT(50, progress_hour_pct(30, 0));
}

/** @brief The last second of an hour must not round up to a finished 100. */
void test_hour_last_second_is_99(void)
{
    TEST_ASSERT_EQUAL_INT(99, progress_hour_pct(59, 59));
}

/** @brief Seconds count towards the hour, not only whole minutes. */
void test_hour_counts_seconds(void)
{
    // 14m 59s is 899s of 3600, which is 24.97 percent and floors to 24
    TEST_ASSERT_EQUAL_INT(24, progress_hour_pct(14, 59));
}

/** @brief Midnight is the start of the day. */
void test_day_midnight_is_0(void)
{
    TEST_ASSERT_EQUAL_INT(0, progress_day_pct(0, 0, 0));
}

/** @brief Six in the morning is a quarter of the day. */
void test_day_6am_is_25(void)
{
    TEST_ASSERT_EQUAL_INT(25, progress_day_pct(6, 0, 0));
}

/** @brief Noon is half of the day. */
void test_day_noon_is_50(void)
{
    TEST_ASSERT_EQUAL_INT(50, progress_day_pct(12, 0, 0));
}

/** @brief The last second of the day must not round up to 100. */
void test_day_last_second_is_99(void)
{
    TEST_ASSERT_EQUAL_INT(99, progress_day_pct(23, 59, 59));
}

/** @brief The first day of the week starts at zero, whichever day that is. */
void test_week_monday_start_on_monday_is_0(void)
{
    TEST_ASSERT_EQUAL_INT(0, progress_week_pct(1, 1, 0, 0, 0));
}

/** @brief Thursday noon is 3.5 of 7 days into a Monday-first week. */
void test_week_thursday_noon_monday_start_is_50(void)
{
    TEST_ASSERT_EQUAL_INT(50, progress_week_pct(4, 1, 12, 0, 0));
}

/** @brief Sunday is the last day of a Monday-first week, so it sits at the far end. */
void test_week_sunday_end_monday_start_is_99(void)
{
    TEST_ASSERT_EQUAL_INT(99, progress_week_pct(0, 1, 23, 59, 59));
}

/** @brief Sunday morning is the start of the week when weeks begin on Sunday. */
void test_week_sunday_start_on_sunday_is_0(void)
{
    TEST_ASSERT_EQUAL_INT(0, progress_week_pct(0, 0, 0, 0, 0));
}

/** @brief The same Sunday is the tail of the week when weeks begin on Monday. */
void test_week_sunday_midnight_monday_start_is_85(void)
{
    // six whole days of seven is 85.7 percent
    TEST_ASSERT_EQUAL_INT(85, progress_week_pct(0, 1, 0, 0, 0));
}

/** @brief Saturday noon in a Sunday-first week is 6.5 of 7 days. */
void test_week_saturday_noon_sunday_start_is_92(void)
{
    TEST_ASSERT_EQUAL_INT(92, progress_week_pct(6, 0, 12, 0, 0));
}

/** @brief The week day count wraps round so a Monday-first Sunday is day 7. */
void test_week_day_sunday_in_monday_week_is_7(void)
{
    TEST_ASSERT_EQUAL_INT(7, progress_week_day(0, 1));
}

/** @brief The week day count starts at 1 on the first day of the week. */
void test_week_day_first_day_is_1(void)
{
    TEST_ASSERT_EQUAL_INT(1, progress_week_day(0, 0));
    TEST_ASSERT_EQUAL_INT(1, progress_week_day(1, 1));
}

/** @brief New Year's Day at midnight is the start of the year. */
void test_year_jan1_midnight_is_0(void)
{
    TEST_ASSERT_EQUAL_INT(0, progress_year_pct(2026, 0, 0, 0, 0));
}

/** @brief Noon on 2 July is exactly half of a 365 day year. */
void test_year_jul2_noon_2026_is_50(void)
{
    // yday 182 plus half a day is 182.5 of 365
    TEST_ASSERT_EQUAL_INT(50, progress_year_pct(2026, 182, 12, 0, 0));
}

/** @brief A leap year is 366 days long, so the same date reads slightly less done. */
void test_year_jul2_noon_2024_is_49(void)
{
    // 182.5 of 366 is 49.86 percent
    TEST_ASSERT_EQUAL_INT(49, progress_year_pct(2024, 182, 12, 0, 0));
}

/** @brief The last second of the year must not round up to 100. */
void test_year_last_second_is_99(void)
{
    TEST_ASSERT_EQUAL_INT(99, progress_year_pct(2026, 364, 23, 59, 59));
    TEST_ASSERT_EQUAL_INT(99, progress_year_pct(2024, 365, 23, 59, 59));
}

/** @brief Century years are leap only when divisible by 400, so 2100 is short and 2000 is long. */
void test_year_century_leap_rule(void)
{
    // 182.5 days in: 182.5 of 365 is exactly 50, while 182.5 of 366 is 49.86
    TEST_ASSERT_EQUAL_INT(50, progress_year_pct(2100, 182, 12, 0, 0));
    TEST_ASSERT_EQUAL_INT(49, progress_year_pct(2000, 182, 12, 0, 0));
}

/** @brief A year of seconds times 100 must not overflow a 32 bit int on the way to a percent. */
void test_year_late_in_a_long_year_does_not_overflow(void)
{
    // 300 days of a 366 day year is 81.97 percent
    TEST_ASSERT_EQUAL_INT(81, progress_year_pct(2024, 300, 0, 0, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hour_start_is_0);
    RUN_TEST(test_hour_half_past_is_50);
    RUN_TEST(test_hour_last_second_is_99);
    RUN_TEST(test_hour_counts_seconds);
    RUN_TEST(test_day_midnight_is_0);
    RUN_TEST(test_day_6am_is_25);
    RUN_TEST(test_day_noon_is_50);
    RUN_TEST(test_day_last_second_is_99);
    RUN_TEST(test_week_monday_start_on_monday_is_0);
    RUN_TEST(test_week_thursday_noon_monday_start_is_50);
    RUN_TEST(test_week_sunday_end_monday_start_is_99);
    RUN_TEST(test_week_sunday_start_on_sunday_is_0);
    RUN_TEST(test_week_sunday_midnight_monday_start_is_85);
    RUN_TEST(test_week_saturday_noon_sunday_start_is_92);
    RUN_TEST(test_week_day_sunday_in_monday_week_is_7);
    RUN_TEST(test_week_day_first_day_is_1);
    RUN_TEST(test_year_jan1_midnight_is_0);
    RUN_TEST(test_year_jul2_noon_2026_is_50);
    RUN_TEST(test_year_jul2_noon_2024_is_49);
    RUN_TEST(test_year_last_second_is_99);
    RUN_TEST(test_year_century_leap_rule);
    RUN_TEST(test_year_late_in_a_long_year_does_not_overflow);
    return UNITY_END();
}
