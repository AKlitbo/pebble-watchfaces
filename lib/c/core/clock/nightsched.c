/**
 * @file nightsched.c
 * @brief The night window, by containment rather than by edge.
 *
 * @ingroup lib_core
 */
#include "nightsched.h"

/**
 * @addtogroup lib_core
 * @{
 */

/// Minutes in a day, which is the only range any of these readings may sit in
#define DAY_MINUTES 1440

/** @brief Whether a reading is a real minute of the day rather than a missing one. */
static bool is_reading(int minute)
{
    return minute >= 0 && minute < DAY_MINUTES;
}

bool clock_window_contains(int start, int end, int now)
{
    if (!is_reading(start) || !is_reading(end) || !is_reading(now))
    {
        return false;
    }

    if (start == end)
    {
        return false;  // an empty window, not an eternal one
    }

    if (start < end)
    {
        return now >= start && now < end;
    }

    // the window runs past midnight, so it is everything from the start to the end of the day
    // plus everything from the start of the day to the end
    return now >= start || now < end;
}

bool night_schedule_active(int mode, int now, int rise, int set,
                           int fixed_start, int fixed_end, bool have_night)
{
    if (!have_night)
    {
        return false;  // nothing to switch to, so the window may as well be shut
    }

    if (mode == NIGHT_SCHED_SOLAR && is_reading(rise) && is_reading(set))
    {
        // night runs from sunset round to sunrise, which is the wrapping case nearly always
        return clock_window_contains(set, rise, now);
    }

    // the fixed pair covers its own mode and stands in for the sun when there is no reading. a
    // stale sunset is not treated as missing on purpose: last month's is within half an hour of
    // tonight's, which beats falling back for a reason the user cannot see
    if (mode == NIGHT_SCHED_SOLAR || mode == NIGHT_SCHED_FIXED)
    {
        return clock_window_contains(fixed_start, fixed_end, now);
    }

    return false;
}

/** @} */
