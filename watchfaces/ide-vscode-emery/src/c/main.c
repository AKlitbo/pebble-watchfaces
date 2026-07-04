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
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "dev/dev.h"
#include "layout.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"
#include "system/vibe/vibe.h"

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
    // rebuild also re-pulls every readout covering a battery-display / unit / format change
    vscode_apply_theme();
    engine_rebuild();

    time_store_reconfigure(time_cfg());  // swaps to the .beats ticker if the format changed
}

static void init(void)
{
    settings_init(vscode_settings_schema());

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds
    // every store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.enabled = true, .live = true, .vibe = vibe_bt_transition}, NULL);
        health_store_init((HealthConfig){.enabled = true, .live = true}, NULL);
        time_store_init(time_cfg(), NULL);
        weather_store_init((WeatherConfig){.enabled = true, .live = true, .poll_min = 30}, NULL);
    }

    s_window = window_create();
    vscode_setup(s_window);              // fonts + frame + overlays + theme colours
    engine_init(s_window, vscode_build); // build the slot layers over the frame
    window_stack_push(s_window, true);

    // any store change repaints all slots (no location store: this face shows no coords)
    time_store_subscribe(engine_mark_dirty);
    health_store_subscribe(engine_mark_dirty);
    weather_store_subscribe(engine_mark_dirty);
    system_store_subscribe(engine_mark_dirty);

    // coords aren't displayed so that channel stays unregistered and the JS sends none.
    // weather_store owns the weather channel itself. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);
    appmessage_open();

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
