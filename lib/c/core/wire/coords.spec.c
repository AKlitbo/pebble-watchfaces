/**
 * @file coords.spec.c
 * @brief Host tests for telling a real fix from the phone saying it has none.
 *
 * This decides what gets written to flash as the last known place. Let a half fix through and the
 * missing half is what a relaunch reads back, over and over, because the bad pair is now the saved
 * one. Both halves being checked is the whole point, so most of this is about the half cases.
 */
#include "unity.h"

#include "wire/coords.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A real pair is worth keeping, the case everything else is measured against. */
void test_a_real_pair_is_real(void)
{
    bool result = coords_look_real("43.7", "-79.4");

    TEST_ASSERT_TRUE(result);
}

/**
 * @brief A latitude with no longitude is not a place.
 *
 * The one that matters. Check only the latitude and this pair passes, gets saved, and the watch
 * reads an empty longitude back on every launch from then on.
 */
void test_a_latitude_on_its_own_is_not_a_place(void)
{
    bool result = coords_look_real("43.7", "");

    TEST_ASSERT_FALSE(result);
}

/** @brief And the same the other way round, or the check is only half there. */
void test_a_longitude_on_its_own_is_not_a_place(void)
{
    bool result = coords_look_real("", "-79.4");

    TEST_ASSERT_FALSE(result);
}

/** @brief No fix at all is not a place. */
void test_two_empty_strings_are_not_a_place(void)
{
    bool result = coords_look_real("", "");

    TEST_ASSERT_FALSE(result);
}

/** @brief The phone says it has no fix in words, and words carry no digits. */
void test_the_no_fix_placeholder_is_not_a_place(void)
{
    bool result = coords_look_real("NO LOCK", "NO LOCK");

    TEST_ASSERT_FALSE(result);
}

/** @brief One half being a placeholder is as bad as it being empty. */
void test_half_a_placeholder_is_not_a_place(void)
{
    bool result = coords_look_real("43.7", "NO LOCK");

    TEST_ASSERT_FALSE(result);
}

/** @brief Zero is a real coordinate, so a fix on the meridian must not read as no fix. */
void test_zero_is_a_real_coordinate(void)
{
    bool result = coords_look_real("0.0", "0.0");

    TEST_ASSERT_TRUE(result);
}

/** @brief A negative pair is a real place, so the minus must not put it off. */
void test_a_negative_pair_is_real(void)
{
    bool result = coords_look_real("-33.86", "-151.2");

    TEST_ASSERT_TRUE(result);
}

/** @brief Nothing at all is not a place, and must not be read as one. */
void test_a_missing_string_is_not_a_place(void)
{
    bool result = coords_look_real(NULL, "-79.4");

    TEST_ASSERT_FALSE(result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_a_real_pair_is_real);
    RUN_TEST(test_a_latitude_on_its_own_is_not_a_place);
    RUN_TEST(test_a_longitude_on_its_own_is_not_a_place);
    RUN_TEST(test_two_empty_strings_are_not_a_place);
    RUN_TEST(test_the_no_fix_placeholder_is_not_a_place);
    RUN_TEST(test_half_a_placeholder_is_not_a_place);
    RUN_TEST(test_zero_is_a_real_coordinate);
    RUN_TEST(test_a_negative_pair_is_real);
    RUN_TEST(test_a_missing_string_is_not_a_place);

    return UNITY_END();
}
