/**
 * @file location_store.h
 * @brief Active location store. It owns the appmessage coords channel (the same phone
 * geolocation the weather rides on), so a face hands it the rules, optionally seeds it, then
 * reads lat/lon back. There's no poll timer. Coords arrive alongside weather.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief The rules a face hands the store.
 */
typedef struct
{
    bool enabled; // false = inert (no channel holds nothing) for a face without coord slots
    bool live;    // true = subscribe the coords channel. false = seed-only (dev/screenshots)
} LocationConfig;

/**
 * @brief Optional prefill, or pinned coords when live = false.
 */
typedef struct
{
    const char *lat;
    const char *lon;
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

const char *location_store_lat(void); // latitude string (empty until we get one)
const char *location_store_lon(void); // longitude string (empty until we get one)

/** @} */
