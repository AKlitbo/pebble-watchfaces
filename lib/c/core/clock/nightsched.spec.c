/**
 * @file nightsched.spec.c
 * @brief Host tests for the night window.
 *
 * The cases worth pinning are the ones that look fine until the day they do not: a window that
 * runs past midnight, the two boundaries (one inclusive, one not, or two adjacent windows fight
 * over a minute), and the fallback that has to fire when a watch has no sun readings. That last
 * one is the whole reason the fixed pair exists, and it is invisible on any watch that does have
 * weather, so nothing but a test would catch it going wrong.
 */
#include "unity.h"

#include "clock/nightsched.h"

void setUp(void) {}
void tearDown(void) {}

// 09:00 to 17:00, the easy case that does not wrap
#define DAY_START 540
#define DAY_END 1020

// 21:00 to 07:00, which is what a real night window looks like
#define NIGHT_START 1260
#define NIGHT_END 420

/** @brief A window inside one day contains the middle of itself. */
void test_plain_window_contains_its_middle(void)
{
    bool result = clock_window_contains(DAY_START, DAY_END, 600);

    TEST_ASSERT_TRUE(result);
}

/** @brief And nothing outside it. */
void test_plain_window_excludes_outside(void)
{
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, DAY_END, 400));
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, DAY_END, 1200));
}

/** @brief The start belongs to the window. */
void test_start_is_inside(void)
{
    bool result = clock_window_contains(DAY_START, DAY_END, DAY_START);

    TEST_ASSERT_TRUE(result);
}

/**
 * @brief The end does not.
 *
 * Half the point of the pair: two windows that meet at one minute must not both claim it.
 */
void test_end_is_outside(void)
{
    bool result = clock_window_contains(DAY_START, DAY_END, DAY_END);

    TEST_ASSERT_FALSE(result);
}

/** @brief A window that runs past midnight holds the evening side of it. */
void test_wrapping_window_holds_the_evening(void)
{
    bool result = clock_window_contains(NIGHT_START, NIGHT_END, 1300);

    TEST_ASSERT_TRUE(result);
}

/** @brief And the small hours on the other side of midnight. */
void test_wrapping_window_holds_the_small_hours(void)
{
    bool result = clock_window_contains(NIGHT_START, NIGHT_END, 60);

    TEST_ASSERT_TRUE(result);
}

/** @brief But not the middle of the day in between. */
void test_wrapping_window_excludes_the_afternoon(void)
{
    bool result = clock_window_contains(NIGHT_START, NIGHT_END, 720);

    TEST_ASSERT_FALSE(result);
}

/** @brief Both ends of the clock, since a wrapping window is where an off-by-one would hide. */
void test_wrapping_window_holds_both_ends_of_the_clock(void)
{
    TEST_ASSERT_TRUE(clock_window_contains(NIGHT_START, NIGHT_END, 0));
    TEST_ASSERT_TRUE(clock_window_contains(NIGHT_START, NIGHT_END, 1439));
}

/** @brief Two dials set the same is an empty window, not one that never closes. */
void test_zero_length_window_is_empty(void)
{
    TEST_ASSERT_FALSE(clock_window_contains(600, 600, 600));
    TEST_ASSERT_FALSE(clock_window_contains(600, 600, 0));
}

/** @brief A missing reading anywhere means no answer rather than a wrong one. */
void test_a_missing_reading_closes_the_window(void)
{
    TEST_ASSERT_FALSE(clock_window_contains(-1, DAY_END, 600));
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, -1, 600));
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, DAY_END, -1));
}

/** @brief So does a reading past the end of the day. */
void test_an_out_of_range_reading_closes_the_window(void)
{
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, 1440, 600));
    TEST_ASSERT_FALSE(clock_window_contains(DAY_START, DAY_END, 1440));
}

/** @brief Off is off, whatever the clock says. */
void test_off_is_never_night(void)
{
    bool result = night_schedule_active(NIGHT_SCHED_OFF, 1300, 420, 1260,
                                        NIGHT_START, NIGHT_END, true);

    TEST_ASSERT_FALSE(result);
}

