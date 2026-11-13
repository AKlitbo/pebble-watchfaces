/**
 * @file layout_places.spec.c
 * @brief Host tests for reading which panels a layout string places.
 *
 * The faces start the phone's battery readings for a panel placed on any layout, so a record read
 * wrong either leaves a placed panel empty or keeps the watch waking for a panel nobody placed. Worth
 * pinning are that every record counts, not only the first, and that only a record's first field is
 * its panel.
 *
 * Run from the unit's folder, with the framework's Unity:
 *   cc -std=c11 -Wall -Wextra -I<framework>/tests/c/spec/unity -Ipaf/c/core -Icore/c \
 *      core/c/layout_places.spec.c core/c/layout_places.c paf/c/core/layout/layout_string.c \
 *      <framework>/tests/c/spec/unity/unity.c && ./a.out
 */
#include "unity.h"

#include "layout_places.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A panel placed after the first record still counts, or a second panel would never start its readings. */
void test_layout_places_module_finds_a_later_record(void)
{
    bool result = layout_places_module("2,0,0,2,2;76,3,0,2,1", 76);

    TEST_ASSERT_TRUE(result);
}

/** @brief A row, column, or size that matches the panel's number is not the panel. */
void test_layout_places_module_ignores_the_place_fields(void)
{
    bool result = layout_places_module("3,1,2,2,1;5,2,0,4,1", 2);

    TEST_ASSERT_FALSE(result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_layout_places_module_finds_a_later_record);
    RUN_TEST(test_layout_places_module_ignores_the_place_fields);
    return UNITY_END();
}
