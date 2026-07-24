/**
 * @file duration.spec.c
 * @brief Host tests for the minute-to-duration formatting.
 *
 * The arithmetic is a divide and a modulo, so the cases worth pinning are the display decisions:
 * the zero-padded minute, the dropped hours under an hour, and a negative reading not printing a
 * stray minus.
 */
#include "unity.h"

#include "clock/duration.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief The everyday case: hours and minutes both present. */
void test_hm_hours_and_minutes(void)
{
    char out[16];

    duration_hm(out, sizeof(out), 445);

    TEST_ASSERT_EQUAL_STRING("7:25", out);
}

/** @brief The minute is zero-padded, or "7:05" reads as "7:5". */
void test_hm_pads_the_minute(void)
{
    char out[16];

    duration_hm(out, sizeof(out), 425);

    TEST_ASSERT_EQUAL_STRING("7:05", out);
}

/** @brief On the hour still shows the ":00" rather than dropping it. */
void test_hm_on_the_hour(void)
{
    char out[16];

    duration_hm(out, sizeof(out), 180);

    TEST_ASSERT_EQUAL_STRING("3:00", out);
}

/** @brief A negative reading reads as nothing, no stray minus. */
void test_hm_negative_is_zero(void)
{
    char out[16];

    duration_hm(out, sizeof(out), -1);

    TEST_ASSERT_EQUAL_STRING("0:00", out);
}

/** @brief Under an hour drops the hours part entirely. */
void test_compact_under_an_hour_has_no_hours(void)
{
    char out[16];

    duration_hm_compact(out, sizeof(out), 45);

    TEST_ASSERT_EQUAL_STRING("45m", out);
}

/** @brief An hour or more shows both parts, minute not padded here. */
void test_compact_hours_and_minutes(void)
{
    char out[16];

    duration_hm_compact(out, sizeof(out), 200);

    TEST_ASSERT_EQUAL_STRING("3h 20m", out);
}

/** @brief On the hour keeps the "0m" so the shape does not jump between forms. */
void test_compact_on_the_hour(void)
{
    char out[16];

    duration_hm_compact(out, sizeof(out), 180);

    TEST_ASSERT_EQUAL_STRING("3h 0m", out);
}

/** @brief A negative reading reads "0m", the same nothing the H:MM form gives. */
void test_compact_negative_is_zero(void)
{
    char out[16];

    duration_hm_compact(out, sizeof(out), -5);

    TEST_ASSERT_EQUAL_STRING("0m", out);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_hm_hours_and_minutes);
    RUN_TEST(test_hm_pads_the_minute);
    RUN_TEST(test_hm_on_the_hour);
    RUN_TEST(test_hm_negative_is_zero);
    RUN_TEST(test_compact_under_an_hour_has_no_hours);
    RUN_TEST(test_compact_hours_and_minutes);
    RUN_TEST(test_compact_on_the_hour);
    RUN_TEST(test_compact_negative_is_zero);

    return UNITY_END();
}
