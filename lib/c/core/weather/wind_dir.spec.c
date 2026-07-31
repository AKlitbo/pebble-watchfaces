/**
 * @file wind_dir.spec.c
 * @brief Host tests for the compass abbreviation and the lean it produces.
 *
 * The cases worth pinning are the ones a careless table would get wrong: that every one of the
 * sixteen points is present and in order, that "S" is not matched by the "SSE" row (a prefix
 * compare would), and that the component has the right sign. That last one is easy to invert and
 * hard to notice, since a wrong sign still produces a plausible-looking number.
 */
#include "unity.h"

#include "weather/wind_dir.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief North is the zero the rest of the compass is measured from. */
void test_north_is_zero(void)
{
    int result = wind_bearing("N");

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief And the quarters land where the compass says. */
void test_quarters_are_a_right_angle_apart(void)
{
    TEST_ASSERT_EQUAL_INT(90, wind_bearing("E"));
    TEST_ASSERT_EQUAL_INT(180, wind_bearing("S"));
    TEST_ASSERT_EQUAL_INT(270, wind_bearing("W"));
}

/** @brief The three-letter points sit halfway between their neighbours. */
void test_three_letter_points_split_the_gap(void)
{
    TEST_ASSERT_EQUAL_INT(22, wind_bearing("NNE"));
    TEST_ASSERT_EQUAL_INT(67, wind_bearing("ENE"));
    TEST_ASSERT_EQUAL_INT(337, wind_bearing("NNW"));
}

/** @brief The phone's casing is not guaranteed, so matching must not depend on it. */
void test_matching_ignores_case(void)
{
    int result = wind_bearing("wsw");

    TEST_ASSERT_EQUAL_INT(wind_bearing("WSW"), result);
}

/**
 * @brief A short point is not matched by a longer row that starts with it.
 *
 * A prefix compare would answer "SSE" for "S" and put the wind 22 degrees out.
 */
void test_a_short_point_is_not_a_prefix_match(void)
{
    int result = wind_bearing("S");

    TEST_ASSERT_EQUAL_INT(180, result);
}

/** @brief Anything not on the compass is no reading, not north. */
void test_an_unknown_direction_is_no_data(void)
{
    TEST_ASSERT_EQUAL_INT(-1, wind_bearing("NNNW"));
    TEST_ASSERT_EQUAL_INT(-1, wind_bearing("up"));
    TEST_ASSERT_EQUAL_INT(-1, wind_bearing(""));
    TEST_ASSERT_EQUAL_INT(-1, wind_bearing(NULL));
}

/** @brief Every one of the sixteen points resolves, and no two share a bearing. */
void test_all_sixteen_points_are_distinct(void)
{
    static const char *const points[] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW",
    };

    int seen[16];
    for (int i = 0; i < 16; i++)
    {
        seen[i] = wind_bearing(points[i]);
        TEST_ASSERT_TRUE(seen[i] >= 0);

        for (int j = 0; j < i; j++)
        {
            TEST_ASSERT_TRUE(seen[i] != seen[j]);
        }
    }
}

/** @brief A wind out of the west runs eastward, which is the way it is going. */
void test_a_westerly_leans_right(void)
{
    int result = wind_lean(wind_bearing("W"));

    TEST_ASSERT_EQUAL_INT(100, result);
}

/** @brief And an easterly the other way. */
void test_an_easterly_leans_left(void)
{
    int result = wind_lean(wind_bearing("E"));

    TEST_ASSERT_EQUAL_INT(-100, result);
}

/** @brief A wind along the line of sight has no sideways part to show. */
void test_a_northerly_leans_neither_way(void)
{
    TEST_ASSERT_EQUAL_INT(0, wind_lean(wind_bearing("N")));
    TEST_ASSERT_EQUAL_INT(0, wind_lean(wind_bearing("S")));
}

/** @brief A corner of the compass carries some of it, but less than a wind straight across. */
void test_a_diagonal_leans_less_than_a_crosswind(void)
{
    int diagonal = wind_lean(wind_bearing("NW"));
    int across = wind_lean(wind_bearing("W"));

    TEST_ASSERT_TRUE(diagonal > 0);
    TEST_ASSERT_TRUE(diagonal < across);
}

/** @brief No reading is zero, rather than being read as north. */
void test_no_bearing_leans_nothing(void)
{
    int result = wind_lean(-1);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief It is a proportion and must never leave its range. */
void test_lean_stays_within_range(void)
{
    for (int bearing = 0; bearing < 360; bearing++)
    {
        int lean = wind_lean(bearing);
        TEST_ASSERT_TRUE(lean >= -100 && lean <= 100);
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_north_is_zero);
    RUN_TEST(test_quarters_are_a_right_angle_apart);
    RUN_TEST(test_three_letter_points_split_the_gap);
    RUN_TEST(test_matching_ignores_case);
    RUN_TEST(test_a_short_point_is_not_a_prefix_match);
    RUN_TEST(test_an_unknown_direction_is_no_data);
    RUN_TEST(test_all_sixteen_points_are_distinct);
    RUN_TEST(test_a_westerly_leans_right);
    RUN_TEST(test_an_easterly_leans_left);
    RUN_TEST(test_a_northerly_leans_neither_way);
    RUN_TEST(test_a_diagonal_leans_less_than_a_crosswind);
    RUN_TEST(test_no_bearing_leans_nothing);
    RUN_TEST(test_lean_stays_within_range);

    return UNITY_END();
}
