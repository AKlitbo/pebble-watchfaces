/**
 * @file weather_store.c
 * @brief The active weather store: holds the readings, owns the appmessage weather channels,
 * and polls the phone on its own timer.
 */
#include "io/stores/weather_store.h"

#include <stdio.h>
#include <time.h>
#include <limits.h>

#include "io/appmessage/appmessage.h"

// a short first fetch after launch (fires from the event loop so appmessage is open by then)
// then the recurring poll runs at the configured interval
#define WEATHER_FIRST_POLL_MS 500

// persist slot for the last good reading so a relaunch (e.g. after the timeline) shows it
// straight away instead of blanking to "--". kept well clear of the settings keys which
// sit in a low band (1 / 5), so 255 leaves the whole low range free for face schemas
#define WEATHER_STORE_PERSIST_KEY 255

static struct
{
    char   temp[12];
    char   cond[32];
    char   location_name[32];
    int    humidity;
    int    wind_kmh;
    char   wind_dir[4];
    char   sunrise[8];
    char   sunset[8];
    int    uv;            // uv index (-1 when none)
    int    temp_max;      // today's high (WEATHER_NO_TEMP when none)
    int    temp_min;      // today's low (WEATHER_NO_TEMP when none)
    int    precip_chance; // percent chance of precip (-1 when none)
    time_t last_sync;
} s_state;

static void (*s_cb)(void);
static AppTimer *s_timer;
static int s_poll_ms;
static bool s_live;  // true = a live face, so the cache is worth reading and writing

// --- state writers (internal: only the channel handlers + the seed touch these) ---

static void reset_state(void)
{
    s_state.temp[0] = '\0';
    s_state.cond[0] = '\0';
    s_state.location_name[0] = '\0';
    s_state.humidity = -1;
    s_state.wind_kmh = -1;
    s_state.wind_dir[0] = '\0';
    s_state.sunrise[0] = '\0';
    s_state.sunset[0] = '\0';
    s_state.uv = -1;
    s_state.temp_max = WEATHER_NO_TEMP;
    s_state.temp_min = WEATHER_NO_TEMP;
    s_state.precip_chance = -1;
    s_state.last_sync = 0;
}

// stash the whole state so a relaunch can restore it. only a live face writes, and never
// from clear() so a failed fetch can't stomp the last reading that actually worked
static void persist_save(void)
{
    if (s_live)
    {
        persist_write_data(WEATHER_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
    }
}

static void set_current(const char *temp, const char *cond)
{
    snprintf(s_state.temp, sizeof(s_state.temp), "%s", temp ? temp : "--");
    snprintf(s_state.cond, sizeof(s_state.cond), "%s", cond ? cond : "--");
    s_state.last_sync = time(NULL);
    persist_save();
    if (s_cb) s_cb();
}

// --- appmessage channel handlers (the store owns its own wiring) ---

/**
 * @brief Current weather channel. A NULL condition means the fetch failed (network drop,
 * provider outage, no location) so we keep the last good reading rather than blanking. It
 * refreshes on the next poll that lands, and survives a relaunch via the cache.
 *
 * @param temp The temperature text.
 * @param cond The condition token, or NULL on failure.
 */
static void on_weather(const char *temp, const char *cond)
{
    if (!cond)
    {
        return;
    }
    set_current(temp, cond);
}

/**
 * @brief Extra weather channel (humidity / wind / sun). Absent values arrive as -1 / "".
 */
static void on_extra(int humidity, int wind_kmh, const char *dir, const char *sunrise, const char *sunset)
{
    s_state.humidity = humidity;
    s_state.wind_kmh = wind_kmh;
    snprintf(s_state.wind_dir, sizeof(s_state.wind_dir), "%s", dir ? dir : "");
    snprintf(s_state.sunrise, sizeof(s_state.sunrise), "%s", sunrise ? sunrise : "");
    snprintf(s_state.sunset, sizeof(s_state.sunset), "%s", sunset ? sunset : "");
    s_state.last_sync = time(NULL);
    persist_save();
    if (s_cb) s_cb();
}

/**
 * @brief Forecast channel (uv / hi-lo / precip). The transport hands absent fields as INT_MIN,
 * so map those back to the store's own no-data values (temps get their own sentinel).
 */
static void on_forecast(int uv, int temp_max, int temp_min, int precip_chance)
{
    s_state.uv = (uv == INT_MIN) ? -1 : uv;
    s_state.temp_max = (temp_max == INT_MIN) ? WEATHER_NO_TEMP : temp_max;
    s_state.temp_min = (temp_min == INT_MIN) ? WEATHER_NO_TEMP : temp_min;
    s_state.precip_chance = (precip_chance == INT_MIN) ? -1 : precip_chance;
    s_state.last_sync = time(NULL);
    persist_save();
    if (s_cb) s_cb();
}

/**
 * @brief Location-name channel.
 */
static void on_location_name(const char *name)
{
    snprintf(s_state.location_name, sizeof(s_state.location_name), "%s", name ? name : "");
    persist_save();
    if (s_cb) s_cb();
}

// --- poll timer ---

/**
 * @brief Ask the phone for weather, then re-arm at the configured interval.
 */
static void poll_fire(void *data)
{
    appmessage_request_weather();
    s_timer = app_timer_register(s_poll_ms, poll_fire, NULL);
}

static void stop_polling(void)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }
}

