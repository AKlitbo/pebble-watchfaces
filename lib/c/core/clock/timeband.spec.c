/**
 * @file timeband.spec.c
 * @brief Host tests for the day-as-an-axis window.
 *
 * Almost every case worth pinning here has a midnight or a boundary in it. A window that opens in
 * the evening and runs into the next afternoon is the normal case for this face, not the odd one,
 * so the wrapping is tested first and hardest. The other half is the clipping: an event that began
 * before the window opened and one that runs past its end both have to draw, and each has to say
 * which end it ran off.
 */
#include "unity.h"

#include "clock/timeband.h"

void setUp(void) {}
void tearDown(void) {}

// a window that opens at 22:00 and runs sixteen hours, so it holds a midnight
#define EVENING_START 1320
#define EVENING_SPAN 960

// a four hour window anchored at epoch zero, so a clipped span's minutes read straight off
#define CLIP_SPAN 240
#define CLIP_END_EPOCH (CLIP_SPAN * 60)

/** @brief A whole day runs midnight to midnight. */
void test_full_day_covers_the_day(void)
{
    TimeBand result = timeband_full_day();

    TEST_ASSERT_EQUAL_INT(0, result.start_min);
    TEST_ASSERT_EQUAL_INT(TIMEBAND_DAY_MINUTES, result.span_min);
}

/** @brief Noon sits halfway along a whole-day axis. */
void test_pos_places_noon_halfway(void)
{
    int result = timeband_pos(timeband_full_day(), 240, 720);

    TEST_ASSERT_EQUAL_INT(120, result);
}

/** @brief The last minute of the day lands on the last unit, not past the end. */
void test_pos_last_minute_stays_inside(void)
{
    int result = timeband_pos(timeband_full_day(), 240, 1439);

    TEST_ASSERT_EQUAL_INT(239, result);
}

/** @brief An axis with no length cannot place anything. */
void test_pos_no_length_is_none(void)
{
    int result = timeband_pos(timeband_full_day(), 0, 720);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief A rolling window opens the requested distance before the moment it is built around. */
void test_rolling_puts_now_where_asked(void)
{
    TimeBand result = timeband_rolling(1380, EVENING_SPAN, 60);

    TEST_ASSERT_EQUAL_INT(EVENING_START, result.start_min);
    TEST_ASSERT_EQUAL_INT(EVENING_SPAN, result.span_min);
}

/** @brief An hour past midnight is measured from the previous evening, not from midnight. */
void test_offset_wraps_past_midnight(void)
{
    TimeBand band = {.start_min = EVENING_START, .span_min = EVENING_SPAN};

    int result = timeband_offset(band, 30);

    TEST_ASSERT_EQUAL_INT(150, result);
}

/** @brief A minute one short of the start is behind the window, not almost a whole day into it. */
void test_offset_just_before_the_start_is_none(void)
{
    TimeBand band = {.start_min = EVENING_START, .span_min = EVENING_SPAN};

    int result = timeband_offset(band, EVENING_START - 1);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief The window is half open, so the minute it ends on belongs to whatever comes next. */
void test_offset_at_the_end_is_none(void)
{
    TimeBand band = {.start_min = EVENING_START, .span_min = EVENING_SPAN};

    int result = timeband_offset(band, 840);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief A span of nothing is not a window, so it is pinned to a minute. */
void test_rolling_pins_a_span_of_nothing(void)
{
    TimeBand result = timeband_rolling(600, 0, 0);

    TEST_ASSERT_EQUAL_INT(1, result.span_min);
}

/** @brief Asking for a moment further in than the window is long pins it to the far end. */
void test_rolling_pins_a_lead_past_the_span(void)
{
    TimeBand result = timeband_rolling(600, 120, 500);

    TEST_ASSERT_EQUAL_INT(480, result.start_min);
}

/** @brief A window built from an hour opens on that hour. */
void test_from_hour_opens_on_the_hour(void)
{
    TimeBand result = timeband_from_hour(22, EVENING_SPAN);

    TEST_ASSERT_EQUAL_INT(EVENING_START, result.start_min);
}

/** @brief An hour past the end of the day wraps rather than running off it. */
void test_from_hour_wraps_a_bad_hour(void)
{
    TimeBand result = timeband_from_hour(26, 60);

    TEST_ASSERT_EQUAL_INT(120, result.start_min);
}

/** @brief A span ending with the window reaches the end of the axis rather than stopping short. */
void test_pos_offset_reaches_the_end(void)
{
    int result = timeband_pos_offset(timeband_full_day(), 240, TIMEBAND_DAY_MINUTES);

    TEST_ASSERT_EQUAL_INT(240, result);
}

/** @brief An offset past the window has nowhere to go. */
void test_pos_offset_past_the_window_is_none(void)
{
    int result = timeband_pos_offset(timeband_full_day(), 240, TIMEBAND_DAY_MINUTES + 1);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

/** @brief An event that finished before the window opened is not on it. */
void test_clip_before_the_window_is_none(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, -7200, -3600, &span);

    TEST_ASSERT_FALSE(result);
}

/** @brief Nor is one that starts after it closes. */
void test_clip_after_the_window_is_none(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, CLIP_END_EPOCH, CLIP_END_EPOCH + 3600, &span);

    TEST_ASSERT_FALSE(result);
}

/** @brief An event wholly inside reads as the minutes it covers, with neither end cut. */
void test_clip_inside_keeps_both_ends(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, 3600, 7200, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(60, span.from);
    TEST_ASSERT_EQUAL_INT(120, span.to);
    TEST_ASSERT_FALSE(span.clipped_start);
    TEST_ASSERT_FALSE(span.clipped_end);
}

/** @brief One already running when the window opened starts at nothing and says it was cut. */
void test_clip_straddling_the_start_says_so(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, -3600, 3600, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(0, span.from);
    TEST_ASSERT_EQUAL_INT(60, span.to);
    TEST_ASSERT_TRUE(span.clipped_start);
    TEST_ASSERT_FALSE(span.clipped_end);
}

/** @brief One still running when it closes reaches the end and says it was cut there. */
void test_clip_straddling_the_end_says_so(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, 10800, 18000, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(180, span.from);
    TEST_ASSERT_EQUAL_INT(CLIP_SPAN, span.to);
    TEST_ASSERT_FALSE(span.clipped_start);
    TEST_ASSERT_TRUE(span.clipped_end);
}

/** @brief An all-day event covers the window and reports both ends cut. */
void test_clip_covering_the_window_cuts_both_ends(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, -3600, 18000, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(0, span.from);
    TEST_ASSERT_EQUAL_INT(CLIP_SPAN, span.to);
    TEST_ASSERT_TRUE(span.clipped_start);
    TEST_ASSERT_TRUE(span.clipped_end);
}

/** @brief A moment someone pinned to their calendar still gets a minute to draw in. */
void test_clip_a_moment_is_a_minute_wide(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, 3600, 3600, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(60, span.from);
    TEST_ASSERT_EQUAL_INT(61, span.to);
    TEST_ASSERT_FALSE(span.clipped_end);
}

/** @brief An event ending exactly as the window opens is behind it, the same way an offset is. */
void test_clip_ending_on_the_start_is_none(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, -3600, 0, &span);

    TEST_ASSERT_FALSE(result);
}

/** @brief Tomorrow's event is on tomorrow's window, not wrapped onto today's. */
void test_clip_drops_tomorrows_event(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, 86400 + 3600, 86400 + 7200, &span);

    TEST_ASSERT_FALSE(result);
}

