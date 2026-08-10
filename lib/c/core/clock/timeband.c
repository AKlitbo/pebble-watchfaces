/**
 * @file timeband.c
 * @brief The window sums: build one, place a moment on it, and clip a span to it.
 */
#include "clock/timeband.h"

#include "math/scale.h"

/// Seconds in a minute, so the epoch maths reads as what it is
#define SECONDS_PER_MINUTE 60

/**
 * @brief Bring any minute count into 0..1439, counting backwards from midnight for a negative one.
 *
 * @param minutes The count to wrap.
 * @return The same moment of the day, 0 to 1439.
 */
static int wrap_day(int minutes)
{
    int wrapped = minutes % TIMEBAND_DAY_MINUTES;
    return wrapped < 0 ? wrapped + TIMEBAND_DAY_MINUTES : wrapped;
}

/**
 * @brief Whole minutes in a second count, rounding down so a part minute before the window still
 * counts as the minute it started in.
 *
 * @param seconds The count, which may be negative.
 * @return The count in minutes.
 */
static int floor_minutes(long long seconds)
{
    long long minutes = seconds / SECONDS_PER_MINUTE;
    if (seconds % SECONDS_PER_MINUTE != 0 && seconds < 0)
    {
        minutes--;
    }
    return (int)minutes;
}

/**
 * @brief Whole minutes in a second count, rounding up so a span ending part way through a minute
 * still covers it.
 *
 * @param seconds The count, which may be negative.
 * @return The count in minutes.
 */
static int ceil_minutes(long long seconds)
{
    long long minutes = seconds / SECONDS_PER_MINUTE;
    if (seconds % SECONDS_PER_MINUTE != 0 && seconds > 0)
    {
        minutes++;
    }
    return (int)minutes;
}

TimeBand timeband_full_day(void)
{
    return (TimeBand){.start_min = 0, .span_min = TIMEBAND_DAY_MINUTES};
}

TimeBand timeband_rolling(int now_min, int span_min, int lead_min)
{
    int span = clamp_int(span_min, 1, TIMEBAND_DAY_MINUTES);
    int lead = clamp_int(lead_min, 0, span);

    return (TimeBand){.start_min = wrap_day(now_min - lead), .span_min = span};
}

TimeBand timeband_from_hour(int base_hour, int span_min)
{
    int hour = wrap_day(base_hour * 60) / 60;

    return (TimeBand){
        .start_min = hour * 60,
        .span_min = clamp_int(span_min, 1, TIMEBAND_DAY_MINUTES),
    };
}

int timeband_offset(TimeBand band, int minute_of_day)
{
    int offset = wrap_day(minute_of_day - band.start_min);

    // the far end belongs to whatever comes next, so two windows laid end to end share no minute
    return offset < band.span_min ? offset : -1;
}

int timeband_pos(TimeBand band, int length, int minute_of_day)
{
    if (length <= 0)
    {
        return -1;
    }

    int offset = timeband_offset(band, minute_of_day);
    return offset < 0 ? -1 : fraction_px(length, offset, band.span_min);
}

int timeband_pos_offset(TimeBand band, int length, int offset_min)
{
    if (length <= 0 || offset_min < 0 || offset_min > band.span_min)
    {
        return -1;
    }

    // the span's own end is allowed here where a moment's is not: a block ending with the window
    // has to reach the end of the axis rather than stopping a unit short of it
    return fraction_px(length, offset_min, band.span_min);
}

bool timeband_clip(TimeBand band, time_t window_epoch, time_t start, time_t end,
                   TimeBandSpan *out)
{
    time_t window_end = window_epoch + (time_t)band.span_min * SECONDS_PER_MINUTE;

    if (end < start)
    {
        end = start;
    }

    // an event pinned to a moment rather than a stretch still has to draw, so give it the minute
    // it starts in. the flags below read the real end so this never reports as running off the band
    time_t occupied_end = (end == start) ? start + SECONDS_PER_MINUTE : end;

    if (occupied_end <= window_epoch || start >= window_end)
    {
        return false;
    }

    // pin to the window before the subtraction so a span from last week cannot make a minute count
    // too big to hold
    time_t first = start < window_epoch ? window_epoch : start;
    time_t last = occupied_end > window_end ? window_end : occupied_end;

    int from = floor_minutes((long long)first - (long long)window_epoch);
    int to = ceil_minutes((long long)last - (long long)window_epoch);

    if (to <= from)
    {
        to = from + 1;
    }

    out->from = from;
    out->to = to;
    out->clipped_start = start < window_epoch;
    out->clipped_end = end > window_end;

    return true;
}

int timeband_clip_daily(TimeBand band, int from_min, int to_min,
                        TimeBandSpan *out, int max_out)
{
    if (from_min < 0 || to_min < 0 || max_out <= 0)
    {
        return 0;
    }

    int start = wrap_day(from_min);
    int length = wrap_day(to_min - from_min);
    if (length == 0)
    {
        // the two readings landing on the same minute is the sun never setting, not a span with
        // no width, so it covers the whole day
        length = TIMEBAND_DAY_MINUTES;
    }

    // the span repeats every day, so the window can catch the tail of yesterday's as well as
    // today's. anything a day further out starts past the end of a window this short
    int written = 0;
    for (int day = -1; day <= 0 && written < max_out; day++)
    {
        int at = wrap_day(start - band.start_min) + day * TIMEBAND_DAY_MINUTES;
        int from = at < 0 ? 0 : at;
        int to = (at + length) > band.span_min ? band.span_min : (at + length);

        if (to <= from)
        {
            continue;
        }

        out[written].from = from;
        out[written].to = to;
        out[written].clipped_start = at < 0;
        out[written].clipped_end = (at + length) > band.span_min;
        written++;
    }

    return written;
}

time_t timeband_window_epoch(TimeBand band, time_t midnight, int now_min)
{
    time_t epoch = midnight + (time_t)band.start_min * SECONDS_PER_MINUTE;

    // a window that opens later in the day than it is now opened yesterday, which is where a
    // rolling one lands every time it crosses midnight
    if (band.start_min > wrap_day(now_min))
    {
        epoch -= (time_t)TIMEBAND_DAY_MINUTES * SECONDS_PER_MINUTE;
    }

    return epoch;
}
