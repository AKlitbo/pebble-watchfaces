// stardate-emery entry point: wires services to the ui shell
#include <pebble.h>

#include "appmessage/appmessage.h"
#include "health/health.h"
#include "settings/settings.h"
#include "shell/shell.h"

// pull fresh health readings into the ui (driven by health events, not polled)
static void update_health(void)
{
    int hr = health_get_current_hr();
    int steps = health_get_today_steps();
    int distance_m = health_get_today_distance();

    shell_update_info(hr, steps, distance_m);
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

// per-minute: redraw the time (and .beats at minute resolution), poll weather
// every 30 min. health is refreshed from health events, not polled here
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    shell_update_time(tick_time);

    if (tick_time->tm_min % 30 == 0)
    {
        appmessage_request_weather();
    }
}

// refresh health readouts only when the health service reports new data
static void health_handler(HealthEventType event, void *context)
{
    if (event == HealthEventHeartRateUpdate || event == HealthEventMovementUpdate ||
        event == HealthEventSignificantUpdate)
    {
        update_health();
    }
}

// redraw the battery gauge when the charge level changes
static void battery_callback(BatteryChargeState state)
{
    shell_set_battery(state.charge_percent);
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

    // seed the health readouts before the first health event
    update_health();
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
