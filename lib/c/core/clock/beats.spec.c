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

#include <stddef.h>

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

/** @brief Without this the face never arms the .beats ticker and the date line sits stale. */
void test_has_token_spots_a_beats_date_format(void)
{
    bool result = beats_has_token("%m%d." BEATS_TOKEN);

    TEST_ASSERT_TRUE(result);
}

/** @brief An ordinary date format must not arm the ticker, or every face pays for a timer. */
void test_has_token_ignores_a_plain_date_format(void)
{
    bool result = beats_has_token("%Y.%m%d");

    TEST_ASSERT_FALSE(result);
}

/** @brief Settings can hand back NULL before the store is seeded, and that must not crash. */
void test_has_token_handles_null(void)
{
    bool result = beats_has_token(NULL);

    TEST_ASSERT_FALSE(result);
}

/** @brief The whole point: the token leaves and a reading takes its place. */
void test_expand_writes_the_reading_over_the_token(void)
{
    char text[16] = "0618." BEATS_TOKEN;

    beats_expand_token(text, 672);

    TEST_ASSERT_EQUAL_STRING("0618.672", text);
}

/** @brief A low reading keeps its padding, else the date line changes width as the day turns. */
void test_expand_pads_a_low_reading_to_three_digits(void)
{
    char text[16] = "0618." BEATS_TOKEN;

    beats_expand_token(text, 7);

    TEST_ASSERT_EQUAL_STRING("0618.007", text);
}

/** @brief Midnight BMT reads @000 rather than an empty gap where the token was. */
void test_expand_writes_zero_as_three_zeroes(void)
{
    char text[16] = BEATS_TOKEN;

    beats_expand_token(text, 0);

    TEST_ASSERT_EQUAL_STRING("000", text);
}

/** @brief Text either side of the token has to survive, since the date is built around it. */
void test_expand_leaves_the_rest_of_the_string_alone(void)
{
    char text[24] = "SOL " BEATS_TOKEN " MARK";

    beats_expand_token(text, 500);

    TEST_ASSERT_EQUAL_STRING("SOL 500 MARK", text);
}

/** @brief A tokenless string must come back untouched, not truncated or part-written. */
void test_expand_leaves_a_tokenless_string_untouched(void)
{
    char text[16] = "2026.0618";

    beats_expand_token(text, 672);

    TEST_ASSERT_EQUAL_STRING("2026.0618", text);
}

/** @brief Every token gets filled, or a format using two would show a stray brace on screen. */
void test_expand_fills_every_token(void)
{
    char text[16] = BEATS_TOKEN "-" BEATS_TOKEN;

    beats_expand_token(text, 123);

    TEST_ASSERT_EQUAL_STRING("123-123", text);
}

/**
 * @brief An out of range reading is clamped, which is what keeps the swap a same-width one.
 *
 * beats_from_ms can only hand back 0 to 999, so this is guarding the promise rather than a
 * reading anyone expects. A four digit number written into a three character token would run
 * over whatever follows it in the buffer.
 */
void test_expand_clamps_a_reading_past_the_top_of_the_day(void)
{
    char text[16] = "X" BEATS_TOKEN "Y";

    beats_expand_token(text, 1500);

    TEST_ASSERT_EQUAL_STRING("X999Y", text);
}

/** @brief A negative reading clamps too, rather than writing a minus sign into the date. */
void test_expand_clamps_a_negative_reading(void)
{
    char text[16] = "X" BEATS_TOKEN "Y";

    beats_expand_token(text, -5);

    TEST_ASSERT_EQUAL_STRING("X000Y", text);
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

    RUN_TEST(test_has_token_spots_a_beats_date_format);
    RUN_TEST(test_has_token_ignores_a_plain_date_format);
    RUN_TEST(test_has_token_handles_null);
    RUN_TEST(test_expand_writes_the_reading_over_the_token);
    RUN_TEST(test_expand_pads_a_low_reading_to_three_digits);
    RUN_TEST(test_expand_writes_zero_as_three_zeroes);
    RUN_TEST(test_expand_leaves_the_rest_of_the_string_alone);
    RUN_TEST(test_expand_leaves_a_tokenless_string_untouched);
    RUN_TEST(test_expand_fills_every_token);
    RUN_TEST(test_expand_clamps_a_reading_past_the_top_of_the_day);
    RUN_TEST(test_expand_clamps_a_negative_reading);

    return UNITY_END();
}
