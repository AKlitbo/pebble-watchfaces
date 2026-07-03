/**
 * @file system_store.h
 * @brief Active system store: the watch's own battery + bluetooth state. It subscribes to the
 * battery and connection services itself and buzzes on a bluetooth transition through a policy
 * the face installs (so the store stays settings- and vibe-agnostic).
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

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
    bool enabled;      // false = inert
    bool live;         // true = subscribe the services. false = seed-only (dev/screenshots)
    BtVibePolicy vibe; // buzz policy for a bluetooth transition or NULL for silent
} SystemConfig;

/**
 * @brief Optional prefill, or pinned state when live = false.
 */
typedef struct
{
    int  battery;
    bool charging;
    bool bluetooth;
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

int  system_store_battery(void);   // how full from 0 to 100
bool system_store_charging(void);  // true while plugged in
bool system_store_bluetooth(void); // true when the phone link is up

/** @} */
