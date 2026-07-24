/**
 * @file distance.spec.c
 * @brief Host tests for the distance conversions and formatting.
 *
 * The rounding to one tenth carries across places (4.96 becomes 5.0), negative input
 * has to clamp, and the formatted string has to carry the right unit. Those are the
 * spots a slip would show a wrong walked distance.
 */
#include "unity.h"

#include "units/distance.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief The unit label must track the setting or the number reads in the wrong system. */
void test_distance_unit_labels_each_system(void)
{
    TEST_ASSERT_EQUAL_STRING("MI", distance_unit(true));
    TEST_ASSERT_EQUAL_STRING("KM", distance_unit(false));
}

/** @brief A mile is 1609.344 m, so an exact thousand miles worth of metres must read back as 1000.0. */
void test_distance_format_value_thousand_miles_exact(void)
{
    char buffer[12];

    distance_format_value(buffer, sizeof(buffer), 1609344, true);

    TEST_ASSERT_EQUAL_STRING("1000.0", buffer);
}

/** @brief The tenth rounding must carry, so 4960 m (4.96 km) has to read "5.0" not "4.9". */
void test_distance_format_value_rounds_up_with_carry(void)
{
    char buffer[12];

    distance_format_value(buffer, sizeof(buffer), 4960, false);

    TEST_ASSERT_EQUAL_STRING("5.0", buffer);
}

/** @brief A negative reading must clamp to "0.0" rather than print a below zero distance. */
void test_distance_format_value_clamps_negative(void)
{
    char buffer[12];

    distance_format_value(buffer, sizeof(buffer), -100, false);

    TEST_ASSERT_EQUAL_STRING("0.0", buffer);
}

/** @brief The full format must staple the unit on, so 5000 m in km reads "5.0 KM". */
void test_distance_format_appends_unit(void)
{
    char buffer[16];

    distance_format(buffer, sizeof(buffer), 5000, false);

    TEST_ASSERT_EQUAL_STRING("5.0 KM", buffer);
}

/** @brief In miles it must convert and label together, so 1609 m rounds to "1.0 MI". */
void test_distance_format_miles_converts_and_labels(void)
{
    char buffer[16];

    distance_format(buffer, sizeof(buffer), 1609, true);

    TEST_ASSERT_EQUAL_STRING("1.0 MI", buffer);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_distance_unit_labels_each_system);
    RUN_TEST(test_distance_format_value_thousand_miles_exact);
    RUN_TEST(test_distance_format_value_rounds_up_with_carry);
    RUN_TEST(test_distance_format_value_clamps_negative);
    RUN_TEST(test_distance_format_appends_unit);
    RUN_TEST(test_distance_format_miles_converts_and_labels);

    return UNITY_END();
}
