/**
 * @file scale.spec.c
 * @brief Host tests for the bar and gauge maths.
 *
 * These are the sums a battery, a gauge, and a graph all lean on, so a wrong one shows up as a bar
 * that overfills or a trace pinned to the wrong edge. The cases worth pinning are the guards that
 * keep a divide safe and the rounding choices that decide whether the first cell ever lights.
 */
#include "unity.h"

#include "math/scale.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A value inside the range is left where it is. */
void test_clamp_leaves_an_inside_value(void)
{
    int result = clamp_int(50, 0, 100);

    TEST_ASSERT_EQUAL_INT(50, result);
}

/** @brief Below the low end pins to the low end. */
void test_clamp_pins_below(void)
{
    int result = clamp_int(-10, 0, 100);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Above the high end pins to the high end. */
void test_clamp_pins_above(void)
{
    int result = clamp_int(150, 0, 100);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/** @brief The gaps sit between the cells, so five cells over 100 with a 2px gap leaves 18 each. */
void test_segment_width_accounts_for_the_gaps(void)
{
    int result = segment_width(100, 2, 5);

    TEST_ASSERT_EQUAL_INT(18, result);
}

/** @brief No gaps is just the plain division. */
void test_segment_width_with_no_gaps(void)
{
    int result = segment_width(100, 0, 4);

    TEST_ASSERT_EQUAL_INT(25, result);
}

/** @brief A count of zero must answer 0 rather than divide by it. */
void test_segment_width_guards_a_zero_count(void)
{
    int result = segment_width(100, 2, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Half of a length is half its pixels. */
void test_fraction_px_halfway(void)
{
    int result = fraction_px(60, 30, 60);

    TEST_ASSERT_EQUAL_INT(30, result);
}

/** @brief A denominator of zero must answer 0 rather than divide by it. */
void test_fraction_px_guards_a_zero_denominator(void)
{
    int result = fraction_px(60, 30, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Any charge at all lights a cell, since a near-empty gauge that reads empty looks broken. */
void test_segments_filled_rounds_the_first_cell_up(void)
{
    int result = segments_filled(1, 10);

    TEST_ASSERT_EQUAL_INT(1, result);
}

/** @brief A full level lights every cell. */
void test_segments_filled_fills_at_one_hundred(void)
{
    int result = segments_filled(100, 10);

    TEST_ASSERT_EQUAL_INT(10, result);
}

/** @brief A level past 100 is pinned first, so it cannot light more cells than there are. */
void test_segments_filled_caps_over_full(void)
{
    int result = segments_filled(150, 10);

    TEST_ASSERT_EQUAL_INT(10, result);
}

/** @brief The bottom of the window lands on the bottom row of the area. */
void test_plot_y_puts_the_low_end_at_the_bottom(void)
{
    int result = plot_y(0, 50, 40, 120, 40);

    TEST_ASSERT_EQUAL_INT(49, result);
}

/** @brief The top of the window lands on the top row of the area. */
void test_plot_y_puts_the_high_end_at_the_top(void)
{
    int result = plot_y(0, 50, 40, 120, 120);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A window with no height cannot map anything, so it falls back to the bottom row. */
void test_plot_y_guards_a_flat_window(void)
{
    int result = plot_y(0, 50, 40, 40, 40);

    TEST_ASSERT_EQUAL_INT(49, result);
}

/** @brief A value above the window pins to the top row rather than mapping off-area. */
void test_plot_y_pins_a_value_above_the_window(void)
{
    int result = plot_y(0, 50, 40, 120, 200);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A value below the window pins to the bottom row rather than mapping off-area. */
void test_plot_y_pins_a_value_below_the_window(void)
{
    int result = plot_y(0, 50, 40, 120, 10);

    TEST_ASSERT_EQUAL_INT(49, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_clamp_leaves_an_inside_value);
    RUN_TEST(test_clamp_pins_below);
    RUN_TEST(test_clamp_pins_above);
    RUN_TEST(test_segment_width_accounts_for_the_gaps);
    RUN_TEST(test_segment_width_with_no_gaps);
    RUN_TEST(test_segment_width_guards_a_zero_count);
    RUN_TEST(test_fraction_px_halfway);
    RUN_TEST(test_fraction_px_guards_a_zero_denominator);
    RUN_TEST(test_segments_filled_rounds_the_first_cell_up);
    RUN_TEST(test_segments_filled_fills_at_one_hundred);
    RUN_TEST(test_segments_filled_caps_over_full);
    RUN_TEST(test_plot_y_puts_the_low_end_at_the_bottom);
    RUN_TEST(test_plot_y_puts_the_high_end_at_the_top);
    RUN_TEST(test_plot_y_guards_a_flat_window);
    RUN_TEST(test_plot_y_pins_a_value_above_the_window);
    RUN_TEST(test_plot_y_pins_a_value_below_the_window);

    return UNITY_END();
}