/** @brief With nothing to switch to, no mode is night. */
void test_no_night_look_is_never_night(void)
{
    TEST_ASSERT_FALSE(night_schedule_active(NIGHT_SCHED_SOLAR, 1300, 420, 1260,
                                            NIGHT_START, NIGHT_END, false));
    TEST_ASSERT_FALSE(night_schedule_active(NIGHT_SCHED_FIXED, 1300, 420, 1260,
                                            NIGHT_START, NIGHT_END, false));
}

/** @brief Following the sun uses sunset round to sunrise. */
void test_solar_runs_from_sunset_to_sunrise(void)
{
    // sunrise 06:00, sunset 20:00. 21:00 is after dark, 12:00 is not
    TEST_ASSERT_TRUE(night_schedule_active(NIGHT_SCHED_SOLAR, 1260, 360, 1200, 0, 0, true));
    TEST_ASSERT_FALSE(night_schedule_active(NIGHT_SCHED_SOLAR, 720, 360, 1200, 0, 0, true));
}

/**
 * @brief With no sun reading it falls back to the fixed pair.
 *
 * The case the whole fallback exists for: a watch that has never had a weather provider set up.
 * The fixed times here would say no and the sun times, if they existed, would say yes, so a
 * fallback that quietly did nothing would pass a sloppier test than this one.
 */
void test_solar_falls_back_when_the_sun_is_unknown(void)
{
    // 02:00, with no readings at all, against a fixed 21:00 to 07:00
    TEST_ASSERT_TRUE(night_schedule_active(NIGHT_SCHED_SOLAR, 120, -1, -1,
                                           NIGHT_START, NIGHT_END, true));
    // and the same clock against a fixed window it sits outside
    TEST_ASSERT_FALSE(night_schedule_active(NIGHT_SCHED_SOLAR, 120, -1, -1,
                                            DAY_START, DAY_END, true));
}

/** @brief One reading missing is as good as none, since a window needs both ends. */
void test_solar_falls_back_when_only_one_sun_reading_is_missing(void)
{
    TEST_ASSERT_TRUE(night_schedule_active(NIGHT_SCHED_SOLAR, 120, -1, 1200,
                                           NIGHT_START, NIGHT_END, true));
    TEST_ASSERT_TRUE(night_schedule_active(NIGHT_SCHED_SOLAR, 120, 360, -1,
                                           NIGHT_START, NIGHT_END, true));
}

/** @brief The fixed mode ignores the sun even when it is there to be read. */
void test_fixed_ignores_the_sun(void)
{
    // sun says night at 21:00, the fixed pair says day
    bool result = night_schedule_active(NIGHT_SCHED_FIXED, 1260, 360, 1200,
                                        DAY_START, DAY_END, true);

    TEST_ASSERT_FALSE(result);
}

/** @brief A mode nobody defined reads as off rather than as something. */
void test_an_unknown_mode_is_off(void)
{
    bool result = night_schedule_active(99, 1300, 420, 1260, NIGHT_START, NIGHT_END, true);

    TEST_ASSERT_FALSE(result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_plain_window_contains_its_middle);
    RUN_TEST(test_plain_window_excludes_outside);
    RUN_TEST(test_start_is_inside);
    RUN_TEST(test_end_is_outside);
    RUN_TEST(test_wrapping_window_holds_the_evening);
    RUN_TEST(test_wrapping_window_holds_the_small_hours);
    RUN_TEST(test_wrapping_window_excludes_the_afternoon);
    RUN_TEST(test_wrapping_window_holds_both_ends_of_the_clock);
    RUN_TEST(test_zero_length_window_is_empty);
    RUN_TEST(test_a_missing_reading_closes_the_window);
    RUN_TEST(test_an_out_of_range_reading_closes_the_window);
    RUN_TEST(test_off_is_never_night);
    RUN_TEST(test_no_night_look_is_never_night);
    RUN_TEST(test_solar_runs_from_sunset_to_sunrise);
    RUN_TEST(test_solar_falls_back_when_the_sun_is_unknown);
    RUN_TEST(test_solar_falls_back_when_only_one_sun_reading_is_missing);
    RUN_TEST(test_fixed_ignores_the_sun);
    RUN_TEST(test_an_unknown_mode_is_off);
    return UNITY_END();
}