// --- public API ---

void weather_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void weather_store_init(WeatherConfig cfg, const WeatherSeed *seed)
{
    s_live = cfg.live;  // set before any set_current so persist_save knows whether to write
    reset_state();
    s_poll_ms = cfg.poll_min * 60 * 1000;

    if (seed)
    {
        set_current(seed->temp, seed->cond);  // s_cb is NULL until the face subscribes so no redraw yet
    }
    else if (cfg.live
             && persist_exists(WEATHER_STORE_PERSIST_KEY)
             && persist_get_size(WEATHER_STORE_PERSIST_KEY) == (int)sizeof(s_state))
    {
        // restore the last good reading so a relaunch shows it right away. s_cb is still NULL
        // so no redraw fires here, but the first paint (window push) re-pulls every readout.
        // the 500ms first poll then refreshes it in the background
        persist_read_data(WEATHER_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        // the store owns every weather channel. faces that don't declare the extra/forecast/
        // location keys simply never see those fire
        appmessage_on_weather(on_weather);
        appmessage_on_weather_extra(on_extra);
        appmessage_on_weather_forecast(on_forecast);
        appmessage_on_location_name(on_location_name);

        // first fetch shortly after launch then every poll interval
        stop_polling();
        s_timer = app_timer_register(WEATHER_FIRST_POLL_MS, poll_fire, NULL);
    }
}

void weather_store_reconfigure(WeatherConfig cfg)
{
    s_poll_ms = cfg.poll_min * 60 * 1000;

    // re-arm at the new interval (no immediate fetch. a real interval change is rare)
    stop_polling();
    if (cfg.enabled && cfg.live && s_poll_ms > 0)
    {
        s_timer = app_timer_register(s_poll_ms, poll_fire, NULL);
    }
}

const char *weather_store_temp(void)          { return s_state.temp; }
const char *weather_store_cond(void)          { return s_state.cond; }
const char *weather_store_location_name(void) { return s_state.location_name; }
int         weather_store_humidity(void)      { return s_state.humidity; }
int         weather_store_wind_kmh(void)      { return s_state.wind_kmh; }
const char *weather_store_wind_dir(void)      { return s_state.wind_dir; }
const char *weather_store_sunrise(void)       { return s_state.sunrise; }
const char *weather_store_sunset(void)        { return s_state.sunset; }
int         weather_store_uv(void)            { return s_state.uv; }
int         weather_store_temp_max(void)      { return s_state.temp_max; }
int         weather_store_temp_min(void)      { return s_state.temp_min; }
int         weather_store_precip_chance(void) { return s_state.precip_chance; }

int weather_store_age_s(void)
{
    return s_state.last_sync ? (int)(time(NULL) - s_state.last_sync) : -1;
}
