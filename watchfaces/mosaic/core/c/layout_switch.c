/**
 * @file layout_switch.c
 * @brief The layout triggers, wired to the stores, the SDK and the engine.
 *
 * @ingroup mosaic
 */
#include "layout_switch.h"

#include "settings_schema.h"
#include "ui/engine/engine.h"
#include "clock/clockstr.h"
#include "clock/nightsched.h"
#include "layout/layout_role.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup mosaic
 * @{
 */

// what Quiet Time read last time we looked. the panels that draw it are tagged as system readings
// and nothing else raises that tag on a tick, so noticing the flip here is what keeps them honest
static bool s_quiet_was_on;

/**
 * @brief Whether the night window is open right now.
 *
 * The sun times are used however old they are. A sunset from last month is still within half an
 * hour of tonight's, which beats dropping to the fixed fallback for a reason nobody can see.
 */
static bool night_is_on(void)
{
    if (gridlock_night_mode() == NIGHT_SCHED_OFF)
    {
        return false;
    }

    const struct tm *now = time_store_tm();
    int minutes = now ? now->tm_hour * 60 + now->tm_min : -1;

    return night_schedule_active(gridlock_night_mode(), minutes,
                                 clockstr_minutes(weather_store_sunrise()),
                                 clockstr_minutes(weather_store_sunset()),
                                 gridlock_night_start_min(), gridlock_night_end_min(),
                                 gridlock_night_layout_set());
}

/**
 * @brief Work out which layout should be showing and switch if it is not already.
 *
 * @return True when the layout changed, so the caller knows to rebuild rather than just repaint.
 */
static bool layout_sync(void)
{
    bool quiet = quiet_time_is_active();

    LayoutRole want = layout_role_pick(quiet, gridlock_quiet_layout_set(),
                                       night_is_on(), gridlock_night_layout_set());

    if (want == gridlock_active_layout_role())
    {
        return false;
    }

    gridlock_set_active_layout_role(want);
    return true;
}

void layout_switch_init(void)
{
    // the engine does not exist yet, so settle the role and let the first build read it
    s_quiet_was_on = quiet_time_is_active();
    layout_sync();
}

void layout_switch_settings_changed(void)
{
    // the caller rebuilds unconditionally straight after, so there is nothing to do here but
    // move the role before it does
    layout_sync();
}

void layout_switch_tick(void)
{
    bool quiet = quiet_time_is_active();
    bool quiet_moved = quiet != s_quiet_was_on;
    s_quiet_was_on = quiet;

    if (layout_sync())
    {
        // the face settles anything holding a layer the rebuild is about to destroy
        gridlock_before_rebuild();

        // a swap moves every cell, which is more than a repaint can express. this covers the
        // Quiet Time panels too, so there is nothing else to do
        engine_rebuild();
        engine_mark_dirty();
        return;
    }

    if (quiet_moved)
    {
        // Quiet Time flipped without changing the layout, which is what happens when no Quiet
        // layout is assigned. whatever shows it still needs telling, and the two faces name that
        // tag differently, so the face raises it
        gridlock_mark_system_dirty();
    }
}

/** @} */
