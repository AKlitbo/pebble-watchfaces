/**
 * @file store_poll.h
 * @brief Wall-clock deadlines for the stores that poll the phone.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>
#include <time.h>

/**
 * @addtogroup lib_stores
 * @{
 */

/**
 * @brief The next wall-clock second a poll of this interval falls due.
 *
 * The boundaries are counted from the epoch rather than from launch, so two watches started
 * minutes apart poll at the same moments and a relaunch does not shift the phase. Every interval
 * the settings offer divides an hour, so in practice these land on the hour, the half hour and so
 * on, which also means a bug is reproducible instead of depending on when the face happened to
 * start.
 *
 * @param poll_min Minutes between polls. Must be above 0.
 * @param now The current wall-clock time.
 * @return The second the next poll is due.
 */
static inline time_t store_poll_next(int poll_min, time_t now)
{
    time_t interval = (time_t)poll_min * SECONDS_PER_MINUTE;
    return ((now / interval) + 1) * interval;
}

/**
 * @brief Whether @p next has come round, moving it on to the following deadline when it has.
 *
 * A clock that jumps forward just fires once and carries on. One that jumps back would leave the
 * deadline stranded more than a whole interval ahead, which would hold the poll silent until wall
 * clock time caught up, so that case is pulled back in instead.
 *
 * @param poll_min Minutes between polls. 0 or less means polling is off and this is never due.
 * @param next The store's deadline, read and updated in place.
 * @param now The current wall-clock time.
 * @return Whether a poll should go out now.
 */
static inline bool store_poll_due(int poll_min, time_t *next, time_t now)
{
    if (poll_min <= 0)
    {
        return false;
    }

    time_t interval = (time_t)poll_min * SECONDS_PER_MINUTE;
    if (*next > now + interval)
    {
        *next = store_poll_next(poll_min, now);
    }

    if (now < *next)
    {
        return false;
    }

    *next = store_poll_next(poll_min, now);
    return true;
}

/** @} */
