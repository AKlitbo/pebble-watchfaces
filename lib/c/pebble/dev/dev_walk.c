/**
 * @file dev_walk.c
 * @brief The shared frame-face dev harness: fixture-as-store-seeds plus the theme and icon
 * walks. Always compiled and gc-dropped from any face that never calls it (release builds).
 */
#include "dev/dev_walk.h"

#include <time.h>

#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "io/stores/health_store.h"
#include "io/stores/system_store.h"
#include "io/stores/location_store.h"
#include "system/settings/settings.h"
#include "ui/engine/engine.h"
#include "ui/weather/conditions_dev_table.h"

/**
 * @brief The believable happy numbers a screenshot sits on. One fixture for now. A selector
 * (NO_DATA / NIGHT / LOW_BATTERY / …) can come later without touching the walks.
 */
typedef struct
{
    const char *temp;
    const char *cond;
    int hr;
    int steps;
    int calories;
    int sleep_min;
    int active_min;
    int distance_m;
    int battery;
    bool bluetooth;
    const char *lat; // LCARS dash style e.g. "33-44"
    const char *lon;
} DevFixture;

static const DevFixture s_default = {
    .temp = "21°", .cond = "PCLDY",
    .hr = 72, .steps = 8431, .calories = 420, .sleep_min = 431, .active_min = 52, .distance_m = 5300,
    .battery = 64, .bluetooth = true,
    .lat = "33-44", .lon = "-112-07",
};

static DevWalkMode s_mode;
static void (*s_apply_theme)(void);
static uint8_t s_theme;
static int s_icon;

/**
 * @brief Re-seed weather_store (live=false) with the fixture temp and a given condition,
 * which fires its subscriber so the engine repaints. Used to step the icon walk.
 *
 * @param cond The condition token to show.
 */
static void seed_weather(const char *cond)
{
    WeatherSeed seed = {.temp = s_default.temp, .cond = cond};
    weather_store_init((WeatherConfig){.enabled = true, .live = false, .poll_min = 0}, &seed);
}

/**
 * @brief Accelerometer tap: advance the active walk by one.
 *
 * @param axis The tap axis (unused).
 * @param direction The tap direction (unused).
 */
static void tap_handler(AccelAxisType axis, int32_t direction)
{
    if (s_mode == DEV_WALK_THEMES)
    {
        uint8_t count = settings_enum_count(SETTING_THEME);
        s_theme = count ? (s_theme + 1) % count : 0;
        settings_set_u8(SETTING_THEME, s_theme);
        if (s_apply_theme) s_apply_theme();
        engine_rebuild();
    }
    else if (s_mode == DEV_WALK_ICONS)
    {
        s_icon = (s_icon + 1) % (int)ARRAY_LENGTH(wx_dev_condition_tokens);
        seed_weather(wx_dev_condition_tokens[s_icon]);
    }
}

void dev_walk_seed_stores(int hour, int min)
{
    // pinned clock never ticking
    time_t now = time(NULL);
    struct tm pinned = *localtime(&now);
    pinned.tm_hour = hour;
    pinned.tm_min = min;
    pinned.tm_sec = 0;
    time_store_init((TimeConfig){.enabled = true, .live = false, .minute_tick = false, .beats = false}, &pinned);

    WeatherSeed wx = {.temp = s_default.temp, .cond = s_default.cond};
    weather_store_init((WeatherConfig){.enabled = true, .live = false, .poll_min = 0}, &wx);

    HealthSeed health = {.hr = s_default.hr, .steps = s_default.steps, .calories = s_default.calories,
                         .sleep_min = s_default.sleep_min, .active_min = s_default.active_min,
                         .distance_m = s_default.distance_m};
    health_store_init((HealthConfig){.enabled = true, .live = false}, &health);

    SystemSeed system = {.battery = s_default.battery, .charging = false, .bluetooth = s_default.bluetooth};
    system_store_init((SystemConfig){.enabled = true, .live = false, .vibe = NULL}, &system);

    LocationSeed location = {.lat = s_default.lat, .lon = s_default.lon};
    location_store_init((LocationConfig){.enabled = true, .live = false}, &location);
}

void dev_walk_init(DevWalkMode mode, uint8_t icon_theme, void (*apply_theme)(void))
{
    s_mode = mode;
    s_apply_theme = apply_theme;
    s_theme = 0;
    s_icon = 0;

    if (mode == DEV_WALK_ICONS)
    {
        settings_set_u8(SETTING_THEME, icon_theme);
        seed_weather(wx_dev_condition_tokens[0]);
    }
    else if (mode == DEV_WALK_THEMES)
    {
        settings_set_u8(SETTING_THEME, 0);
    }

    // re-colour zones + swap the frame for the forced theme then repaint everything
    if (apply_theme) apply_theme();
    engine_rebuild();

    if (mode != DEV_WALK_NONE)
    {
        accel_tap_service_subscribe(tap_handler);
    }
}

void dev_walk_deinit(void)
{
    if (s_mode != DEV_WALK_NONE)
    {
        accel_tap_service_unsubscribe();
    }
}
