/**
 * @file nightsched.h
 * @brief Whether a night-time window is open right now, from plain clock readings.
 *
 * A face that swaps its look after dark has to answer one question every minute: is it night? This
 * answers it by asking whether now falls *inside* a window rather than by watching for the moment
 * it crossed one. That matters more than it looks. An edge-detecting version misses its cue
 * whenever the clock jumps for daylight saving, whenever the app was asleep at the crossing, or
 * whenever a tick is simply dropped, and once it has missed it stays wrong until the next crossing
 * comes round. Containment cannot get stuck: it is right again on the very next minute.
 *
 * Every reading comes in as minutes past midnight, and -1 means there is no reading. The caller
 * gathers them; nothing here touches a watch, which is what keeps it host-testable.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>

/**
 * @addtogroup lib_core
 * @{
 */

/// What the schedule is following. The caller's own enum should line up with these
typedef enum
{
    NIGHT_SCHED_OFF = 0,   ///< Never night
    NIGHT_SCHED_SOLAR,     ///< Sunset to sunrise, falling back to the fixed pair with no reading
    NIGHT_SCHED_FIXED,     ///< The fixed pair only
    NIGHT_SCHED_COUNT
} NightSchedMode;

/**
 * @brief Whether now falls inside a window that may run past midnight.
 *
 * The start is inclusive and the end exclusive, so two windows that meet at the same minute never
 * both claim it. A window that starts and ends at the same minute is empty rather than eternal,
 * which is the reading that does not surprise anyone who set both dials the same by accident.
 *
 * @param start First minute inside the window, 0 to 1439, or -1 for no reading.
 * @param end First minute past it, same range.
 * @param now Minutes past midnight.
 * @return True while now is inside. False if any reading is missing.
 */
bool clock_window_contains(int start, int end, int now);

/**
 * @brief Whether the night window is open, whichever way the schedule is set.
 *
 * Following the sun needs both a sunrise and a sunset to work from. When either is missing this
 * falls back to the fixed pair rather than giving up, so a watch that has never been told where it
 * is still does something sensible, and the two dials the user set earn their keep twice.
 *
 * @param mode A NightSchedMode. Anything out of range reads as off.
 * @param now Minutes past midnight.
 * @param rise Sunrise, or -1 for no reading.
 * @param set Sunset, same.
 * @param fixed_start The user's own start, used by the fixed mode and as the solar fallback.
 * @param fixed_end And their end.
 * @param have_night True when there is actually a night look to switch to.
 * @return True when the night look should be the one showing.
 */
bool night_schedule_active(int mode, int now, int rise, int set,
                           int fixed_start, int fixed_end, bool have_night);

/** @} */
