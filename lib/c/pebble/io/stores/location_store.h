/**
 * @file location_store.h
 * @brief Active location store. It owns the appmessage coords channel (the same phone
 * geolocation the weather rides on), so a face hands it the rules, optionally seeds it, then
 * reads lat/lon back. There's no poll timer. Coords arrive alongside weather.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_stores
 * @{
 */

/**
 * @brief The rules a face hands the store.
 */
typedef struct
{
    bool enabled; ///< False makes the store do nothing for a face without coordinate slots
    bool live;    ///< True subscribes the coordinates channel and false just keeps the fake data for screenshots
} LocationConfig;

/**
 * @brief Optional prefill, or pinned coords when live = false.
 */
typedef struct
{
    const char *lat; ///< Latitude string
    const char *lon; ///< Longitude string
} LocationSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live).
 * @param seed Optional prefill, or NULL.
 */
void location_store_init(LocationConfig cfg, const LocationSeed *seed);

/** @brief Hand it the function to call when the coordinates change (the screen redraw). */
void location_store_subscribe(void (*cb)(void));

/** @brief The latitude string, or empty if we have not got one yet. */
const char *location_store_lat(void);

/** @brief The longitude string, or empty if we have not got one yet. */
const char *location_store_lon(void);

/** @} */