/** @brief An end before the start is read as having no length rather than running backwards. */
void test_clip_a_backwards_span_still_draws(void)
{
    TimeBand band = {.start_min = 0, .span_min = CLIP_SPAN};
    TimeBandSpan span = {0};

    bool result = timeband_clip(band, 0, 7200, 3600, &span);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_INT(120, span.from);
    TEST_ASSERT_EQUAL_INT(121, span.to);
}

/** @brief Daylight wholly inside a whole-day window is one piece. */
void test_clip_daily_inside_is_one_piece(void)
{
    TimeBandSpan pieces[2] = {0};

    int result = timeband_clip_daily(timeband_full_day(), 360, 1080, pieces, 2);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(360, pieces[0].from);
    TEST_ASSERT_EQUAL_INT(1080, pieces[0].to);
}

/** @brief A window opening at noon sees today's daylight and tomorrow's, earliest first. */
void test_clip_daily_across_midnight_is_two_pieces(void)
{
    TimeBand band = {.start_min = 720, .span_min = TIMEBAND_DAY_MINUTES};
    TimeBandSpan pieces[2] = {0};

    int result = timeband_clip_daily(band, 360, 1080, pieces, 2);

    TEST_ASSERT_EQUAL_INT(2, result);
    TEST_ASSERT_EQUAL_INT(0, pieces[0].from);
    TEST_ASSERT_EQUAL_INT(360, pieces[0].to);
    TEST_ASSERT_TRUE(pieces[0].clipped_start);
    TEST_ASSERT_EQUAL_INT(1080, pieces[1].from);
    TEST_ASSERT_EQUAL_INT(TIMEBAND_DAY_MINUTES, pieces[1].to);
    TEST_ASSERT_TRUE(pieces[1].clipped_end);
}

