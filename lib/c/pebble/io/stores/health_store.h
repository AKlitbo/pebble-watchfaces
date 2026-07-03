/**
 * @file health_store.h
 * @brief Active health store. It subscribes to the watch's health service itself and holds
 * the readings, so a face just hands it the rules, optionally seeds it, then reads the numbers
 * back.
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
    bool enabled; // false = inert
    bool live;    // true = subscribe the health service. false = seed-only (dev/screenshots)
} HealthConfig;

/**
 * @brief Optional prefill, or pinned readings when live = false.
 */
typedef struct
{
    int hr;
    int steps;
    int calories;
    int sleep_min;
    int active_min;
    int distance_m;
} HealthSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live).
 * @param seed Optional prefill, or NULL.
 */
void health_store_init(HealthConfig cfg, const HealthSeed *seed);

/** @brief Hand it the function to call when the numbers change (the screen redraw). */
void health_store_subscribe(void (*cb)(void));

/**
 * @brief The last hour of heart rate readings, one per minute (a 60 slot buffer). The store
 * fills it on each update. The little charts read it back out.
 */
uint8_t *health_store_hr_history(void);

int health_store_hr(void);         // beats per minute (-1 means none yet)
int health_store_steps(void);      // steps so far today (-1 means none yet)
int health_store_calories(void);   // active calories (-1 means none yet)
int health_store_sleep_min(void);  // minutes of sleep (-1 means none yet)
int health_store_active_min(void); // active minutes (-1 means none yet)
int health_store_distance_m(void); // today's walked distance in meters (0 if unavailable)

/** @} */
