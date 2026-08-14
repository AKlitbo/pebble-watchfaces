/**
 * @file dev_ops.c
 * @brief The ops-slot walk and the fixture behind it. Always compiled and dropped by the linker
 * from any build that never calls it, the same way the shared theme walk is.
 */
#include "dev/dev_ops.h"

#include <time.h>

#include "io/stores/health_store.h"
#include "io/stores/location_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "ops/ops.h"
#include "ui/engine/engine.h"

// the four slots show four consecutive entries. one tap per readout walks the
// whole catalog so every entry gets photographed in each of the four positions
static uint8_t s_index;
static void (*s_apply_theme)(void);

// every frame reads a different clock, because a sheet of shots all sitting on the same minute
// looks like one screenshot pasted over and over. 53 minutes a step is coprime with the hour, so
// the walk crosses a whole day without landing on a time twice or falling into an obvious
// pattern, and the clock stays a function of the walk index. That last part matters: the walk
// returns to its starting frame pixel for pixel, which is how tap-walk.sh knows it has wrapped
#define CLOCK_BASE_MIN (7 * 60 + 38)
#define CLOCK_STEP_MIN 53

/**
 * @brief Pin the clock to this frame's time.
 *
 * The time store is only ever seeded here with live off and takes no subscription on that path,
 * so re-initialising it per tap re-seeds rather than stacking handlers.
 *
 * @param index The walk position.
 */
static void apply_clock(uint8_t index)
{
    int minutes = (CLOCK_BASE_MIN + index * CLOCK_STEP_MIN) % (24 * 60);

    time_t now = time(NULL);
    struct tm pinned = *localtime(&now);
    pinned.tm_hour = minutes / 60;
    pinned.tm_min = minutes % 60;
    pinned.tm_sec = 0;
    time_store_init((TimeConfig){.enabled = true, .live = false, .minute_tick = false, .beats = false}, &pinned);
}

uint8_t dev_ops_walk_pick(int slot)
{
    // the tall weather block is the left column's own thing and sits outside the
    // walk. it is the shipped default so the first shot of an untouched face
    // already covers it
    uint8_t id = (uint8_t)((s_index + slot) % OPS_COUNT);
    return ops_is_tall(id) ? OPS_EMPTY : id;
}

/**
 * @brief Accelerometer tap: step the pair along by one.
 *
 * @param axis The tap axis (unused).
 * @param direction The tap direction (unused).
 */
static void tap_handler(AccelAxisType axis, int32_t direction)
{
    s_index = (uint8_t)((s_index + 1) % OPS_COUNT);
    apply_clock(s_index);
    engine_rebuild();
}

void dev_ops_seed_stores(void)
{
    // pinned clock never ticking, moved on by the walk rather than by a tick
    apply_clock(0);

    // unlike the shared fixture this fills every reading in, because a slot being photographed
    // showing "--" is a picture of nothing
    WeatherSeed wx = WEATHER_SEED_EMPTY;
    wx.temp = 21;
    wx.cond = "PCLDY";
    wx.humidity = 62;
    wx.wind_kmh = 12;
    wx.wind_dir = "NW";
    wx.sunrise = "06:31";
    wx.sunset = "20:14";
    wx.uv = 7;
    wx.temp_max = 28;
    wx.temp_min = 17;
    wx.precip_chance = 20;
    weather_store_init((WeatherConfig){.enabled = true, .live = false, .poll_min = 0}, &wx);

    HealthSeed health = {.hr = 72, .steps = 8431, .calories = 420, .sleep_min = 431,
                         .active_min = 52, .distance_m = 5300};
    health_store_init((HealthConfig){.enabled = true, .live = false, .sleep = true,
                                     .active = true, .calories = true}, &health);

    SystemSeed system = {.battery = 64, .charging = false, .bluetooth = true};
    system_store_init((SystemConfig){.enabled = true, .live = false, .vibe = NULL}, &system);

    LocationSeed location = {.lat = "33-44", .lon = "-112-07"};
    location_store_init((LocationConfig){.enabled = true, .live = false}, &location);
}

void dev_ops_init(void (*apply_theme)(void))
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

void dev_ops_deinit(void)
{
    accel_tap_service_unsubscribe();
}