/** @brief A span that wraps midnight itself, like the night, comes back as its two ends. */
void test_clip_daily_a_wrapping_span_is_its_two_ends(void)
{
    TimeBandSpan pieces[2] = {0};

    int result = timeband_clip_daily(timeband_full_day(), 1080, 360, pieces, 2);

    TEST_ASSERT_EQUAL_INT(2, result);
    TEST_ASSERT_EQUAL_INT(0, pieces[0].from);
    TEST_ASSERT_EQUAL_INT(360, pieces[0].to);
    TEST_ASSERT_EQUAL_INT(1080, pieces[1].from);
    TEST_ASSERT_EQUAL_INT(TIMEBAND_DAY_MINUTES, pieces[1].to);
}

/** @brief Sunrise and sunset landing on the same minute is a sun that never sets. */
void test_clip_daily_same_minute_is_the_whole_day(void)
{
    TimeBandSpan pieces[2] = {0};

    int result = timeband_clip_daily(timeband_full_day(), 360, 360, pieces, 2);

    TEST_ASSERT_EQUAL_INT(2, result);
    TEST_ASSERT_EQUAL_INT(0, pieces[0].from);
    TEST_ASSERT_EQUAL_INT(360, pieces[0].to);
    TEST_ASSERT_EQUAL_INT(360, pieces[1].from);
    TEST_ASSERT_EQUAL_INT(TIMEBAND_DAY_MINUTES, pieces[1].to);
}

/** @brief A missing reading is not a time, so there is nothing to shade. */
void test_clip_daily_no_reading_is_no_pieces(void)
{
    TimeBandSpan pieces[2] = {0};

    int result = timeband_clip_daily(timeband_full_day(), -1, 1080, pieces, 2);

    TEST_ASSERT_EQUAL_INT(0, result);
}

/** @brief Room for one piece takes the earlier one and leaves the rest. */
void test_clip_daily_honours_the_room_it_is_given(void)
{
    TimeBand band = {.start_min = 720, .span_min = TIMEBAND_DAY_MINUTES};
    TimeBandSpan pieces[1] = {0};

    int result = timeband_clip_daily(band, 360, 1080, pieces, 1);

    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_INT(0, pieces[0].from);
}

/** @brief A window that opened earlier today sits on today's midnight. */
void test_window_epoch_today(void)
{
    TimeBand band = {.start_min = 480, .span_min = 600};

    time_t result = timeband_window_epoch(band, 86400, 600);

    TEST_ASSERT_EQUAL_INT(86400 + 480 * 60, (int)result);
}

/** @brief One that opens later in the day than it is now opened yesterday. */
void test_window_epoch_yesterday_when_it_wrapped(void)
{
    TimeBand band = {.start_min = EVENING_START, .span_min = EVENING_SPAN};

    time_t result = timeband_window_epoch(band, 86400, 30);

    TEST_ASSERT_EQUAL_INT(86400 + EVENING_START * 60 - 86400, (int)result);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_full_day_covers_the_day);
    RUN_TEST(test_pos_places_noon_halfway);
    RUN_TEST(test_pos_last_minute_stays_inside);
    RUN_TEST(test_pos_no_length_is_none);
    RUN_TEST(test_rolling_puts_now_where_asked);
    RUN_TEST(test_offset_wraps_past_midnight);
    RUN_TEST(test_offset_just_before_the_start_is_none);
    RUN_TEST(test_offset_at_the_end_is_none);
    RUN_TEST(test_rolling_pins_a_span_of_nothing);
    RUN_TEST(test_rolling_pins_a_lead_past_the_span);
    RUN_TEST(test_from_hour_opens_on_the_hour);
    RUN_TEST(test_from_hour_wraps_a_bad_hour);
    RUN_TEST(test_pos_offset_reaches_the_end);
    RUN_TEST(test_pos_offset_past_the_window_is_none);
    RUN_TEST(test_clip_before_the_window_is_none);
    RUN_TEST(test_clip_after_the_window_is_none);
    RUN_TEST(test_clip_inside_keeps_both_ends);
    RUN_TEST(test_clip_straddling_the_start_says_so);
    RUN_TEST(test_clip_straddling_the_end_says_so);
    RUN_TEST(test_clip_covering_the_window_cuts_both_ends);
    RUN_TEST(test_clip_a_moment_is_a_minute_wide);
    RUN_TEST(test_clip_ending_on_the_start_is_none);
    RUN_TEST(test_clip_drops_tomorrows_event);
    RUN_TEST(test_clip_a_backwards_span_still_draws);
    RUN_TEST(test_clip_daily_inside_is_one_piece);
    RUN_TEST(test_clip_daily_across_midnight_is_two_pieces);
    RUN_TEST(test_clip_daily_a_wrapping_span_is_its_two_ends);
    RUN_TEST(test_clip_daily_same_minute_is_the_whole_day);
    RUN_TEST(test_clip_daily_no_reading_is_no_pieces);
    RUN_TEST(test_clip_daily_honours_the_room_it_is_given);
    RUN_TEST(test_window_epoch_today);
    RUN_TEST(test_window_epoch_yesterday_when_it_wrapped);

    return UNITY_END();
}
