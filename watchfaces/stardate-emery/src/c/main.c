// stardate-emery entry point: wires services to the ui shell
#include <pebble.h>

#include "appmessage/appmessage.h"
#include "health/health.h"
#include "settings/settings.h"
#include "shell/shell.h"

static int s_battery_level;

// pull fresh health + battery readings into the ui
static void update_info(void)
{
    int hr = health_get_current_hr();
    int steps = health_get_today_steps();
    int distance_m = health_get_today_distance();

    shell_update_info(hr, steps, distance_m, s_battery_level);
}

// redraw after a settings change, re-rendering the clock when the time/date format changed
static void on_settings_changed(bool time_or_date_changed)
{
    shell_update_display();
    if (time_or_date_changed)
    {
        time_t now = time(NULL);
        shell_update_time(localtime(&now));
    }
}

// per-minute: redraw time, refresh readouts, poll weather every 30 min
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    shell_update_time(tick_time);

    // refresh health readouts (also keeps .beats current at minute resolution)
    update_info();

    if (tick_time->tm_min % 30 == 0)
    {
        appmessage_request_weather();
    }
}

// refresh readouts on a relevant health event
static void health_handler(HealthEventType event, void *context)
{
    if (event == HealthEventHeartRateUpdate || event == HealthEventMovementUpdate ||
        event == HealthEventSignificantUpdate)
    {
        update_info();
    }
}

// cache the battery level and refresh the ui
static void battery_callback(BatteryChargeState state)
{
    s_battery_level = state.charge_percent;
    update_info();
}

// initialize sub-systems, seed the first render, and subscribe to services
static void init(void)
{
    settings_init();
    shell_init();

    // wire the appmessage transport to the shell (the transport stays shell-agnostic)
    appmessage_init((AppMessageHandlers){
        .on_weather = shell_set_weather,
        .on_coords = shell_set_coords,
        .on_settings_changed = on_settings_changed,
    });
    health_init(health_handler);

    // seed the initial time/date render before the first tick
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    shell_update_time(tick_time);

    // services and callbacks
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    battery_state_service_subscribe(battery_callback);
    battery_callback(battery_state_service_peek());

    // seed the info line (heart rate / steps / battery) before the first event
    update_info();
}

// tear down the ui shell
static void deinit(void)
{
    shell_deinit();
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
