/**
 * @file dev_wx.c
 * @brief The weather walk. Always compiled and dropped by the linker from any build that never
 * calls it, the same way the other harnesses are.
 */
#include "dev/dev_wx.h"

#include <time.h>

#include "io/stores/health_store.h"
#include "io/stores/location_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "ui/engine/engine.h"

// every token the generated icon table resolves in the order it lists them
// the day glyph and its night twin sit next to each other so a contact sheet
// pairs them up
static const char *const s_conditions[] = {
    "CLEAR",  "CLEAR_NIGHT",
    "PCLDY",  "PCLDY_NIGHT",
    "CLDY",   "CLDY_NIGHT",
    "FOGGY",  "FOGGY_NIGHT",
    "DRZL",   "DRZL_NIGHT",
    "RAIN",   "RAIN_NIGHT",
    "SHWR",   "SHWR_NIGHT",
    "SNOW",   "SNOW_NIGHT",
    "SNSH",   "SNSH_NIGHT",
    "STRM",   "STRM_NIGHT",
    "FZDZ",   "FZDZ_NIGHT",
    "FZRN",   "FZRN_NIGHT",
    // not a real reading but it is the glyph the face falls back to so it gets checked too
    "",
};

static uint8_t s_index;
static void (*s_apply_theme)(void);

/**
 * @brief Pin the weather store to one condition, leaving the rest of the reading alone.
 *
 * @param index Which condition, wrapped to the table.
 */
static void apply_condition(uint8_t index)
{
    WeatherSeed wx = WEATHER_SEED_EMPTY;
    wx.temp = 21;
    wx.cond = s_conditions[index % ARRAY_LENGTH(s_conditions)];
    weather_store_init((WeatherConfig){.enabled = true, .live = false, .poll_min = 0}, &wx);
}

/**
 * @brief Accelerometer tap: step to the next condition.
 *
 * @param axis The tap axis (unused).
 * @param direction The tap direction (unused).
 */
static void tap_handler(AccelAxisType axis, int32_t direction)
{
    s_index = (uint8_t)((s_index + 1) % ARRAY_LENGTH(s_conditions));
    apply_condition(s_index);
    engine_rebuild();
}

void dev_wx_seed_stores(int hour, int min)
{
    time_t now = time(NULL);
    struct tm pinned = *localtime(&now);
    pinned.tm_hour = hour;
    pinned.tm_min = min;
    pinned.tm_sec = 0;
    time_store_init((TimeConfig){.enabled = true, .live = false, .minute_tick = false, .beats = false}, &pinned);

    apply_condition(0);

    HealthSeed health = {.hr = 72, .steps = 8431, .calories = 420, .sleep_min = 431,
                         .active_min = 52, .distance_m = 5300};
    health_store_init((HealthConfig){.enabled = true, .live = false, .sleep = true,
                                     .active = true, .calories = true}, &health);

    SystemSeed system = {.battery = 64, .charging = false, .bluetooth = true};
    system_store_init((SystemConfig){.enabled = true, .live = false, .vibe = NULL}, &system);

    LocationSeed location = {.lat = "33-44", .lon = "-112-07"};
    location_store_init((LocationConfig){.enabled = true, .live = false}, &location);
}

void dev_wx_init(void (*apply_theme)(void))
{
    s_index = 0;
    s_apply_theme = apply_theme;

    if (s_apply_theme)
    {
        s_apply_theme();
    }

    engine_rebuild();
    accel_tap_service_subscribe(tap_handler);
}

void dev_wx_deinit(void)
{
    accel_tap_service_unsubscribe();
}
