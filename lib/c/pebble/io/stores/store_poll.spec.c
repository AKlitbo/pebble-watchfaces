/**
 * @file store_poll.spec.c
 * @brief Host tests for the store poll deadlines.
 *
 * The stores read the wall clock themselves but hand it to these two, so the deciding is pure and
 * testable while the clock reading stays at the edge. What matters here is not the arithmetic but
 * the two ways a deadline goes wrong: firing more often than the interval, which spends the user's
 * phone battery and a provider's quota, and never firing at all, which leaves the face showing
 * yesterday's weather with nothing to say it is stale.
 */
#include "unity.h"

#include "io/stores/store_poll.h"

/** A middling cadence, and the interval it works out to. */
#define POLL_MIN 30
#define INTERVAL (POLL_MIN * SECONDS_PER_MINUTE)

void setUp(void) {}
void tearDown(void) {}

/** @brief The next deadline is the following interval boundary, the case the rest are corners of. */
void test_the_next_deadline_is_the_following_boundary(void)
{
    time_t result = store_poll_next(POLL_MIN, 0);

    TEST_ASSERT_EQUAL_INT(INTERVAL, (int)result);
}

/** @brief A second before a boundary still aims at that boundary rather than skipping it. */
void test_just_before_a_boundary_aims_at_it(void)
{
    time_t result = store_poll_next(POLL_MIN, INTERVAL - 1);

    TEST_ASSERT_EQUAL_INT(INTERVAL, (int)result);
}

/** @brief Landing exactly on a boundary moves to the next one, so a poll cannot immediately redo itself. */
void test_on_a_boundary_moves_to_the_next_one(void)
{
    time_t result = store_poll_next(POLL_MIN, INTERVAL);

    TEST_ASSERT_EQUAL_INT(2 * INTERVAL, (int)result);
}

/**
 * @brief Deadlines are counted from the epoch, not from launch.
 *
 * The point of counting from the epoch is that two watches started minutes apart still poll at the
 * same moments, and a relaunch does not shift the phase. Two different launch times landing on one
 * deadline is that promise written down.
 */
void test_two_launch_times_in_one_interval_share_a_deadline(void)
{
    time_t early = store_poll_next(POLL_MIN, 100);
    time_t late = store_poll_next(POLL_MIN, 1000);

    TEST_ASSERT_EQUAL_INT((int)early, (int)late);
}

/** @brief A cadence of zero means polling is off, so nothing is ever due. */
void test_polling_off_is_never_due(void)
{
    time_t next = 0;

    bool result = store_poll_due(0, &next, 999999);

    TEST_ASSERT_FALSE(result);
}

/** @brief A negative cadence is off too, and must not be multiplied into a nonsense interval. */
void test_a_negative_cadence_is_never_due(void)
{
    time_t next = 0;

    bool result = store_poll_due(-5, &next, 999999);

    TEST_ASSERT_FALSE(result);
}

/** @brief A deadline still ahead is not due, or the store polls every tick instead of every interval. */
void test_a_deadline_still_ahead_is_not_due(void)
{
    time_t next = INTERVAL;

    bool result = store_poll_due(POLL_MIN, &next, INTERVAL - 1);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(INTERVAL, (int)next);
}

/** @brief The deadline second itself is due, not the one after it. */
void test_the_deadline_second_is_due(void)
{
    time_t next = INTERVAL;

    bool result = store_poll_due(POLL_MIN, &next, INTERVAL);

    TEST_ASSERT_TRUE(result);
}

/** @brief Firing moves the deadline on, so the same interval cannot fire a second time. */
void test_firing_moves_the_deadline_on(void)
{
    time_t next = INTERVAL;

    store_poll_due(POLL_MIN, &next, INTERVAL);

    TEST_ASSERT_EQUAL_INT(2 * INTERVAL, (int)next);
}

/** @brief Asking again in the same interval answers no, which is what stops a poll storm. */
void test_it_does_not_fire_twice_in_one_interval(void)
{
    time_t next = INTERVAL;
    store_poll_due(POLL_MIN, &next, INTERVAL);

    bool result = store_poll_due(POLL_MIN, &next, INTERVAL + 1);

    TEST_ASSERT_FALSE(result);
}

/**
 * @brief A clock that jumped forward fires once and carries on rather than catching up.
 *
 * A phone that corrects a badly wrong clock can move the watch hours ahead. Every missed interval
 * firing at once would spend a day of a provider's quota in one go, so the deadline just re-bases
 * on the new now.
 */
void test_a_clock_that_jumped_forward_fires_once(void)
{
    time_t next = INTERVAL;
    time_t jumped = 100 * INTERVAL;

    bool result = store_poll_due(POLL_MIN, &next, jumped);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT((int)(101 * INTERVAL), (int)next);
}

/**
 * @brief A clock that jumped back pulls the deadline in instead of stranding it.
 *
 * Without this the deadline sits however far the clock moved into the future, and the store simply
 * stops polling until real time catches up. A timezone fix or a bad sync can be hours, so the face
 * would sit on stale weather for the rest of the day with no sign anything is wrong.
 */
void test_a_clock_that_jumped_back_pulls_the_deadline_in(void)
{
    time_t next = 100 * INTERVAL;

    bool result = store_poll_due(POLL_MIN, &next, 0);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT(INTERVAL, (int)next);
}

/**
 * @brief A deadline exactly one interval ahead is normal and must be left alone.
 *
 * This is the edge the jump-back guard sits on. Pulling this one in would re-base the deadline on
 * every ordinary tick, which quietly turns the cadence into something else.
 */
void test_a_deadline_one_interval_ahead_is_left_alone(void)
{
    time_t next = 2 * INTERVAL;

    bool result = store_poll_due(POLL_MIN, &next, INTERVAL);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL_INT((int)(2 * INTERVAL), (int)next);
}

/** @brief A one minute cadence is the tightest a face asks for and must still land on the next minute. */
void test_the_tightest_cadence_still_advances(void)
{
    time_t next = 0;

    bool result = store_poll_due(1, &next, 90);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(120, (int)next);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_the_next_deadline_is_the_following_boundary);
    RUN_TEST(test_just_before_a_boundary_aims_at_it);
    RUN_TEST(test_on_a_boundary_moves_to_the_next_one);
    RUN_TEST(test_two_launch_times_in_one_interval_share_a_deadline);
    RUN_TEST(test_polling_off_is_never_due);
    RUN_TEST(test_a_negative_cadence_is_never_due);
    RUN_TEST(test_a_deadline_still_ahead_is_not_due);
    RUN_TEST(test_the_deadline_second_is_due);
    RUN_TEST(test_firing_moves_the_deadline_on);
    RUN_TEST(test_it_does_not_fire_twice_in_one_interval);
    RUN_TEST(test_a_clock_that_jumped_forward_fires_once);
    RUN_TEST(test_a_clock_that_jumped_back_pulls_the_deadline_in);
    RUN_TEST(test_a_deadline_one_interval_ahead_is_left_alone);
    RUN_TEST(test_the_tightest_cadence_still_advances);

    return UNITY_END();
}
