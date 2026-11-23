/**
 * @file main.c
 * @brief Sketchbook entry point: wires services to their stores and drives the slot engine
 * directly (no shell), for whichever scene is picked.
 *
 * The scene reacts to the weather, so it polls every 30 min, and it reacts to the sun, so the
 * minute tick also watches for the palette flipping at sunrise and sunset. It never displays
 * coordinates, so on_coords is left unset and the JS sends none.
 */
#include <pebble.h>

#include "io/appmessage/appmessage.h"
#include "ui/engine/engine.h"
#include "ui/readouts.h"
#include "io/stores/health_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "dev/dev_hooks.h"
#include "layout/engine.h"
#include "persist_keys.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"
#include "system/vibe/vibe.h"

static Window *s_window;

// only the rectangle has a stats row, so only it reads the health store
#if !defined(PBL_ROUND)
// a heart rate reading can land every second during a workout, and each one would repaint the
// whole scene. a reading paints straight away and then holds the next ones off for this long
#define HEALTH_HOLD_MS 10000

static AppTimer *s_health_hold;  ///< Running while health readings are held off, NULL otherwise
static bool s_health_owed;       ///< A reading landed during the hold and is not on screen yet
#endif

/**
 * @brief Repaint everything, or rebuild it when the palette baked into the slots has moved.
 *
 * Every full repaint comes through here. It puts the latest health readings up with everything
 * else, so a reading held off until the end of the hold has nothing left to paint.
 *
 * @param rebuild True to rebuild the slots rather than repaint them.
 */
static void repaint(bool rebuild)
{
#if !defined(PBL_ROUND)
    s_health_owed = false;
#endif

    if (rebuild)
    {
        engine_rebuild();
    }
    else
    {
        engine_mark_dirty();
    }
}

/** @brief Store callback for the weather and system stores: any change repaints everything. */
static void on_store_changed(void)
{
    repaint(false);
}

#if !defined(PBL_ROUND)
static void on_health_hold_end(void *data);

/** @brief Paint the latest health readings and hold the next ones off for HEALTH_HOLD_MS. */
static void paint_health(void)
{
    repaint(false);
    s_health_hold = app_timer_register(HEALTH_HOLD_MS, on_health_hold_end, NULL);
}

/**
 * @brief The hold is over. A reading that landed during it goes up now and starts another hold.
 *
 * @param data Unused.
 */
static void on_health_hold_end(void *data)
{
    s_health_hold = NULL;

    if (s_health_owed)
    {
        paint_health();
    }
}

/**
 * @brief Health store callback: paint a reading straight away, or owe it to the end of the hold.
 *
 * A single change after a quiet spell shows at once, and a workout's stream of them paints at most
 * once every HEALTH_HOLD_MS.
 */
static void on_health_changed(void)
{
    if (s_health_hold)
    {
        s_health_owed = true;
        return;
    }

    paint_health();
}
#endif

/**
 * @brief The time-store rules from the current settings (cadence follows what is on screen).
 *
 * Two things can want the .beats ticker and they are independent: the clock itself, and a date
 * format ending in a reading. A beat is 86.4 seconds, so a date line left on the minute tick
 * sits on a stale number and reports every rollover up to a minute late. The store runs both
 * cadences at once, so the date line can have its ticker while the clock keeps its minute tick.
 */
static TimeConfig time_cfg(void)
{
    bool clock_beats = settings_u8(SETTING_TIME_FORMAT) == TIME_FORMAT_BEATS;
    bool hourly = settings_u8(SETTING_HOURLY_VIBE) != VIBE_NONE;

    return (TimeConfig){
        .live = true,
        .minute_tick = !clock_beats,  // .beats replaces the clock, and only the clock
        .beats = clock_beats || readout_date_shows_beats(),
        .hourly = hourly,             // a .beats clock still wakes on the hour for the buzz
    };
}

