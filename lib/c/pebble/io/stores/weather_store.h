/**
 * @file weather_store.h
 * @brief Active weather store. It owns the appmessage weather channels and its own poll
 * timer, so a face just hands it the rules (enabled / live / poll interval), optionally
 * seeds it, then reads the values and subscribes for repaints. The phone data flows straight
 * in and nobody wires it.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

#include "wire/weather_wire.h"

/**
 * @addtogroup lib_stores
 * @{
 */

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 */
typedef struct
{
    bool     enabled;     ///< False makes the store do nothing (no channels and no timer)
    bool     live;        ///< True subscribes the channels and polls. False just keeps the fake data for screenshots
    int      poll_min;    ///< Minutes between weather requests (e.g. 10, 20, 30)
    uint32_t persist_key; ///< Slot for the last good reading so the face owns the key instead of the store
} WeatherConfig;

/**
 * @brief Optional prefill: shown until the first real reading lands, or pinned when
 * live = false.
 */
typedef struct
{
    int16_t     temp;      ///< Reading in the user's unit, WEATHER_NO_TEMP for none
    const char *cond;      ///< Short word for the sky like "SUNNY"
    int         humidity;  ///< Percent, -1 for none
    int         wind_kmh;  ///< Wind in km/h, -1 for none
    const char *wind_dir;  ///< Wind direction like "NW", empty for none
    const char *sunrise;   ///< Sunrise time like "06:30", empty for none
    const char *sunset;    ///< Sunset time like "21:30", empty for none
    int         uv;        ///< UV index, -1 for none
    int         temp_max;  ///< Today's high, WEATHER_NO_TEMP for none
    int         temp_min;  ///< Today's low, WEATHER_NO_TEMP for none
    int         precip_chance; ///< Chance of rain in percent, -1 for none
    int         feels_like;    ///< How warm it feels, WEATHER_NO_TEMP for none
    int         pressure;      ///< Surface pressure in hPa, -1 for none
    int         dew_point;     ///< Dew point temperature, WEATHER_NO_TEMP for none
    const WeatherHourly *forecast_hourly; ///< Optional hourly strip, NULL for none
    const WeatherDaily  *forecast_daily;  ///< Optional 7 day strip, NULL for none
} WeatherSeed;

/// A WeatherSeed with every optional reading already set to its no-data value. Spread it and
/// override only what you have, so a partial seed still reads as "--" instead of a bogus 0.
/// @code
/// WeatherSeed seed = WEATHER_SEED_EMPTY;
/// seed.temp = 23;
/// seed.cond = "SUNNY";
/// @endcode
#define WEATHER_SEED_EMPTY ((WeatherSeed){                       \
    .temp = WEATHER_NO_TEMP, .cond = "",                         \
    .humidity = -1, .wind_kmh = -1, .wind_dir = "",             \
    .sunrise = "", .sunset = "", .uv = -1,                       \
    .temp_max = WEATHER_NO_TEMP, .temp_min = WEATHER_NO_TEMP,   \
    .precip_chance = -1,                                         \
    .feels_like = WEATHER_NO_TEMP, .pressure = -1,               \
    .dew_point = WEATHER_NO_TEMP,                                \
    .forecast_hourly = NULL, .forecast_daily = NULL })

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

/** @brief The temperature reading in the user's unit, or WEATHER_NO_TEMP if we have not got one yet. */
int         weather_store_temp(void);

/** @brief The short word for the sky, or empty if we have not got one yet. */
const char *weather_store_cond(void);

/** @brief The location name like "Toronto", or empty if we have not got one yet. */
const char *weather_store_location_name(void);

/** @brief The humidity in percent, or -1 if we have not got one yet. */
int         weather_store_humidity(void);

/** @brief The wind speed in km/h, or -1 if we have not got one yet. */
int         weather_store_wind_kmh(void);

/** @brief The wind direction like "NW", or empty if we have not got one yet. */
const char *weather_store_wind_dir(void);

/** @brief The sunrise time like "06:30", or empty if we have not got one yet. */
const char *weather_store_sunrise(void);

/** @brief The sunset time like "21:30", or empty if we have not got one yet. */
const char *weather_store_sunset(void);

/** @brief How warm it feels, or WEATHER_NO_TEMP if we have not got one yet. */
int         weather_store_feels_like(void);

/** @brief The surface pressure in hPa, or -1 if we have not got one yet. */
int         weather_store_pressure(void);

/** @brief The dew point temperature, or WEATHER_NO_TEMP if we have not got one yet. */
int         weather_store_dew_point(void);

/** @brief The UV index, or -1 if we have not got one yet. */
int         weather_store_uv(void);

/** @brief Today's high, or WEATHER_NO_TEMP if we have not got one yet. */
int         weather_store_temp_max(void);

/** @brief Today's low, or WEATHER_NO_TEMP if we have not got one yet. */
int         weather_store_temp_min(void);

/** @brief The chance of rain in percent, or -1 if we have not got one yet. */
int         weather_store_precip_chance(void);

/** @brief The hourly forecast strip. count is 0 until a reading lands. */
const WeatherHourly *weather_store_forecast_hourly(void);

/** @brief The 7-day forecast strip. count is 0 until a reading lands. */
const WeatherDaily *weather_store_forecast_daily(void);

/** @brief How many seconds since the last reading turned up, or -1 if we have none. */
int weather_store_age_s(void);

/** @} */
