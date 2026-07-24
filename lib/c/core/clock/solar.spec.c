/**
 * @file solar.spec.c
 * @brief Host tests for the day/night progress and the next sun event.
 *
 * The arithmetic is easy in the middle of the day and awkward at the edges, so the cases worth
 * pinning are the ones with a boundary or a midnight in them: night wrapping past midnight, the
 * moment on sunrise or sunset, and the three day-phases the next event has to tell apart. A missing
 * reading of -1 must never be treated as a real time.
 */
#include "unity.h"

#include "clock/solar.h"

void setUp(void) {}
void tearDown(void) {}

// a day where the sun is up from 06:00 (360) to 18:00 (1080)
#define RISE 360
#define SET 1080

/** @brief Noon is exactly halfway through a six-to-six day. */
void test_day_midday_is_halfway(void)
{
    int result = solar_day_progress(RISE, SET, 720);

    TEST_ASSERT_EQUAL_INT(50, result);
}

/** @brief Before sunrise it is night, so day-progress has nothing to show. */
void test_day_before_sunrise_is_none(void)
{
    int result = solar_day_progress(RISE, SET, 300);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief After sunset it is night again, and past the end is not 100 but nothing. */
void test_day_after_sunset_is_none(void)
{
    int result = solar_day_progress(RISE, SET, 1200);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief A missing reading is not a time and must not be counted as midnight. */
void test_day_no_data_is_none(void)
{
    int result = solar_day_progress(-1, SET, 720);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief In daylight there is no night to show. */
void test_night_during_the_day_is_none(void)
{
    int result = solar_night_progress(RISE, SET, 720);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief The small hours wrap onto the night that began at yesterday's sunset, so midnight reads as halfway through rather than as no data. */
void test_night_after_midnight_wraps(void)
{
    int result = solar_night_progress(RISE, SET, 0);

    TEST_ASSERT_EQUAL_INT(50, result);
}

/** @brief Just after sunset the night has barely begun. */
void test_night_just_after_sunset_is_low(void)
{
    int result = solar_night_progress(RISE, SET, 1081);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Before sunrise, the next event is that sunrise, and its distance is the wait. */
void test_next_is_sunrise_while_dark(void)
{
    bool is_sunrise = false;

    int result = solar_next_event(RISE, SET, 300, &is_sunrise);

    TEST_ASSERT_EQUAL_INT(60, result);
    TEST_ASSERT_TRUE(is_sunrise);
}

/** @brief While the sun is up, the next event is the sunset. */
void test_next_is_sunset_while_up(void)
{
    bool is_sunrise = true;

    int result = solar_next_event(RISE, SET, 720, &is_sunrise);

    TEST_ASSERT_EQUAL_INT(360, result);
    TEST_ASSERT_FALSE(is_sunrise);
}

/** @brief Once the sun has set, the next event is tomorrow's sunrise, counted across midnight. */
void test_next_is_tomorrows_sunrise_after_set(void)
{
    bool is_sunrise = false;

    int result = solar_next_event(RISE, SET, 1200, &is_sunrise);

    TEST_ASSERT_EQUAL_INT(600, result);
    TEST_ASSERT_TRUE(is_sunrise);
}

/** @brief With no data there is no event, and the caller's flag is left as it was. */
void test_next_no_data_is_none_and_leaves_the_flag(void)
{
    bool is_sunrise = false;

    int result = solar_next_event(RISE, -1, 720, &is_sunrise);

    TEST_ASSERT_EQUAL_INT(-1, result);
    TEST_ASSERT_FALSE(is_sunrise);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_day_midday_is_halfway);
    RUN_TEST(test_day_before_sunrise_is_none);
    RUN_TEST(test_day_after_sunset_is_none);
    RUN_TEST(test_day_no_data_is_none);
    RUN_TEST(test_night_during_the_day_is_none);
    RUN_TEST(test_night_after_midnight_wraps);
    RUN_TEST(test_night_just_after_sunset_is_low);
    RUN_TEST(test_next_is_sunrise_while_dark);
    RUN_TEST(test_next_is_sunset_while_up);
    RUN_TEST(test_next_is_tomorrows_sunrise_after_set);
    RUN_TEST(test_next_no_data_is_none_and_leaves_the_flag);

    return UNITY_END();
}
