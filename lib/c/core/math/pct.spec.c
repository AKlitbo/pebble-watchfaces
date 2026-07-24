/**
 * @file pct.spec.c
 * @brief Host tests for the goal percentage.
 *
 * Every goal panel draws its bar off this, so the two things worth pinning are the ones that are
 * not arithmetic: a goal of zero must not divide, and a beaten goal must not fill past the end of
 * the bar.
 */
#include "unity.h"

#include "math/pct.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief Half the goal is half the bar, the case everything else is a corner of. */
void test_half_way_is_fifty(void)
{
    int result = pct_of(5000, 10000);

    TEST_ASSERT_EQUAL_INT(50, result);
}

/** @brief Exactly on the goal is a full bar and not 99. */
void test_on_the_goal_is_one_hundred(void)
{
    int result = pct_of(10000, 10000);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/**
 * @brief Beating the goal must cap, or the bar fills past its own outline.
 *
 * Walking twice the step goal is the everyday case here, not an edge one.
 */
void test_past_the_goal_caps_at_one_hundred(void)
{
    int result = pct_of(20000, 10000);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/**
 * @brief A goal of zero must answer 0 rather than divide by it.
 *
 * A goal comes out of the settings, so a user or a bad restore can make it zero, and dividing by
 * it takes the watch down rather than drawing a wrong bar.
 */
void test_a_zero_goal_does_not_divide(void)
{
    int result = pct_of(500, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A goal below zero is no more divisible than a zero one. */
void test_a_negative_goal_does_not_divide(void)
{
    int result = pct_of(500, -10);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Nothing done yet is an empty bar. */
void test_nothing_done_is_zero(void)
{
    int result = pct_of(0, 10000);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/**
 * @brief The no-data sentinel reads as an empty bar, same as a real zero.
 *
 * Worth pinning because it is a decision rather than an accident: a reading of -1 means the store
 * has nothing yet, and this answers 0 for it, so a caller that wants to draw no bar at all has to
 * notice the -1 before it asks. The value line beside the bar does notice, and prints "--".
 */
void test_the_no_data_sentinel_reads_as_zero(void)
{
    int result = pct_of(-1, 10000);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief The maths truncates rather than rounds, so a third of the way is 33 and not 34. */
void test_it_truncates(void)
{
    int result = pct_of(1, 3);

    TEST_ASSERT_EQUAL_INT(33, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_half_way_is_fifty);
    RUN_TEST(test_on_the_goal_is_one_hundred);
    RUN_TEST(test_past_the_goal_caps_at_one_hundred);
    RUN_TEST(test_a_zero_goal_does_not_divide);
    RUN_TEST(test_a_negative_goal_does_not_divide);
    RUN_TEST(test_nothing_done_is_zero);
    RUN_TEST(test_the_no_data_sentinel_reads_as_zero);
    RUN_TEST(test_it_truncates);

    return UNITY_END();
}
