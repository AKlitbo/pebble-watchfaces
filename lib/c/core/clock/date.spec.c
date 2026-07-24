/**
 * @file date.spec.c
 * @brief Host tests for the pure calendar math.
 *
 * The week number and month shapes drive the month grid and the little date
 * novelty panels. A wrong leap rule or a bad year boundary shows the wrong
 * number all day, so the boundaries are what these tests pin down.
 */
#include "unity.h"

#include "clock/date.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A year opening on a Thursday must read week 1 or the whole year runs a week late. */
void test_iso_week_jan1_on_thursday_is_week_1(void)
{
    // 2026-01-01 is a Thursday, yday 0
    int result = date_iso_week(2026, 0, 4);

    TEST_ASSERT_EQUAL_INT(1, result);
}

/** @brief A year opening on a Friday still belongs to last year's final week, not week 0. */
void test_iso_week_jan1_on_friday_is_last_years_week_53(void)
{
    // 2027-01-01 is a Friday, yday 0
    int result = date_iso_week(2027, 0, 5);

    TEST_ASSERT_EQUAL_INT(53, result);
}

/** @brief The last Monday of a 52-week year already belongs to next year's week 1. */
void test_iso_week_dec30_2024_is_week_1(void)
{
    // 2024-12-30 is a Monday, yday 364
    int result = date_iso_week(2024, 364, 1);

    TEST_ASSERT_EQUAL_INT(1, result);
}

/** @brief New Year's Day after a 53-week year must read 53, not clamp to 52. */
void test_iso_week_jan1_2021_is_week_53(void)
{
    // 2021-01-01 is a Friday, yday 0
    int result = date_iso_week(2021, 0, 5);

    TEST_ASSERT_EQUAL_INT(53, result);
}

/** @brief A plain mid-year Monday keeps the everyday case honest. */
void test_iso_week_mid_year_monday(void)
{
    // 2026-07-13 is a Monday, yday 193
    int result = date_iso_week(2026, 193, 1);

    TEST_ASSERT_EQUAL_INT(29, result);
}

/**
 * @brief The last week of a 53-week year must read 53, not wrap to 1.
 *
 * The clamp only borrows next year's week 1 once the week runs PAST the year's own total, so a
 * week landing exactly on the total has to stay put. A > slipping to >= passes every other test
 * here and only shows up on New Year's Eve.
 */
void test_iso_week_dec31_2026_is_week_53(void)
{
    // 2026-12-31 is a Thursday, yday 364, in a year that runs 53 weeks
    int result = date_iso_week(2026, 364, 4);

    TEST_ASSERT_EQUAL_INT(53, result);
}

/** @brief A mid-year day belongs to its own year, the everyday case for the week total. */
void test_iso_week_year_mid_year_is_same_year(void)
{
    // 2026-07-13 is a Monday, yday 193
    int result = date_iso_week_year(2026, 193, 1);

    TEST_ASSERT_EQUAL_INT(2026, result);
}

/**
 * @brief New Year's Day borrowing last year's week 53 must report last year.
 *
 * The week tile totals against this year, so if it reported 2021 here the panel would read
 * "53/52" against 2021's 52 weeks.
 */
void test_iso_week_year_jan1_2021_is_previous_year(void)
{
    // 2021-01-01 is a Friday, yday 0, and belongs to 2020's week 53
    int result = date_iso_week_year(2021, 0, 5);

    TEST_ASSERT_EQUAL_INT(2020, result);
}

/** @brief Late December already in next year's week 1 must report next year. */
void test_iso_week_year_dec30_2024_is_next_year(void)
{
    // 2024-12-30 is a Monday, yday 364, and belongs to 2025's week 1
    int result = date_iso_week_year(2024, 364, 1);

    TEST_ASSERT_EQUAL_INT(2025, result);
}

/**
 * @brief The week and its total must come from the same ISO year.
 *
 * This is the whole point of the pairing: the number can never exceed its own total.
 */
void test_iso_week_never_exceeds_its_own_years_total(void)
{
    // 2021-01-01, where the week comes from 2020 so the total has to come from 2020 too
    int week  = date_iso_week(2021, 0, 5);
    int total = date_iso_weeks_in_year(date_iso_week_year(2021, 0, 5));

    TEST_ASSERT_EQUAL_INT(53, week);
    TEST_ASSERT_EQUAL_INT(53, total);
    TEST_ASSERT_TRUE(week <= total);
}

