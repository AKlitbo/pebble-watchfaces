/**
 * @file series.spec.c
 * @brief Host tests for the sample-series reductions.
 *
 * The heart-rate sparkline and the steps bars both feed off these, so the cases worth pinning are
 * the ones a plain loop gets wrong: a gap in the middle must not count as a reading, an all-empty
 * history must leave the caller's seeded values alone, and "most recent" has to mean the last real
 * slot rather than the last slot.
 */
#include "unity.h"

#include "math/series.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A clean run reads its own low, high and last, with 0 marking the empty slots. */
void test_range_reads_low_high_and_last(void)
{
    const uint8_t samples[] = {60, 72, 55, 80};
    int lo = -1;
    int hi = -1;
    int last = -1;

    int valid = series_range(samples, 4, 0, &lo, &hi, &last);

    TEST_ASSERT_EQUAL_INT(4, valid);
    TEST_ASSERT_EQUAL_INT(55, lo);
    TEST_ASSERT_EQUAL_INT(80, hi);
    TEST_ASSERT_EQUAL_INT(80, last);
}

/** @brief A gap is skipped and "last" is the last real slot, or an end-to-end loop reads the trailing empty slot's zero and the low collapses to it. */
void test_range_skips_the_gaps(void)
{
    const uint8_t samples[] = {60, 0, 75, 0};
    int lo = -1;
    int hi = -1;
    int last = -1;

    int valid = series_range(samples, 4, 0, &lo, &hi, &last);

    TEST_ASSERT_EQUAL_INT(2, valid);
    TEST_ASSERT_EQUAL_INT(60, lo);
    TEST_ASSERT_EQUAL_INT(75, hi);
    TEST_ASSERT_EQUAL_INT(75, last);
}

/** @brief One real reading is a valid series: low, high and last are all that reading. */
void test_range_reads_a_lone_sample(void)
{
    const uint8_t samples[] = {0, 0, 66, 0};
    int lo = -1;
    int hi = -1;
    int last = -1;

    int valid = series_range(samples, 4, 0, &lo, &hi, &last);

    TEST_ASSERT_EQUAL_INT(1, valid);
    TEST_ASSERT_EQUAL_INT(66, lo);
    TEST_ASSERT_EQUAL_INT(66, hi);
    TEST_ASSERT_EQUAL_INT(66, last);
}

/** @brief An all-empty history leaves the caller's seeded outputs untouched, or a stray 0 makes a blank panel read as a real zero. */
void test_range_leaves_outputs_alone_when_empty(void)
{
    const uint8_t samples[] = {0, 0, 0};
    int lo = 7;
    int hi = 7;
    int last = 7;

    int valid = series_range(samples, 3, 0, &lo, &hi, &last);

    TEST_ASSERT_EQUAL_INT(0, valid);
    TEST_ASSERT_EQUAL_INT(7, lo);
    TEST_ASSERT_EQUAL_INT(7, hi);
    TEST_ASSERT_EQUAL_INT(7, last);
}

/** @brief The busiest slot is what the bars scale against. */
void test_max_reads_the_busiest_slot(void)
{
    const uint16_t values[] = {120, 3400, 900, 50};

    int result = series_max_u16(values, 4);

    TEST_ASSERT_EQUAL_INT(3400, result);
}

/** @brief An empty series has no busiest slot, so it scales against zero. */
void test_max_of_nothing_is_zero(void)
{
    const uint16_t values[] = {0};

    int result = series_max_u16(values, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_range_reads_low_high_and_last);
    RUN_TEST(test_range_skips_the_gaps);
    RUN_TEST(test_range_reads_a_lone_sample);
    RUN_TEST(test_range_leaves_outputs_alone_when_empty);
    RUN_TEST(test_max_reads_the_busiest_slot);
    RUN_TEST(test_max_of_nothing_is_zero);

    return UNITY_END();
}
