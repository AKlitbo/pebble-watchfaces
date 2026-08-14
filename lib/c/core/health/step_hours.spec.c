/**
 * @file step_hours.spec.c
 * @brief Host tests for the step chart's hour arithmetic.
 *
 * Both encode a timing detail of the watch rather than plain maths: an hour settles a minute
 * later than you would guess, and a record's place in a batch says nothing about its hour.
 */
#include "unity.h"

#include "health/step_hours.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A minute into the hour, everything before it has settled. */
void test_past_the_first_minute_settles_the_hour_before(void)
{
    int result = step_hours_settled(9, 1);

    TEST_ASSERT_EQUAL_INT(9, result);
}

/** @brief On the rollover minute the hour just gone is held back, since its last record is late. */
void test_on_the_rollover_minute_the_hour_just_gone_is_held(void)
{
    int result = step_hours_settled(9, 0);

    TEST_ASSERT_EQUAL_INT(8, result);
}

/** @brief Midnight's first minute has no earlier hour to hold back, and must not go negative. */
void test_midnight_first_minute_floors_at_zero(void)
{
    int result = step_hours_settled(0, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief The last hour of the day settles like any other. */
void test_last_hour_settles_like_the_rest(void)
{
    int result = step_hours_settled(23, 30);

    TEST_ASSERT_EQUAL_INT(23, result);
}

/** @brief Midnight itself is the first bucket, or an off-by-one shifts the whole day's bars an hour along. */
void test_day_start_is_bucket_zero(void)
{
    int result = step_hours_bucket(0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A record lands in the hour it falls in, not the one it is nearest. */
void test_record_lands_in_its_own_hour(void)
{
    int result = step_hours_bucket(3 * 3600 + 59 * 60);

    TEST_ASSERT_EQUAL_INT(3, result);
}

/** @brief The first second of an hour belongs to that hour, not the one before. */
void test_hour_boundary_belongs_to_the_new_hour(void)
{
    int result = step_hours_bucket(4 * 3600);

    TEST_ASSERT_EQUAL_INT(4, result);
}

/** @brief The last second of the day is still the last bucket, since spilling one past the end drops the evening's steps off the chart. */
void test_last_second_of_the_day_is_the_last_bucket(void)
{
    int result = step_hours_bucket(24 * 3600 - 1);

    TEST_ASSERT_EQUAL_INT(23, result);
}

/** @brief A record past midnight is not this day's, so it has no bucket. */
void test_past_the_end_of_the_day_has_no_bucket(void)
{
    int result = step_hours_bucket(24 * 3600);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/**
 * @brief A record before the day started has no bucket either.
 *
 * The watch only ever moves a window's start forward, so this should not come up. It is guarded
 * anyway because the caller does the subtraction, and a wrong day there would index off the front.
 */
void test_before_the_day_started_has_no_bucket(void)
{
    int result = step_hours_bucket(-60);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_past_the_first_minute_settles_the_hour_before);
    RUN_TEST(test_on_the_rollover_minute_the_hour_just_gone_is_held);
    RUN_TEST(test_midnight_first_minute_floors_at_zero);
    RUN_TEST(test_last_hour_settles_like_the_rest);
    RUN_TEST(test_day_start_is_bucket_zero);
    RUN_TEST(test_record_lands_in_its_own_hour);
    RUN_TEST(test_hour_boundary_belongs_to_the_new_hour);
    RUN_TEST(test_last_second_of_the_day_is_the_last_bucket);
    RUN_TEST(test_past_the_end_of_the_day_has_no_bucket);
    RUN_TEST(test_before_the_day_started_has_no_bucket);

    return UNITY_END();
}
