/**
 * @file weather_store.h
 * @brief Active weather store. It owns the appmessage weather channels and its own poll
 * timer, so a face just hands it the rules (enabled / live / poll interval), optionally
 * seeds it, then reads the values and subscribes for repaints. The phone data flows straight
 * in and nobody wires it.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

// sentinel for a missing hi/low temperature since real temps can be negative and 0 is valid
#define WEATHER_NO_TEMP -1000

// how many columns a forecast strip can carry. matches FORECAST_COLS on the phone. the 2x4
// stacks two rows of four so both strips fill up to eight
#define WEATHER_FORECAST_COLS 8

/** @brief One hourly forecast column: a sky code plus a temperature (WEATHER_NO_TEMP for none). */
typedef struct
{
    uint8_t code;  // condition code (index into the shared vocabulary, 255 = unknown)
    int16_t temp;  // temperature in the user's unit
} WeatherHourCol;

/** @brief One daily forecast column: a sky code plus the day's high and low. */
typedef struct
{
    uint8_t code;
    int16_t temp_max;
    int16_t temp_min;
} WeatherDayCol;

/** @brief The hourly strip. count is 0 until a reading lands. */
typedef struct
{
    uint8_t        count;       // how many columns are filled (0 = none yet)
    uint8_t        base_hour;   // hour of day of the first column (0-23)
    uint8_t        step_hours;  // hours between columns
    WeatherHourCol col[WEATHER_FORECAST_COLS];
} WeatherHourly;

/** @brief The 7-day strip. count is 0 until a reading lands. */
typedef struct
{
    uint8_t       count;         // how many columns are filled (0 = none yet)
    uint8_t       base_weekday;  // weekday of the first column (0 = Sunday)
    WeatherDayCol col[WEATHER_FORECAST_COLS];
} WeatherDaily;

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 */
typedef struct
{
    bool enabled;   // false = inert (no channels no timer holds nothing)
    bool live;      // true = subscribe the channels + poll. false = seed-only (dev/screenshots)
    int  poll_min;  // minutes between weather requests (e.g. 10 / 20 / 30)
} WeatherConfig;

/**
 * @brief Optional prefill: shown until the first real reading lands, or pinned when
 * live = false.
 */
typedef struct
{
    const char *temp;      // "23C"
    const char *cond;      // "SUNNY"
    int         humidity;  // percent, -1 for none
    int         wind_kmh;  // km/h, -1 for none
    const char *wind_dir;  // "NW", "" for none
    const char *sunrise;   // "06:30", "" for none
    const char *sunset;    // "21:30", "" for none
    int         uv;        // uv index, -1 for none
    int         temp_max;  // today's high, WEATHER_NO_TEMP for none
    int         temp_min;  // today's low, WEATHER_NO_TEMP for none
    int         precip_chance; // percent, -1 for none
    const WeatherHourly *forecast_hourly; // optional hourly strip, NULL for none
    const WeatherDaily  *forecast_daily;  // optional 7-day strip, NULL for none
} WeatherSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live / poll interval).
 * @param seed Optional prefill, or NULL.
 */
void weather_store_init(WeatherConfig cfg, const WeatherSeed *seed);

/**
 * @brief Re-apply the rules (e.g. the poll interval changed). Re-arms the timer.
 *
 * @param cfg The new rules.
 */
void weather_store_reconfigure(WeatherConfig cfg);

/** @brief Hand it the function to call when the weather changes (the screen redraw). */
void weather_store_subscribe(void (*cb)(void));

const char *weather_store_temp(void);          // "23°C" (empty until we get one)
const char *weather_store_cond(void);          // short word for the sky (empty until we get one)
const char *weather_store_location_name(void); // "Toronto" (empty until we get one)
int         weather_store_humidity(void);      // damp percent (-1 means none yet)
int         weather_store_wind_kmh(void);      // wind in km/h (-1 means none yet)
const char *weather_store_wind_dir(void);      // "NW" (empty means none yet)
const char *weather_store_sunrise(void);       // "06:30" (empty means none yet)
const char *weather_store_sunset(void);        // "21:30" (empty means none yet)
int         weather_store_uv(void);            // uv index (-1 means none yet)
int         weather_store_temp_max(void);      // today's high (WEATHER_NO_TEMP means none)
int         weather_store_temp_min(void);      // today's low (WEATHER_NO_TEMP means none)
int         weather_store_precip_chance(void); // precip percent (-1 means none yet)

/** @brief The hourly forecast strip. count is 0 until a reading lands. */
const WeatherHourly *weather_store_forecast_hourly(void);

/** @brief The 7-day forecast strip. count is 0 until a reading lands. */
const WeatherDaily *weather_store_forecast_daily(void);

/** @brief How many seconds since the last reading turned up, or -1 if we have none. */
int weather_store_age_s(void);

/** @} */
