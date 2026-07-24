/**
 * @file beats.spec.c
 * @brief Host tests for the pure .beats clock math.
 *
 * The .beats readout splits the day into 1000 equal parts off Biel time. If the
 * division or the day wrap drifts the whole clock reads the wrong number all day,
 * so the boundaries are what these tests pin down.
 */
#include "unity.h"

#include "clock/beats.h"

void setUp(void) {}
void tearDown(void) {}

/** @brief A broken divisor would drift the .beats readout away from the real time of day. */
void test_beats_start_of_day_is_zero(void)
{
    int result = beats_from_ms(0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief If the last beat rounded up the display would flash a phantom 1000 before midnight. */
void test_beats_last_millisecond_of_day_is_999(void)
{
    int result = beats_from_ms(MS_PER_DAY - 1);

    TEST_ASSERT_EQUAL_INT(999, result);
}

/** @brief A missing day wrap would push .beats past 999 and freeze the clock after midnight. */
void test_beats_wraps_at_day_boundary(void)
{
    int result = beats_from_ms(MS_PER_DAY);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief One full beat in must read 1, else every beat lands one place early or late. */
void test_beats_one_beat_in_is_one(void)
{
    int result = beats_from_ms(MS_PER_BEAT);

    TEST_ASSERT_EQUAL_INT(1, result);
}

/** @brief A negative offset (clock not set yet) must clamp to 0 rather than read garbage. */
void test_beats_negative_clamps_to_zero(void)
{
    int result = beats_from_ms(-5);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Right on a boundary the next flip is a whole beat away, never 0, or a timer fires forever. */
void test_ms_until_next_beat_on_boundary_is_full_beat(void)
{
    uint32_t result = ms_until_next_beat(0);

    TEST_ASSERT_EQUAL_UINT32((uint32_t)MS_PER_BEAT, result);
}

/** @brief One ms into a beat leaves a beat minus one, else the beats timer wakes at the wrong moment. */
void test_ms_until_next_beat_mid_beat_is_remainder(void)
{
    uint32_t result = ms_until_next_beat(1);

    TEST_ASSERT_EQUAL_UINT32((uint32_t)MS_PER_BEAT - 1, result);
}

/** @brief One ms before a flip must return 1 so the timer fires right on the boundary. */
void test_ms_until_next_beat_one_before_flip_is_one(void)
{
    uint32_t result = ms_until_next_beat(MS_PER_BEAT - 1);

    TEST_ASSERT_EQUAL_UINT32(1, result);
}

/** @brief BMT is an hour ahead, so UTC midnight is already an hour into the BMT day. */
void test_ms_from_hms_offsets_by_an_hour(void)
{
    int32_t result = beats_ms_from_hms(0, 0, 0, 0);

    TEST_ASSERT_EQUAL_INT32(3600 * 1000, result);
}

/**
 * @brief The BMT day turns over at 23:00 UTC, and the maths has to turn over with it.
 *
 * The one boundary in the function. An hour before midnight UTC is midnight in Biel, so this must
 * read zero rather than running on to 24 hours worth of ms. Without the wrap the .beats readout
 * spends the last hour of every day counting past 999.
 */
void test_ms_from_hms_wraps_at_the_bmt_day_turn(void)
{
    int32_t result = beats_ms_from_hms(23, 0, 0, 0);

    TEST_ASSERT_EQUAL_INT32(0, result);
}

/** @brief A second past the turn is a second into the new BMT day. */
void test_ms_from_hms_just_past_the_turn(void)
{
    int32_t result = beats_ms_from_hms(23, 0, 1, 0);

    TEST_ASSERT_EQUAL_INT32(1000, result);
}

/**
 * @brief The last millisecond of the BMT day must fit, which is what keeps this in an int32.
 *
 * 22:59:59.999 UTC is the top of the BMT day at 86,399,999 ms. The whole readout is int32 maths on
 * the strength of that number staying under 86.4 million, and dropping to 64 bit division to hold
 * a bigger one would cost the image hundreds of bytes it does not have.
 */
void test_ms_from_hms_at_the_top_of_the_day_still_fits(void)
{
    int32_t result = beats_ms_from_hms(22, 59, 59, 999);

    TEST_ASSERT_EQUAL_INT32(86399999, result);
}

/** @brief The sub-second part rides through, since a beat boundary falls mid-second. */
void test_ms_from_hms_keeps_the_milliseconds(void)
{
    int32_t result = beats_ms_from_hms(0, 0, 0, 500);

    TEST_ASSERT_EQUAL_INT32(3600 * 1000 + 500, result);
}

/** @brief A whole day of it lines up with the .beats maths next door, end to end. */
void test_ms_from_hms_feeds_the_beats_maths(void)
{
    // 22:59:59.999 UTC is the last instant of the BMT day, so the last beat of it
    int result = beats_from_ms(beats_ms_from_hms(22, 59, 59, 999));

    TEST_ASSERT_EQUAL_INT(999, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_beats_start_of_day_is_zero);
    RUN_TEST(test_beats_last_millisecond_of_day_is_999);
    RUN_TEST(test_beats_wraps_at_day_boundary);
    RUN_TEST(test_beats_one_beat_in_is_one);
    RUN_TEST(test_beats_negative_clamps_to_zero);
    RUN_TEST(test_ms_until_next_beat_on_boundary_is_full_beat);
    RUN_TEST(test_ms_until_next_beat_mid_beat_is_remainder);
    RUN_TEST(test_ms_until_next_beat_one_before_flip_is_one);
    RUN_TEST(test_ms_from_hms_offsets_by_an_hour);
    RUN_TEST(test_ms_from_hms_wraps_at_the_bmt_day_turn);
    RUN_TEST(test_ms_from_hms_just_past_the_turn);
    RUN_TEST(test_ms_from_hms_at_the_top_of_the_day_still_fits);
    RUN_TEST(test_ms_from_hms_keeps_the_milliseconds);
    RUN_TEST(test_ms_from_hms_feeds_the_beats_maths);

    return UNITY_END();
}
