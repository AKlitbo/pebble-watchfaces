/**
 * @file health_store.h
 * @brief Active health store. It subscribes to the watch's health service itself and holds
 * the readings, so a face just hands it the rules, optionally seeds it, then reads the numbers
 * back.
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
    bool     enabled;     ///< False makes the store do nothing
    bool     live;        ///< True subscribes the health service and false just keeps the fake data for screenshots
    uint32_t persist_key; ///< Slot for the saved history so the face owns the key instead of the store
} HealthConfig;

/**
 * @brief Optional prefill, or pinned readings when live = false.
 */
typedef struct
{
    int hr;         ///< Heart rate in beats per minute
    int steps;      ///< Steps so far today
    int calories;   ///< Active calories so far today
    int sleep_min;  ///< Minutes of sleep
    int active_min; ///< Active minutes so far today
    int distance_m; ///< Walked distance in meters
    const uint8_t *hr_history;   ///< Optional 60 slot minute by minute heart rate curve (0 means no reading), or NULL
    const uint16_t *step_hourly; ///< Optional 24 slot hour by hour step curve, or NULL
    int step_hours;              ///< How many of those hourly buckets are real (0 to 24)
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
 * @brief Sample the live heart rate into the rolling history. Call it once a minute (off the
 * face's minute tick) so the graph builds a continuous line rather than waiting on the sparse
 * heart rate events. A no-op in seed mode.
 */
void health_store_poll_hr(void);

/**
 * @brief The last hour of heart rate readings, one per minute (a 60 slot buffer). The store
 * fills it on each update. The little charts read it back out.
 */
uint8_t *health_store_hr_history(void);

/**
 * @brief Today's steps split into per-hour buckets, one slot per hour from midnight (24 slots).
 * The bar chart reads it back out.
 */
const uint16_t *health_store_step_hourly(void);

/**
 * @brief How many of the hourly step buckets hold a real value (0 to 24). Steps count 0 as a
 * real reading (a quiet hour), so a chart needs the count rather than a no-data sentinel.
 */
int health_store_step_hours(void);

/** @brief The heart rate in beats per minute, or -1 if we have not got one yet. */
int health_store_hr(void);

/** @brief The steps taken so far today, or -1 if we have not got one yet. */
int health_store_steps(void);

/** @brief The active calories so far today, or -1 if we have not got one yet. */
int health_store_calories(void);

/** @brief The minutes of sleep, or -1 if we have not got one yet. */
int health_store_sleep_min(void);

/** @brief The active minutes so far today, or -1 if we have not got one yet. */
int health_store_active_min(void);

/** @brief Today's walked distance in meters, or 0 if we cannot read it. */
int health_store_distance_m(void);

/** @} */
