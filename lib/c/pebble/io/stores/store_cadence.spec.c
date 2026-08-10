/**
 * @file store_cadence.spec.c
 * @brief Host tests for the shared store cadence list.
 *
 * Every store hangs its periodic work here, so a fault in the list is not one store misbehaving
 * but weather, stocks and the calendar all quietly stopping at once, with the face still drawing
 * the last thing it had. The list holds process-wide state and offers no reset, so each test below
 * counts only its own callback and the registrations accumulate down the file. The overflow test
 * runs last for that reason, since it deliberately fills the list.
 */
#include "unity.h"

#include "io/stores/store_cadence.h"

static int s_first_calls;
static int s_second_calls;
static int s_order[2];
static int s_order_len;

void setUp(void)
{
    s_first_calls = 0;
    s_second_calls = 0;
    s_order_len = 0;
}

void tearDown(void) {}

static void bump_first(void)
{
    s_first_calls++;
    if (s_order_len < 2)
    {
        s_order[s_order_len++] = 1;
    }
}

static void bump_second(void)
{
    s_second_calls++;
    if (s_order_len < 2)
    {
        s_order[s_order_len++] = 2;
    }
}

/** @brief A callback that never runs is a store that never refreshes, so the plain case is pinned. */
void test_a_registered_callback_runs_on_the_cadence(void)
{
    store_cadence_register(bump_first);

    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(1, s_first_calls);
}

/** @brief Firing again runs it again, since this is a cadence and not a one-shot. */
void test_it_runs_again_on_the_next_cadence(void)
{
    store_cadence_fire();
    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(2, s_first_calls);
}

/**
 * @brief Registering the same callback twice does nothing the second time.
 *
 * A store that inits twice would otherwise poll twice per cadence, spending double the phone
 * wakeups and double a provider's quota for the same readings.
 */
void test_registering_the_same_callback_twice_only_runs_it_once(void)
{
    store_cadence_register(bump_first);
    store_cadence_register(bump_first);

    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(1, s_first_calls);
}

/** @brief A NULL registration must be ignored rather than stored and later called. */
void test_a_null_registration_is_ignored(void)
{
    store_cadence_register(NULL);

    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(1, s_first_calls);
}

/** @brief The order is the registration order, which is what lets a store rely on an earlier one. */
void test_callbacks_run_in_registration_order(void)
{
    store_cadence_register(bump_second);

    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(2, s_order_len);
    TEST_ASSERT_EQUAL_INT(1, s_order[0]);
    TEST_ASSERT_EQUAL_INT(2, s_order[1]);
}

/** @brief Both stay registered, so adding a second store does not displace the first. */
void test_a_second_registration_does_not_displace_the_first(void)
{
    store_cadence_fire();

    TEST_ASSERT_EQUAL_INT(1, s_first_calls);
    TEST_ASSERT_EQUAL_INT(1, s_second_calls);
}

/*
 * one filler per slot, so the overflow test can hand over more callbacks than the list can hold
 * without reusing one and hitting the dedupe instead of the cap
 */
static int s_filler_calls;
static void filler_0(void) { s_filler_calls++; }
static void filler_1(void) { s_filler_calls++; }
static void filler_2(void) { s_filler_calls++; }
static void filler_3(void) { s_filler_calls++; }
static void filler_4(void) { s_filler_calls++; }
static void filler_5(void) { s_filler_calls++; }
static void filler_6(void) { s_filler_calls++; }
static void filler_7(void) { s_filler_calls++; }
static void filler_8(void) { s_filler_calls++; }

/**
 * @brief Past the cap the extra registration is dropped rather than written off the end.
 *
 * The list is a fixed array, so the alternative to dropping is writing past it and corrupting
 * whatever sits after. Dropping loses one store's cadence, which the log says out loud. This runs
 * last because it fills the list for good.
 */
void test_a_full_list_drops_the_extra_registration(void)
{
    void (*fillers[9])(void) = {
        filler_0, filler_1, filler_2, filler_3, filler_4,
        filler_5, filler_6, filler_7, filler_8,
    };
    for (int i = 0; i < 9; i++)
    {
        store_cadence_register(fillers[i]);
    }
    s_filler_calls = 0;

    store_cadence_fire();

    // the list already held the earlier tests' callbacks, so at most the free slots were taken
    TEST_ASSERT_TRUE(s_filler_calls < 9);
    TEST_ASSERT_EQUAL_INT(1, s_first_calls);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_a_registered_callback_runs_on_the_cadence);
    RUN_TEST(test_it_runs_again_on_the_next_cadence);
    RUN_TEST(test_registering_the_same_callback_twice_only_runs_it_once);
    RUN_TEST(test_a_null_registration_is_ignored);
    RUN_TEST(test_callbacks_run_in_registration_order);
    RUN_TEST(test_a_second_registration_does_not_displace_the_first);
    RUN_TEST(test_a_full_list_drops_the_extra_registration);

    return UNITY_END();
}
