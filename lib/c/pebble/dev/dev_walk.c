/**
 * @file dev_walk.c
 * @brief The shared frame-face dev harness: fixture-as-store-seeds plus the theme walk.
 * Always compiled and gc-dropped from any face that never calls it (release builds).
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

/**
 * @brief The believable happy numbers a screenshot sits on, and the baseline every shot below
 * varies from.
 */
typedef struct
{
    int16_t temp;
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
    .temp = 21, .cond = "PCLDY",
    .hr = 72, .steps = 8431, .calories = 420, .sleep_min = 431, .active_min = 52, .distance_m = 5300,
    .battery = 64, .bluetooth = true,
    .lat = "33-44", .lon = "-112-07",
};

/**
 * @brief One screenshot's worth of state, so a walk of eight themes is not eight shots of the
 * same numbers.
 *
 * A contact sheet of identical readings shows the palette and nothing else, and it reads as a
 * mock rather than as eight watches. These carry the things the chrome and the scene actually
 * show: what the weather is doing, how full the battery is, whether the phone is connected,
 * whether the Quiet Time mark is up, and where the minute hand sits.
 */
typedef struct
{
    int16_t     temp;
    const char *cond;
    int         hr;
    int         steps;
    int         battery;
    bool        bluetooth;
    bool        quiet_icon;
    uint8_t     minute;
} DevShot;

// spread deliberately rather than at random: a flat battery and a full one, a dropped phone,
// the Quiet Time mark up and down, and a range of weather so the scene is not drawing the same
// sky eight times
static const DevShot s_shots[] = {
    {.temp = 21, .cond = "PCLDY", .hr = 72, .steps = 8431,  .battery = 64,  .bluetooth = true,  .quiet_icon = false, .minute = 42},
    {.temp = 3,  .cond = "SNOW",  .hr = 58, .steps = 12045, .battery = 92,  .bluetooth = true,  .quiet_icon = true,  .minute = 18},
    {.temp = -8, .cond = "CLEAR", .hr = 61, .steps = 3120,  .battery = 41,  .bluetooth = false, .quiet_icon = false, .minute = 5},
    {.temp = 31, .cond = "CLEAR", .hr = 88, .steps = 15680, .battery = 17,  .bluetooth = true,  .quiet_icon = true,  .minute = 55},
    {.temp = 14, .cond = "DRZL",  .hr = 66, .steps = 6402,  .battery = 78,  .bluetooth = true,  .quiet_icon = false, .minute = 27},
    {.temp = 27, .cond = "CLDY",  .hr = 74, .steps = 9310,  .battery = 8,   .bluetooth = false, .quiet_icon = true,  .minute = 33},
    {.temp = 9,  .cond = "FOGGY", .hr = 55, .steps = 4870,  .battery = 100, .bluetooth = true,  .quiet_icon = false, .minute = 11},
    {.temp = 35, .cond = "PCLDY", .hr = 91, .steps = 11250, .battery = 53,  .bluetooth = true,  .quiet_icon = true,  .minute = 48},
};

static DevWalkMode s_mode;
static void (*s_apply_theme)(void);
static uint8_t s_theme;
// the daylight the walk was started in, so a shot moves the minute without moving the hour and
// dragging a night sweep into daytime
static int s_hour;

/**
 * @brief Re-seed the stores from one shot.
 *
 * The stores are only ever seeded here with live off, and neither takes a subscription on that
 * path, so re-initialising them per tap re-seeds rather than stacking handlers.
 *
 * @param index Which shot, wrapped to the table.
 */
static void apply_shot(uint8_t index)
{
    const DevShot *shot = &s_shots[index % ARRAY_LENGTH(s_shots)];

    struct tm pinned;
    time_t now = time(NULL);
    pinned = *localtime(&now);
    pinned.tm_hour = s_hour;
    pinned.tm_min = shot->minute;
    pinned.tm_sec = 0;
    time_store_init((TimeConfig){.enabled = true, .live = false, .minute_tick = false, .beats = false}, &pinned);

    WeatherSeed wx = WEATHER_SEED_EMPTY;
    wx.temp = shot->temp;
    wx.cond = shot->cond;
    weather_store_init((WeatherConfig){.enabled = true, .live = false, .poll_min = 0}, &wx);

    HealthSeed health = {.hr = shot->hr, .steps = shot->steps, .calories = s_default.calories,
                         .sleep_min = s_default.sleep_min, .active_min = s_default.active_min,
                         .distance_m = s_default.distance_m};
    health_store_init((HealthConfig){.enabled = true, .live = false}, &health);

    SystemSeed system = {.battery = shot->battery, .charging = false, .bluetooth = shot->bluetooth};
    system_store_init((SystemConfig){.enabled = true, .live = false, .vibe = NULL}, &system);

    // a no-op on a face that never subscribed to the id, which is what we want: it only moves
    // for the faces that actually draw the mark
    settings_set_u8(SETTING_QUIET_TIME_ICON, shot->quiet_icon ? 1 : 0);
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
        // the data moves with the theme, so a sheet of every palette is also a sheet of every
        // battery level, both bluetooth states and a range of weather
        apply_shot(s_theme);
        if (s_apply_theme) s_apply_theme();
        engine_rebuild();
    }
}

void dev_walk_seed_stores(int hour, int min)
{
    s_hour = hour;

    // pinned clock never ticking
    time_t now = time(NULL);
    struct tm pinned = *localtime(&now);
    pinned.tm_hour = hour;
    pinned.tm_min = min;
    pinned.tm_sec = 0;
    time_store_init((TimeConfig){.enabled = true, .live = false, .minute_tick = false, .beats = false}, &pinned);

    // spread the empty seed so every reading the fixture does not set (humidity, wind, uv, …)
    // reads as "--" rather than a bogus 0
    WeatherSeed wx = WEATHER_SEED_EMPTY;
    wx.temp = s_default.temp;
    wx.cond = s_default.cond;
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

void dev_walk_init(DevWalkMode mode, void (*apply_theme)(void))
{
    s_mode = mode;
    s_apply_theme = apply_theme;
    s_theme = 0;

    if (mode == DEV_WALK_THEMES)
    {
        settings_set_u8(SETTING_THEME, 0);
        apply_shot(0);
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
