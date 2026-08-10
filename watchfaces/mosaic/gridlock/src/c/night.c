/**
 * @file night.c
 * @brief The night schedule, wired to the stores and the engine.
 *
 * @ingroup gridlock_settings
 */
#include "night.h"

#include "settings_schema.h"
#include "engine/grid_engine.h"
#include "clock/clockstr.h"
#include "clock/nightsched.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup gridlock_settings
 * @{
 */

/**
 * @brief Work out which layout should be showing and switch if it is not already.
 *
 * The sun times are used however old they are. A sunset from last month is still within half an
 * hour of tonight's, which beats dropping to the fixed fallback for a reason nobody can see.
 *
 * @return True when the layout changed, so the caller knows to rebuild rather than just repaint.
 */
static bool night_sync(void)
{
    // nothing to do for the people who never turn this on, and this is the common case
    if (gridlock_night_mode() == NIGHT_SCHED_OFF)
    {
        if (!gridlock_active_layout_is_night())
        {
            return false;
        }

        gridlock_set_active_layout(false);
        return true;
    }

    const struct tm *now = time_store_tm();
    int minutes = now ? now->tm_hour * 60 + now->tm_min : -1;

    bool want = night_schedule_active(gridlock_night_mode(), minutes,
                                      clockstr_minutes(weather_store_sunrise()),
                                      clockstr_minutes(weather_store_sunset()),
                                      gridlock_night_start_min(), gridlock_night_end_min(),
                                      gridlock_night_layout_set());

    if (want == gridlock_active_layout_is_night())
    {
        return false;
    }

    gridlock_set_active_layout(want);
    return true;
}

void night_layout_init(void)
{
    // the engine does not exist yet, so settle the flag and let the first build read it
    night_sync();
}

void night_layout_settings_changed(void)
{
    // the caller rebuilds unconditionally straight after, so there is nothing to do here but
    // move the flag before it does
    night_sync();
}

void night_layout_tick(void)
{
    if (night_sync())
    {
        // a swap moves every cell, which is more than a repaint can express
        engine_rebuild();
        engine_mark_dirty();
    }
}

/** @} */