/**
 * @brief Take up a settings change: switch scene if it moved, otherwise re-theme.
 *
 * A scene change puts the new scene up and themes it. Any other change only needs the theme read
 * again. A save from the phone runs this, and so does the dev harness, at startup and on every step
 * of a walk, so a walk that picks a scene draws it the way a real save would.
 */
static void apply_settings_change(void)
{
    if (!sketchbook_scene_reselect())
    {
        sketchbook_apply_theme();
    }
}

/**
 * @brief Redraw after a settings change: switch scene if it moved, re-theme, rebuild, re-arm.
 *
 * @param time_or_date_changed True if a time or date format changed (unused: a rebuild re-pulls
 * every readout anyway, and the ticker is re-armed on every change, since Hourly Vibration picks it
 * too).
 */
static void on_settings_changed(bool time_or_date_changed)
{
    apply_settings_change();
    repaint(true);

    time_store_reconfigure(time_cfg());  // on every change, since the formats and the buzz both pick it
}

/**
 * @brief Tick callback: repaint the scene and the slots, and fire the hourly vibe.
 *
 * On the one minute the sun comes up or goes down the whole palette changes, and the text
 * colours are baked into the slot layers, so that minute needs a rebuild rather than a
 * repaint. Every other minute takes the cheap path.
 *
 * vibe_hourly buzzes once per hour however many tickers land in minute 0, and stays silent during
 * quiet time and for VIBE_NONE. On a .beats clock the hour tick time_cfg asks for is what lands it
 * there. The SDK sends no hour tick for the hour the clocks repeat in autumn, so on a .beats clock
 * that hour buzzes only when a beat lands in its minute 0. Catching the rest would take the minute
 * tick, sixty wakes an hour all year for one buzz a year.
 */
static void on_time_tick(void)
{
    repaint(sketchbook_palette_changed());

    vibe_hourly(time_store_tm(), settings_u8(SETTING_HOURLY_VIBE));
}

static void init(void)
{
    // one schema for the whole face, with the scene choice alongside the palette and layout
    settings_init(sketchbook_settings_schema());

    // each store owns its own source. init starts it (live) or seeds it
    // dev mode seeds every store (live=false) from paf.dev.config.json, or the fixture where it
    // names none, so screenshots come out the same every time
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.live = true, .vibe = vibe_bt_transition}, NULL);
#if !defined(PBL_ROUND)
        // heart rate, steps and distance only show in the stats row, and a round screen has none
        // so a round build leaves the health service alone. the health capability still covers
        // the round watch, since an appinfo lists its capabilities once for every platform
        // a dev build on round still seeds the store through the dev plugin, and nothing reads it
        health_store_init((HealthConfig){.live = true, .distance = true, .persist_key = HEALTH_STORE_KEY}, NULL);
#endif
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.live = true, .poll_min = 30,
                                           .persist_key = WEATHER_STORE_KEY}, NULL);
    }

    s_window = window_create();
    sketchbook_setup(s_window);              // fonts + palette, for the chosen scene
    engine_init(s_window, sketchbook_build); // build the scene, chrome, and text slots
    window_stack_push(s_window, true);

    // any store change repaints everything: the scene reads the weather and the clock, so a
    // reading landing changes the picture and not just the numbers. health readings are held off
    // between paints, since during a workout they come every second
    time_store_subscribe(on_time_tick);
#if !defined(PBL_ROUND)
    health_store_subscribe(on_health_changed);
#endif
    weather_store_subscribe(on_store_changed);
    system_store_subscribe(on_store_changed);

    // weather_store owns its channels. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);
    appmessage_open(2048);

    // dev mode only: paint the fixture, or with a walk picked in paf.dev.config.json show its first
    // screen and step on each tap
    dev_start(apply_settings_change);
}

static void deinit(void)
{
    dev_stop();
    system_store_deinit();
    engine_deinit();
    sketchbook_teardown();
    window_destroy(s_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
