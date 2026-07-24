/**
 * @file weekday.spec.c
 * @brief Host tests for the two-letter weekday lookup.
 *
 * The index comes straight from tm_wday and the phone, so the ends of the range and
 * the out of range guard are the parts worth pinning down.
 */
#include "unity.h"

#include "clock/weekday.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief Index 0 must be Sunday, the anchor the rest of the week counts from. */
void test_weekday_short_sunday_is_su(void)
{
    const char *result = weekday_short(0);

    TEST_ASSERT_EQUAL_STRING("SU", result);
}

/** @brief Index 6 must be Saturday, the far end of the table. */
void test_weekday_short_saturday_is_sa(void)
{
    const char *result = weekday_short(6);

    TEST_ASSERT_EQUAL_STRING("SA", result);
}

/** @brief A negative index must return an empty string rather than read before the table. */
void test_weekday_short_below_range_is_empty(void)
{
    const char *result = weekday_short(-1);

    TEST_ASSERT_EQUAL_STRING("", result);
}

/** @brief An index past Saturday must return an empty string rather than read past the table. */
void test_weekday_short_above_range_is_empty(void)
{
    const char *result = weekday_short(7);

    TEST_ASSERT_EQUAL_STRING("", result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_weekday_short_sunday_is_su);
    RUN_TEST(test_weekday_short_saturday_is_sa);
    RUN_TEST(test_weekday_short_below_range_is_empty);
    RUN_TEST(test_weekday_short_above_range_is_empty);

    return UNITY_END();
}
