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
#include "io/stores/location_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "layout.h"
#include "persist_keys.h"
#include "reel/reel.h"
#include "settings_schema.h"
#include "system/settings/setting_values.h"
#include "system/settings/settings.h"
#include "system/vibe/vibe.h"
#include "theme/custom_colors.h"
#include "ui/engine/engine.h"

// weather polls the phone this often. the reconnect catch-up reuses it as the staleness gate so
// the two cadences can never drift apart
#define WEATHER_POLL_MIN 30

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
        .enabled = true,
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

    sidereel_apply_theme();
    sidereel_apply_header_font();
    engine_rebuild();

    time_store_reconfigure(time_cfg());
}

/**
 * @brief Buzz once at the top of the hour with the pattern the user picked.
 *
 * The minute tick drives this so it lands exactly when tm_min hits 0, and vibe_choice stays
 * silent during quiet time (and for VIBE_NONE).
 */
static void hourly_vibe(void)
{
    if (time_store_tm()->tm_min == 0)
    {
        vibe_choice(settings_u8(SETTING_HOURLY_VIBE));
    }
}

static void on_time_tick(void)
{
    // the reel marks its own tag, and starts the scroll if this was a plain one-minute step
    reel_set_minute(time_store_tm()->tm_min, true);

    engine_mark_dirty_tags(TAG_TIME);
    hourly_vibe();
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
}

static void on_system_changed(void)
{
    engine_mark_dirty_tags(TAG_SYSTEM | TAG_CHROME);
}

/**
 * @brief Whether a reading is worth re-requesting: it never synced, or it is at least its own
 * poll interval old. Polling turned off (poll_min 0) is left alone, same as its store.
 */
static bool reconnect_should_refresh(int age_s, int poll_min)
{
    if (poll_min <= 0)
    {
        return false;
    }

    return age_s < 0 || age_s >= poll_min * 60;
}

/**
 * @brief The phone app just reconnected, so catch up a reading that went stale while it was away.
 *
 * Gating on age means bluetooth flapping cannot re-fetch faster than the normal cadence.
 */
static void on_phone_reconnected(void)
{
    if (reconnect_should_refresh(weather_store_age_s(), WEATHER_POLL_MIN))
    {
        appmessage_request_weather();
    }
}

static void init(void)
{
    settings_init(sidereel_settings_schema());
    dev_force_settings();

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds every
    // store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.enabled = true, .live = true, .vibe = vibe_bt_transition}, NULL);
        // the history flags are what the two graph panels read. they cost a storage write per
        // minute, so they are only on because a cell can actually show them
        health_store_init((HealthConfig){.enabled = true, .live = true, .calories = true,
                                         .sleep = true, .active = true,
                                         .hr_history = true, .step_history = true,
                                         .persist_key = HEALTH_STORE_KEY}, NULL);
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.enabled = true, .live = true, .poll_min = WEATHER_POLL_MIN,
                                           .persist_key = WEATHER_STORE_KEY}, NULL);
        location_store_init((LocationConfig){.enabled = true, .live = true}, NULL);
    }

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
    location_store_subscribe(on_weather_changed);  // a move only ever shifts the weather widgets
    system_store_subscribe(on_system_changed);
    system_store_on_reconnect(on_phone_reconnected);

    // weather_store + location_store own their channels. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);

    // the packed appearance string is too big for one persist slot, so it arrives and leaves
    // through its own pair of hooks rather than the settings field table
    appmessage_on_custom_colors(sidereel_set_custom_colors);
    appmessage_set_custom_colors_provider(sidereel_get_custom_colors);
    appmessage_open();

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
