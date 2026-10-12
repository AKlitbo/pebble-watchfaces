/**
 * @file main.c
 * @brief ide-vscode-emery entry point: wires services to their stores and drives the slot
 * engine directly (no shell).
 *
 * Shows the weather condition and temperature in the terminal stats row, so it polls
 * weather every 30 min. It never displays coordinates, so on_coords is left unset and the
 * JS sends none.
 */
#include <pebble.h>

#include "io/appmessage/appmessage.h"
#include "ui/engine/engine.h"
#include "io/stores/health_store.h"
#include "io/stores/store_poll.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "dev/dev.h"
#include "layout.h"
#include "persist_keys.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"
#include "system/vibe/vibe.h"

// weather polls the phone this often. the reconnect catch-up reuses it as the staleness gate so
// the two cadences can never drift apart
#define WEATHER_POLL_MIN 30

static Window *s_window;

/**
 * @brief The time-store rules from the current settings. The cadence follows the time format.
 * On a .beats clock the hourly buzz also asks for a wake at the top of each hour.
 */
static TimeConfig time_cfg(void)
{
    bool beats = settings_u8(SETTING_TIME_FORMAT) == TIME_FORMAT_BEATS;
    bool hourly = settings_u8(SETTING_HOURLY_VIBE) != VIBE_NONE;

    return (TimeConfig){
        .live = !DEV_MODE,      // a dev build keeps the clock its fixture pinned
        .minute_tick = !beats,  // .beats replaces the clock
        .beats = beats,
        .hourly = hourly,       // a .beats clock still wakes on the hour for the buzz
    };
}

/**
 * @brief Redraw after a settings change: re-theme, rebuild the slots, re-arm the ticker.
 *
 * @param time_or_date_changed True if a time or date format changed (unused: a rebuild re-pulls
 * every readout anyway, and the ticker is re-armed on every change, since Hourly Vibration picks it
 * too).
 */
static void on_settings_changed(bool time_or_date_changed)
{
    // re-apply the zone colours + frame then rebuild so the text-slots pick them up. the
    // rebuild also re-pulls every readout covering a battery-display / unit / format change
    vscode_apply_theme();
    engine_rebuild();

    time_store_reconfigure(time_cfg());  // on every change, since the format and the buzz both pick it
}

/**
 * @brief Tick callback: repaint the slots and fire the hourly vibe.
 *
 * vibe_hourly buzzes once per hour however many tickers land in minute 0, and stays silent during
 * quiet time and for VIBE_NONE. On a .beats clock the hour tick time_cfg asks for is what lands it
 * there. The SDK sends no hour tick for the hour the clocks repeat in autumn, so on a .beats clock
 * that hour buzzes only when a beat lands in its minute 0. Catching the rest would take the minute
 * tick, sixty wakes an hour all year for one buzz a year.
 */
static void on_time_tick(void)
{
    engine_mark_dirty();
    vibe_hourly(time_store_tm(), settings_u8(SETTING_HOURLY_VIBE));
}

/**
 * @brief The phone app just reconnected, so catch up a reading that went stale while it was away.
 *
 * The phone's pkjs may have been closed for the whole gap, and the store's own poll is 30 min, so
 * without this the face sits on an old reading. Gating on age means bluetooth flapping cannot
 * re-fetch faster than the normal cadence.
 */
static void on_phone_reconnected(void)
{
    if (store_poll_reconnect_due(weather_store_age_s(), WEATHER_POLL_MIN))
    {
        appmessage_request_weather();
    }
}

static void init(void)
{
    settings_init(vscode_settings_schema());

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds
    // every store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.live = true, .vibe = vibe_bt_transition}, NULL);
        health_store_init((HealthConfig){.live = true, .distance = true, .persist_key = HEALTH_STORE_KEY}, NULL);
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.live = true, .poll_min = WEATHER_POLL_MIN,
                                           .persist_key = WEATHER_STORE_KEY}, NULL);
    }

    s_window = window_create();
    vscode_setup(s_window);              // fonts + frame + overlays + theme colours
    engine_init(s_window, vscode_build); // build the slot layers over the frame
    window_stack_push(s_window, true);

    // any store change repaints all slots (no location store: this face shows no coords).
    // the time tick also drives the hourly vibe
    time_store_subscribe(on_time_tick);
    health_store_subscribe(engine_mark_dirty);
    weather_store_subscribe(engine_mark_dirty);
    system_store_subscribe(engine_mark_dirty);
    system_store_on_reconnect(on_phone_reconnected);

    // coords aren't displayed so that channel stays unregistered and the JS sends none
    // weather_store owns the weather channel itself. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);
    appmessage_open(2048);

    // dev mode only: force the theme then paint the fixture then subscribe the tap walk
    dev_start(vscode_apply_theme);
}

static void deinit(void)
{
    dev_stop();
    system_store_deinit();
    engine_deinit();
    vscode_teardown();
    window_destroy(s_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
