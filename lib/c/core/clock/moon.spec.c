/**
 * @file moon.spec.c
 * @brief Host tests for the pure moon-phase math.
 *
 * The whole readout hangs off one mean synodic month counted from a known new moon.
 * The tricky parts are the wrap for dates before the epoch and the rounding that
 * snaps the age onto one of eight glyphs, so those are what these tests pin down.
 */
#include "unity.h"

#include "clock/moon.h"

// the epoch + exactly half a lunation lands on the full moon
#define MOON_HALF_SEC (MOON_SYNODIC_SEC / 2)

void setUp(void) {}
void tearDown(void) {}

/** @brief Right on the epoch the age is 0, the anchor every other reading counts from. */
void test_moon_age_at_epoch_is_zero(void)
{
    int32_t result = moon_age_sec(MOON_EPOCH_UTC);

    TEST_ASSERT_EQUAL_INT32(0, result);
}

/** @brief Without the wrap a date before the epoch would report a negative age and read as new forever. */
void test_moon_age_before_epoch_stays_positive(void)
{
    int32_t result = moon_age_sec(MOON_EPOCH_UTC - 100);

    TEST_ASSERT_EQUAL_INT32(MOON_SYNODIC_SEC - 100, result);
}

/** @brief One full lunation later the age must fold back to 0 or the phase would drift off the ring. */
void test_moon_age_wraps_after_full_cycle(void)
{
    int32_t result = moon_age_sec(MOON_EPOCH_UTC + MOON_SYNODIC_SEC);

    TEST_ASSERT_EQUAL_INT32(0, result);
}

/** @brief The new moon must land on glyph 0, else the whole phase strip is shifted. */
void test_moon_glyph_at_epoch_is_new(void)
{
    int result = moon_glyph_index(MOON_EPOCH_UTC, 8);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A count of 0 must return a safe 0 rather than divide by zero and crash the watch. */
void test_moon_glyph_nonpositive_count_is_zero(void)
{
    int result = moon_glyph_index(MOON_EPOCH_UTC, 0);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Late in the cycle the rounding can reach the glyph count, and that must wrap back to new. */
void test_moon_glyph_wraps_back_to_new_at_end_of_ring(void)
{
    int result = moon_glyph_index(MOON_EPOCH_UTC + 2500000, 8);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief The new moon is 0 percent lit, the floor of the illumination triangle. */
void test_moon_illumination_is_zero_at_new(void)
{
    int result = moon_illumination_pct(MOON_EPOCH_UTC);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Half a lunation in is the full moon and must read a clean 100 percent. */
void test_moon_illumination_is_full_at_half_cycle(void)
{
    int result = moon_illumination_pct(MOON_EPOCH_UTC + MOON_HALF_SEC);

    TEST_ASSERT_EQUAL_INT(100, result);
}

/**
 * @brief Past the full moon the percent has to fall again, or every waning day reads mirrored.
 *
 * The illumination is a triangle up to full and back down, and the two readings either side of it
 * take different branches. New moon and full moon both land on the way up, so without this the
 * whole downslope goes unread and a fortnight of the moon panel could be inside out.
 *
 * The maths truncates, so the last quarter lands one short of a clean 50.
 */
void test_moon_illumination_falls_back_after_the_full_moon(void)
{
    // three quarters of the way round the cycle is the last quarter
    // so it is half lit and already on the way down
    int result = moon_illumination_pct(MOON_EPOCH_UTC + MOON_SYNODIC_SEC - MOON_HALF_SEC / 2);

    TEST_ASSERT_EQUAL_INT(49, result);
}

/** @brief The name at the epoch must be the new moon, the label that pairs with glyph 0. */
void test_moon_phase_name_at_epoch_is_new(void)
{
    const char *result = moon_phase_name(MOON_EPOCH_UTC);

    TEST_ASSERT_EQUAL_STRING("NEW", result);
}

/** @brief At half a lunation the name must be the full moon so the word matches the picture. */
void test_moon_phase_name_at_half_cycle_is_full(void)
{
    const char *result = moon_phase_name(MOON_EPOCH_UTC + MOON_HALF_SEC);

    TEST_ASSERT_EQUAL_STRING("FULL", result);
}

/** @brief At the new moon the full moon is half a cycle out, about 15 days, not 0. */
void test_moon_days_to_full_at_new_is_15(void)
{
    int result = moon_days_to_phase(MOON_EPOCH_UTC, true);

    TEST_ASSERT_EQUAL_INT(15, result);
}

/** @brief Right on the full moon the wait must read 0 or the panel counts to a moon already up. */
void test_moon_days_to_full_at_full_is_zero(void)
{
    int result = moon_days_to_phase(MOON_EPOCH_UTC + MOON_HALF_SEC, true);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief A day past the full moon must wrap to the next one, not report a negative wait. */
void test_moon_days_to_full_wraps_past_the_target(void)
{
    int result = moon_days_to_phase(MOON_EPOCH_UTC + MOON_HALF_SEC + 86400, true);

    TEST_ASSERT_EQUAL_INT(29, result);
}

/** @brief At the full moon the next new moon is the other half of the cycle away. */
void test_moon_days_to_new_at_full_is_15(void)
{
    int result = moon_days_to_phase(MOON_EPOCH_UTC + MOON_HALF_SEC, false);

    TEST_ASSERT_EQUAL_INT(15, result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_moon_age_at_epoch_is_zero);
    RUN_TEST(test_moon_age_before_epoch_stays_positive);
    RUN_TEST(test_moon_age_wraps_after_full_cycle);
    RUN_TEST(test_moon_glyph_at_epoch_is_new);
    RUN_TEST(test_moon_glyph_nonpositive_count_is_zero);
    RUN_TEST(test_moon_glyph_wraps_back_to_new_at_end_of_ring);
    RUN_TEST(test_moon_illumination_is_zero_at_new);
    RUN_TEST(test_moon_illumination_is_full_at_half_cycle);
    RUN_TEST(test_moon_illumination_falls_back_after_the_full_moon);
    RUN_TEST(test_moon_phase_name_at_epoch_is_new);
    RUN_TEST(test_moon_phase_name_at_half_cycle_is_full);
    RUN_TEST(test_moon_days_to_full_at_new_is_15);
    RUN_TEST(test_moon_days_to_full_at_full_is_zero);
    RUN_TEST(test_moon_days_to_full_wraps_past_the_target);
    RUN_TEST(test_moon_days_to_new_at_full_is_15);

    return UNITY_END();
}