/** @brief A year opening on a Thursday runs 53 ISO weeks, so the week tile's total must read 53. */
void test_iso_weeks_in_year_thursday_start_is_53(void)
{
    int result = date_iso_weeks_in_year(2026);

    TEST_ASSERT_EQUAL_INT(53, result);
}

/** @brief A plain year runs 52 ISO weeks, so a wrong total would inflate every week fraction. */
void test_iso_weeks_in_year_plain_is_52(void)
{
    int result = date_iso_weeks_in_year(2025);

    TEST_ASSERT_EQUAL_INT(52, result);
}

/** @brief January 1st must read day 1 or every day of the year is off by one. */
void test_day_of_year_jan1_is_1(void)
{
    int result = date_day_of_year(0);

    TEST_ASSERT_EQUAL_INT(1, result);
}

/** @brief December 31st must read 0 left or the countdown never reaches the end. */
void test_days_left_dec31_is_0(void)
{
    // 2026-12-31 is yday 364
    int result = date_days_left_in_year(2026, 364);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A leap year's January 1st has 365 left, so a missed leap day undercounts all year. */
void test_days_left_leap_jan1_is_365(void)
{
    int result = date_days_left_in_year(2024, 0);

    TEST_ASSERT_EQUAL_INT(365, result);
}

/** @brief A leap February must get its 29th or the month grid drops a day. */
void test_days_in_month_leap_february(void)
{
    int result = date_days_in_month(2024, 1);

    TEST_ASSERT_EQUAL_INT(29, result);
}

/** @brief A plain February stays at 28 or the grid invents a day. */
void test_days_in_month_plain_february(void)
{
    int result = date_days_in_month(2025, 1);

    TEST_ASSERT_EQUAL_INT(28, result);
}

/** @brief A century year like 1900 is not a leap year even though it divides by 4. */
void test_days_in_month_century_february_is_28(void)
{
    int result = date_days_in_month(1900, 1);

    TEST_ASSERT_EQUAL_INT(28, result);
}

/** @brief The 400-year exception keeps 2000 a leap year. */
void test_days_in_month_year_2000_february_is_29(void)
{
    int result = date_days_in_month(2000, 1);

    TEST_ASSERT_EQUAL_INT(29, result);
}

/** @brief A month index outside 0 to 11 must read 0 rather than walk off the table. */
void test_days_in_month_bad_month_is_0(void)
{
    int result = date_days_in_month(2026, 12);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Walking back from mid-month must land on the month's real opening weekday. */
void test_first_wday_walks_back_to_the_1st(void)
{
    // July 13th 2026 is a Monday so July 1st was a Wednesday
    int result = date_first_wday(1, 13);

    TEST_ASSERT_EQUAL_INT(3, result);
}

/** @brief On the 1st itself the walk is zero steps and must return today's weekday. */
void test_first_wday_on_the_1st_is_today(void)
{
    int result = date_first_wday(3, 1);

    TEST_ASSERT_EQUAL_INT(3, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_iso_week_jan1_on_thursday_is_week_1);
    RUN_TEST(test_iso_week_jan1_on_friday_is_last_years_week_53);
    RUN_TEST(test_iso_week_dec30_2024_is_week_1);
    RUN_TEST(test_iso_week_jan1_2021_is_week_53);
    RUN_TEST(test_iso_week_mid_year_monday);
    RUN_TEST(test_iso_week_dec31_2026_is_week_53);
    RUN_TEST(test_iso_week_year_mid_year_is_same_year);
    RUN_TEST(test_iso_week_year_jan1_2021_is_previous_year);
    RUN_TEST(test_iso_week_year_dec30_2024_is_next_year);
    RUN_TEST(test_iso_week_never_exceeds_its_own_years_total);
    RUN_TEST(test_iso_weeks_in_year_thursday_start_is_53);
    RUN_TEST(test_iso_weeks_in_year_plain_is_52);
    RUN_TEST(test_day_of_year_jan1_is_1);
    RUN_TEST(test_days_left_dec31_is_0);
    RUN_TEST(test_days_left_leap_jan1_is_365);
    RUN_TEST(test_days_in_month_leap_february);
    RUN_TEST(test_days_in_month_plain_february);
    RUN_TEST(test_days_in_month_century_february_is_28);
    RUN_TEST(test_days_in_month_year_2000_february_is_29);
    RUN_TEST(test_days_in_month_bad_month_is_0);
    RUN_TEST(test_first_wday_walks_back_to_the_1st);
    RUN_TEST(test_first_wday_on_the_1st_is_today);

    return UNITY_END();
}
