/**
 * @file tide.spec.c
 * @brief Host tests for the tidal rise and fall.
 *
 * The shape matters more than any one reading here, so the cases worth pinning are the ones a
 * plain sawtooth would also pass: that the turns really flatten out, that the level never leaves
 * 0..100, and that a negative minute count wraps instead of running the maths on a negative
 * remainder. The 24h50m drift is the other half of it, since a tide that came back at the same
 * clock time every day would be the one thing a tide never does.
 */
#include "unity.h"

#include "clock/tide.h"

void setUp(void) {}
void tearDown(void) {}

// a solar day, for checking the tide against the clock rather than against itself
#define MINUTES_PER_DAY 1440

/** @brief The count starts at dead low, which is what the rest of the cases are measured from. */
void test_starts_at_low_water(void)
{
    int result = tide_level(0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A full period on the water is back where it started, or the cycle never closes and the level walks away over a week. */
void test_returns_to_low_water_after_one_period(void)
{
    int result = tide_level(TIDE_PERIOD_MIN);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief High water lands halfway through, and it reaches the top of the range. */
void test_reaches_high_water_midway(void)
{
    int result = tide_level(TIDE_PERIOD_MIN / 2 + 1);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/** @brief The level is a level: nothing in the cycle may fall outside 0..100. */
void test_stays_within_range_across_the_cycle(void)
{
    int lowest = 100;
    int highest = 0;

    for (int32_t minute = 0; minute < TIDE_PERIOD_MIN; minute++)
    {
        int level = tide_level(minute);
        lowest = level < lowest ? level : lowest;
        highest = level > highest ? level : highest;
    }

    TEST_ASSERT_EQUAL_INT(0, lowest);
    TEST_ASSERT_EQUAL_INT(100, highest);
}

/** @brief The same point in a later cycle reads the same, so the water does not wander. */
void test_repeats_every_period(void)
{
    int first = tide_level(200);
    int later = tide_level(200 + TIDE_PERIOD_MIN * 9);

    TEST_ASSERT_EQUAL_INT(first, later);
}

/**
 * @brief A count from before the epoch still lands inside the cycle.
 *
 * C leaves a negative remainder for a negative dividend, so an unguarded modulo would put the
 * water below the beach.
 */
void test_negative_minutes_wrap_into_the_cycle(void)
{
    int result = tide_level(-300);

    TEST_ASSERT_EQUAL_INT(tide_level(-300 + TIDE_PERIOD_MIN), result);
    TEST_ASSERT_TRUE(result >= 0 && result <= 100);
}

/**
 * @brief The tide stands almost still at the turn and runs fastest halfway through.
 *
 * This is the case a straight ramp fails. Without it the water would crawl up the sand at one
 * constant rate, which is the tell that nothing real is behind it.
 */
void test_moves_slower_at_the_turn_than_at_mid_tide(void)
{
    int at_turn = tide_level(20) - tide_level(0);
    int at_middle = tide_level(TIDE_PERIOD_MIN / 4 + 20) - tide_level(TIDE_PERIOD_MIN / 4);

    TEST_ASSERT_TRUE(at_turn < at_middle);
}

/** @brief It floods over the first half, and the direction is what the arrow on the face draws. */
void test_rising_through_the_first_half(void)
{
    bool result = tide_rising(TIDE_PERIOD_MIN / 4);

    TEST_ASSERT_TRUE(result);
}

/** @brief And ebbs back over the second, since an arrow stuck one way round is worse than none at all. */
void test_falling_through_the_second_half(void)
{
    bool result = tide_rising((TIDE_PERIOD_MIN * 3) / 4);

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief Low water comes back 24h50m later, not 24h.
 *
 * Two periods run fifty minutes past a solar day, which is the drift that makes a tide walk
 * around the clock over a fortnight.
 */
void test_low_water_drifts_fifty_minutes_later_each_day(void)
{
    int result = tide_level(MINUTES_PER_DAY + 50);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(MINUTES_PER_DAY + 50, TIDE_PERIOD_MIN * 2);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_starts_at_low_water);
    RUN_TEST(test_returns_to_low_water_after_one_period);
    RUN_TEST(test_reaches_high_water_midway);
    RUN_TEST(test_stays_within_range_across_the_cycle);
    RUN_TEST(test_repeats_every_period);
    RUN_TEST(test_negative_minutes_wrap_into_the_cycle);
    RUN_TEST(test_moves_slower_at_the_turn_than_at_mid_tide);
    RUN_TEST(test_rising_through_the_first_half);
    RUN_TEST(test_falling_through_the_second_half);
    RUN_TEST(test_low_water_drifts_fifty_minutes_later_each_day);

    return UNITY_END();
}
