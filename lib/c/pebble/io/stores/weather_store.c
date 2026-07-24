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
#include "io/stores/store_persist.h"

// a short first fetch after launch (fires from the event loop so appmessage is open by then)
// then the recurring poll runs at the configured interval
#define WEATHER_FIRST_POLL_MS 500

// the phone JS can take several seconds to come up on a cold launch, so a first request that lands
// before it is ready would leave the face blank until the next full poll (30 min). until the first
// reading arrives, keep asking on this short cadence for a bounded number of tries, then settle
#define WEATHER_BOOT_RETRY_MS 3000
#define WEATHER_BOOT_RETRIES 8

static struct
{
    uint8_t tag;          // STORE_TAG_WEATHER, so a restore can tell this blob from another shape
    int16_t temp;         // current temperature in the user's unit (WEATHER_NO_TEMP when none)
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
    int    feels_like;    // apparent temperature (WEATHER_NO_TEMP when none)
    int    pressure;      // surface pressure in hPa (-1 when none)
    int    dew_point;     // dew point temperature (WEATHER_NO_TEMP when none)
    WeatherHourly hourly; // the hourly forecast strip (count 0 when none)
    WeatherDaily  daily;  // the 7-day forecast strip (count 0 when none)
    time_t last_sync;
} s_state;
_Static_assert(sizeof(s_state) <= PERSIST_DATA_MAX_LENGTH, "weather state must fit one persist key");

static void (*s_cb)(void);
static AppTimer *s_timer;
static int s_poll_ms;
static int s_boot_retries; // short cold-boot re-asks used so far, until the first reading lands
static bool s_live;  // true = a live face, so the cache is worth reading and writing
static uint32_t s_persist_key; // the slot the face handed us for the saved reading
static bool s_dirty; // a channel touched the state this inbox, so persist_flush writes it once

// --- state writers (internal: only the channel handlers + the seed touch these) ---

static void reset_state(void)
{
    s_state.temp = WEATHER_NO_TEMP;
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
    s_state.feels_like = WEATHER_NO_TEMP;
    s_state.pressure = -1;
    s_state.dew_point = WEATHER_NO_TEMP;
    s_state.hourly.count = 0;
    s_state.daily.count = 0;
    s_state.last_sync = 0;
}

// mark the reading as needing a save. a combined weather push fires several channel handlers
// in one inbox, so rather than each writing the whole struct to flash we just flag it dirty
// and persist_flush does one write when the inbox is fully dispatched (one poll = one write)
static void mark_dirty(void)
{
    s_dirty = true;
}

// the shared tail every channel handler runs after it writes: stamp the sync time, flag the cache
// dirty, and repaint. keeps the six handlers from each spelling it out
static void mark_synced(void)
{
    s_state.last_sync = time(NULL);
    mark_dirty();
    if (s_cb) s_cb();
}

// the coalesced write, run once per inbox via the transport's inbox-complete hook. only a live
// face writes, and never from clear() so a failed fetch can't stomp the last good reading
static void persist_flush(void)
{
    if (!s_dirty)
    {
        return;
    }
    if (!s_live)
    {
        s_dirty = false;
        return;
    }

    // hold the dirty flag when the write does not land so the next inbox tries again rather than
    // leaving the cache quietly stale
    s_dirty = !store_save(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_WEATHER);
}

static void set_current(int temp, const char *cond)
{
    s_state.temp = (int16_t)temp;
    snprintf(s_state.cond, sizeof(s_state.cond), "%s", cond ? cond : "--");
    mark_synced();
}

// prefill the whole store from a seed (dev/screenshots). copies the extras too so a seeded
// face shows full weather not just temp and cond. s_cb is NULL at init so no redraw fires here
static void apply_seed(const WeatherSeed *seed)
{
    s_state.temp = seed->temp;
    snprintf(s_state.cond, sizeof(s_state.cond), "%s", seed->cond ? seed->cond : "--");
    s_state.humidity = seed->humidity;
    s_state.wind_kmh = seed->wind_kmh;
    snprintf(s_state.wind_dir, sizeof(s_state.wind_dir), "%s", seed->wind_dir ? seed->wind_dir : "");
    snprintf(s_state.sunrise, sizeof(s_state.sunrise), "%s", seed->sunrise ? seed->sunrise : "");
    snprintf(s_state.sunset, sizeof(s_state.sunset), "%s", seed->sunset ? seed->sunset : "");
    s_state.uv = seed->uv;
    s_state.temp_max = seed->temp_max;
    s_state.temp_min = seed->temp_min;
    s_state.precip_chance = seed->precip_chance;
    s_state.feels_like = seed->feels_like;
    s_state.pressure = seed->pressure;
    s_state.dew_point = seed->dew_point;
    if (seed->forecast_hourly)
    {
        s_state.hourly = *seed->forecast_hourly;
    }
    if (seed->forecast_daily)
    {
        s_state.daily = *seed->forecast_daily;
    }
    s_state.last_sync = time(NULL);
    if (s_cb) s_cb();
}

// --- appmessage channel handlers (the store owns its own wiring) ---

/**
 * @brief Current weather channel. A NULL condition means the fetch failed (network drop,
 * provider outage, no location) so we keep the last good reading rather than blanking. It
 * refreshes on the next poll that lands, and survives a relaunch via the cache.
 *
 * @param temp The temperature in the user's unit (ignored when cond is NULL).
 * @param cond The condition token, or NULL on failure.
 */
