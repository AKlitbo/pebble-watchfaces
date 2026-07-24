/**
 * @file system_store.h
 * @brief Active system store: the watch's own battery and bluetooth state. It subscribes to the
 * battery and connection services itself and buzzes on a bluetooth change through a policy
 * the face installs (so the store never has to know about settings or buzzing).
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
 * @brief Called on a real bluetooth transition (a change, not the first reading), so the face
 * can buzz however its own settings say. NULL for a silent link change.
 *
 * @param connected True when the link just came up, false when it just dropped.
 */
typedef void (*BtVibePolicy)(bool connected);

/**
 * @brief The rules a face hands the store.
 */
typedef struct
{
    bool enabled;      ///< False makes the store do nothing
    bool live;         ///< True subscribes the services and false just keeps the fake data for screenshots
    BtVibePolicy vibe; ///< Buzz policy for a bluetooth change, or NULL for silent
} SystemConfig;

/**
 * @brief Optional prefill, or pinned state when live = false.
 */
typedef struct
{
    int  battery;   ///< Battery level from 0 to 100
    bool charging;  ///< True when plugged in
    bool bluetooth; ///< True when the phone link is up
} SystemSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live / vibe policy).
 * @param seed Optional prefill, or NULL.
 */
void system_store_init(SystemConfig cfg, const SystemSeed *seed);

/** @brief Stop tracking the battery + connection services. */
void system_store_deinit(void);

/** @brief Hand it the function to call when something changes (the screen redraw). */
void system_store_subscribe(void (*cb)(void));

/** @brief Hand it the function to call when the phone app link comes back after a drop. */
void system_store_on_reconnect(void (*cb)(void));

/** @brief How full the battery is from 0 to 100. */
int  system_store_battery(void);

/** @brief True while the watch is plugged in. */
bool system_store_charging(void);

/** @brief True when the phone link is up. */
bool system_store_bluetooth(void);

/** @} */
