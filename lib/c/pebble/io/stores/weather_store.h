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
    const char *temp;  // "23C"
    const char *cond;  // "SUNNY"
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

/** @brief How many seconds since the last reading turned up, or -1 if we have none. */
int weather_store_age_s(void);

/** @} */
