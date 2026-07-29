/**
 * @file main.c
 * @brief radar-array-emery entry point: wires services to their stores and drives the slot
 * engine directly (no shell).
 */
#include <pebble.h>

#include "io/appmessage/appmessage.h"
#include "ui/engine/engine.h"
#include "io/stores/health_store.h"
#include "io/stores/location_store.h"
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
 * @brief The time-store rules from the current settings (cadence follows the time format).
 */
static TimeConfig time_cfg(void)
{
    bool beats = settings_u8(SETTING_TIME_FORMAT) == TIME_FORMAT_BEATS;
    return (TimeConfig){
        .enabled = true,
        .live = true,
        .minute_tick = !beats,  // the shell faces run one cadence: .beats replaces the clock
        .beats = beats,
    };
}

/**
 * @brief Redraw after a settings change: re-theme, rebuild the slots, re-arm the ticker.
 *
 * @param time_or_date_changed True if the time or date format changed (unused: a rebuild
 * re-pulls every readout anyway).
 */
static void on_settings_changed(bool time_or_date_changed)
{
    // re-apply the zone colours + frame then rebuild so the text-slots pick them up. the
    // rebuild also re-pulls every readout covering a steps-mode / unit / format change
    radar_apply_theme();
    engine_rebuild();

    time_store_reconfigure(time_cfg());  // swaps to the .beats ticker if the format changed
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

/**
 * @brief Minute-tick callback: repaint the slots and fire the hourly vibe.
 */
static void on_time_tick(void)
{
    engine_mark_dirty();
    hourly_vibe();
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
 * The phone's pkjs may have been closed for the whole gap, and the store's own poll is 30 min, so
 * without this the face sits on an old reading. Gating on age means bluetooth flapping cannot
 * re-fetch faster than the normal cadence.
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
    // radar owns its schema: v1 with its own key and no migration. the readout-style date
    // default ("SAT 19 JUN") lives in the schema's field table
    settings_init(radar_settings_schema());

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds
    // every store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.enabled = true, .live = true, .vibe = vibe_bt_transition}, NULL);
        health_store_init((HealthConfig){.enabled = true, .live = true, .persist_key = HEALTH_STORE_KEY}, NULL);
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.enabled = true, .live = true, .poll_min = WEATHER_POLL_MIN,
                                           .persist_key = WEATHER_STORE_KEY}, NULL);
        location_store_init((LocationConfig){.enabled = true, .live = true}, NULL);
    }

    s_window = window_create();
    radar_setup(s_window);              // fonts + frame + overlays + theme colours
    engine_init(s_window, radar_build); // build the slot layers over the frame
    window_stack_push(s_window, true);

    // any store change repaints all slots. the time tick also drives the hourly vibe
    time_store_subscribe(on_time_tick);
    health_store_subscribe(engine_mark_dirty);
    weather_store_subscribe(engine_mark_dirty);
    location_store_subscribe(engine_mark_dirty);
    system_store_subscribe(engine_mark_dirty);
    system_store_on_reconnect(on_phone_reconnected);

    // weather_store + location_store own their channels. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);
    appmessage_open();

    // dev mode only: force the theme then paint the fixture then subscribe the tap walk
    dev_start(radar_apply_theme);
}

static void deinit(void)
{
    dev_stop();
    system_store_deinit();
    engine_deinit();
    radar_teardown();
    window_destroy(s_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
