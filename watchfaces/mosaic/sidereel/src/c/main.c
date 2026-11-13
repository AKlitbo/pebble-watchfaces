/**
 * @file main.c
 * @brief sidereel-emery entry point: wires services to their stores and drives the slot engine
 * directly (no shell).
 *
 * Unlike the other faces here, every store change routes to a tag rather than a blanket
 * repaint. The reel scrolls on its own timer, and a tagless slot would redraw on every one of
 * those frames.
 */
#include <pebble.h>

#include "mosaic/goal_vibe.h"
#include "dev/dev.h"
#include "io/appmessage/appmessage.h"
#include "io/stores/health_store.h"
#include "io/stores/phone_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "layout.h"
#include "mosaic/layout_switch.h"
#include "mosaic/phone_gate.h"
#include "persist_keys.h"
#include "reel/reel.h"
#include "settings_schema.h"
#include "system/settings/setting_values.h"
#include "system/settings/settings.h"
#include "system/vibe/vibe.h"
#include "theme/custom_colors.h"
#include "ui/engine/engine.h"

static Window *s_window;

/**
 * @brief The time-store rules.
 *
 * The reel scrolls on the minute whatever the time format says, so unlike the shell faces the
 * minute tick never turns off. .beats carries no hour for the pennant, so it is not run here.
 */
static TimeConfig time_cfg(void)
{
    return (TimeConfig){
        .live = true,
        .minute_tick = true,
        .beats = false,
    };
}

/**
 * @brief Redraw after a settings change: settle the reel, re-colour, rebuild the slots.
 *
 * @param time_or_date_changed True if the time or date format changed (unused: a rebuild
 * re-pulls every readout anyway).
 */
static void on_settings_changed(bool time_or_date_changed)
{
    // the scroll timer marks a layer that engine_rebuild is about to destroy, so settle it first
    reel_cancel();

    // a layout or its trigger may have changed, so settle the role before the rebuild reads it
    layout_switch_settings_changed();

    sidereel_apply_theme();
    sidereel_apply_header_font();
    engine_rebuild();

    time_store_reconfigure(time_cfg());
    // a phone battery panel may have been placed or taken off, which starts or stops the phone's
    // updates for it
    phone_gate_settings_changed();
}

static void on_time_tick(void)
{
    // the reel marks its own tag
    // a plain one-minute step scrolls, unless the wearer turned Reduce Motion on
    reel_set_minute(time_store_tm()->tm_min, !sidereel_reduce_motion());

    engine_mark_dirty_tags(TAG_TIME);

    // buzzes once at the top of each hour, and stays quiet when the face reopens in minute 0
    vibe_hourly(time_store_tm(), settings_u8(SETTING_HOURLY_VIBE));
    layout_switch_tick();
}

// the store callbacks are void(*)(void) so they cannot carry their own tag
static void on_health_changed(void)
{
    goal_vibe_update();
    engine_mark_dirty_tags(TAG_HEALTH);
}

static void on_weather_changed(void)
{
    engine_mark_dirty_tags(TAG_WEATHER);
    // a sunset arriving mid-evening can mean the layout should already have swapped
    layout_switch_tick();
}

static void on_system_changed(void)
{
    engine_mark_dirty_tags(TAG_SYSTEM | TAG_CHROME);
}

static void on_phone_changed(void)
{
    engine_mark_dirty_tags(TAG_PHONE);
}

static void init(void)
{
    settings_init(sidereel_settings_schema());
    dev_force_settings();

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds every
    // store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.live = true, .vibe = vibe_bt_transition}, NULL);
        // the phone's battery, from the Pebble app's Phone plugin. it hears the phone come and go
        // through the live system store above, and clears its reading while the phone is away
        phone_gate_start();
        // the history flags are what the two graph panels read. they cost a storage write per
        // minute, so they are only on because a cell can actually show them
        health_store_init((HealthConfig){.live = true, .calories = true,
                                         .sleep = true, .active = true,
                                         .hr_history = HEALTH_HR_HISTORY, .step_history = HEALTH_STEP_HISTORY, .distance = true,
                                         .persist_key = HEALTH_STORE_KEY}, NULL);
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.live = true, .poll_min = 30,
                                           .persist_key = WEATHER_STORE_KEY}, NULL);
    }

    // settle which layout wins before the first build, so a watchface launched after dark comes
    // up on the night layout instead of flashing the day one
    layout_switch_init();

    s_window = window_create();
    sidereel_setup(s_window);

    // the reel's first paint happens inside engine_init, so it needs the right minute already
    // settled at offset zero. never animate this one
    reel_set_minute(time_store_tm()->tm_min, false);

    engine_init(s_window, sidereel_build);
    window_stack_push(s_window, true);

    // take stock of today's goals before the first reading lands, so reopening the face on a
    // goal already met stays quiet
    goal_vibe_init();

    time_store_subscribe(on_time_tick);
    health_store_subscribe(on_health_changed);
    weather_store_subscribe(on_weather_changed);
    system_store_subscribe(on_system_changed);
    phone_store_subscribe(on_phone_changed);

    // weather_store owns its channel. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);

    // the packed appearance string is too big for one persist slot, so it arrives and leaves
    // through its own pair of hooks rather than the settings field table
    appmessage_on_custom_colors(sidereel_set_custom_colors);
    appmessage_set_custom_colors_provider(sidereel_get_custom_colors);
    appmessage_open(4096);

    dev_start(sidereel_apply_theme);
}

static void deinit(void)
{
    dev_stop();
    reel_cancel();  // the scroll marks the reel layer dirty, and it is about to go away
    system_store_deinit();
    engine_deinit();
    sidereel_teardown();
    window_destroy(s_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