static void on_weather(int temp, const char *cond)
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
    mark_synced();
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
    mark_synced();
}

/**
 * @brief Air channel (feels-like / pressure / dew point). Absent fields arrive as INT_MIN,
 * mapped back to the store's no-data values (temps get their own sentinel).
 */
static void on_air(int feels_like, int pressure, int dew_point)
{
    s_state.feels_like = (feels_like == INT_MIN) ? WEATHER_NO_TEMP : feels_like;
    s_state.pressure = (pressure == INT_MIN) ? -1 : pressure;
    s_state.dew_point = (dew_point == INT_MIN) ? WEATHER_NO_TEMP : dew_point;
    mark_synced();
}

/**
 * @brief Hourly forecast channel. Takes the strip the reader unpacked and puts it away.
 *
 * A message that does not read clean leaves the last good row where it is.
 */
static void on_forecast_hourly(const uint8_t *buf, uint16_t len)
{
    if (!weather_hourly_decode(buf, len, &s_state.hourly))
    {
        return;
    }

    mark_synced();
}

/**
 * @brief 7-day forecast channel. Takes the strip the reader unpacked and puts it away.
 *
 * A message that does not read clean leaves the last good row where it is.
 */
static void on_forecast_daily(const uint8_t *buf, uint16_t len)
{
    if (!weather_daily_decode(buf, len, &s_state.daily))
    {
        return;
    }

    mark_synced();
}

/**
 * @brief Location-name channel.
 */
static void on_location_name(const char *name)
{
    snprintf(s_state.location_name, sizeof(s_state.location_name), "%s", name ? name : "");
    mark_dirty();
    if (s_cb) s_cb();
}

// --- poll timer ---

/**
 * @brief Ask the phone for weather, then re-arm at the configured interval.
 */
static void poll_fire(void *data)
{
    appmessage_request_weather();
    // re-arm only when there is a real interval, so a 0ms poll_min can never spin the loop.
    // this keeps poll_fire self-defending regardless of how the timer was first armed
    if (s_poll_ms <= 0)
    {
        s_timer = NULL;  // fired and not re-arming, so drop the now-stale handle
        return;
    }

    // until the first reading lands, re-ask on the short boot cadence for a bounded window rather
    // than waiting out the full interval. a cold launch can send the first request before the phone
    // JS is up, and the reconnect refresh only fires on a real disconnect->connect transition, not
    // on the initial connect, so without this the face would stay blank until the next poll
    int next_ms = s_poll_ms;
    if (s_state.last_sync == 0 && s_boot_retries < WEATHER_BOOT_RETRIES)
    {
        s_boot_retries++;
        next_ms = WEATHER_BOOT_RETRY_MS;
    }
    s_timer = app_timer_register(next_ms, poll_fire, NULL);
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
    s_live = cfg.live;  // set before any set_current so persist_flush knows whether to write
    s_persist_key = cfg.persist_key;
    reset_state();
    s_poll_ms = cfg.poll_min * 60 * 1000;
    s_boot_retries = 0;  // fresh cold-boot re-ask budget

    if (seed)
    {
        apply_seed(seed);  // s_cb is NULL until the face subscribes so no redraw yet
    }
    else if (cfg.live)
    {
        // restore the last good reading so a relaunch shows it right away. s_cb is still NULL
        // so no redraw fires here, but the first paint (window push) re-pulls every readout.
        // the 500ms first poll then refreshes it in the background
        if (store_restore(s_persist_key, &s_state, sizeof(s_state), STORE_TAG_WEATHER))
        {
            // the counts come back off flash and index the draw loops, so pin them to what the
            // arrays actually hold before anything reads them
            if (s_state.hourly.count > WEATHER_FORECAST_COLS)
            {
                s_state.hourly.count = 0;
            }
            if (s_state.daily.count > WEATHER_FORECAST_COLS)
            {
                s_state.daily.count = 0;
            }
        }
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
        appmessage_on_weather_air(on_air);
        appmessage_on_weather_forecast_hourly(on_forecast_hourly);
        appmessage_on_weather_forecast_daily(on_forecast_daily);
        appmessage_on_location_name(on_location_name);
        // one coalesced persist per inbox instead of one write per channel handler
        appmessage_on_inbox_complete(persist_flush);

        // first fetch shortly after launch then every poll interval. poll_min 0 disables
        // polling (matching reconfigure), so only arm when there is a real interval,
        // otherwise poll_fire would re-arm a 0ms timer and spin the event loop
        stop_polling();
        if (s_poll_ms > 0)
        {
            s_timer = app_timer_register(WEATHER_FIRST_POLL_MS, poll_fire, NULL);
        }
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

int         weather_store_temp(void)          { return s_state.temp; }
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
int         weather_store_feels_like(void)    { return s_state.feels_like; }
int         weather_store_pressure(void)      { return s_state.pressure; }
int         weather_store_dew_point(void)     { return s_state.dew_point; }

const WeatherHourly *weather_store_forecast_hourly(void) { return &s_state.hourly; }
const WeatherDaily  *weather_store_forecast_daily(void)  { return &s_state.daily; }

int weather_store_age_s(void)
{
    return s_state.last_sync ? (int)(time(NULL) - s_state.last_sync) : -1;
}
