/**
 * @file dev.h
 * @brief The dev mode switch and its hooks. DEV_MODE is 0 for any build you ship, and the hooks
 * are static inline so a release build leaves no dev symbol at all. main.c calls them straight,
 * with no preprocessor branch anywhere in it.
 *
 * The screenshot harness that fills these in is not kept in the repo. It builds its own layouts to
 * walk the modules, the store layouts and the starter presets, which is a lot of machinery for
 * something only ever run by hand.
 *
 * @ingroup gridlock_dev
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup gridlock_dev
 * @{
 */

// === master switch ===
// keep this 0 for any build you ship
#define DEV_MODE 0

/**
 * @brief Applies the theme, clock and tap-mode overrides into the settings structs.
 *
 * Writes the settings blobs only, so it belongs right after settings_init and before the hubs and
 * the engine come up.
 */
static inline void dev_apply_overrides(void) {}

/**
 * @brief Seeds every store from a fixture so a screenshot is deterministic.
 *
 * @return True when the stores were seeded, false to let init bring them up live.
 */
static inline bool dev_seed_stores(void) { return false; }

/**
 * @brief Overrides a tm's hour and minute with a forced dev time.
 *
 * @param t The tm to rewrite in place.
 */
static inline void dev_force_time(struct tm *t) { (void)t; }

/**
 * @brief Subscribes the accel tap handler for whichever walk is switched on.
 *
 * @param window The face's window. A walk that changes theme per shot needs it, since the
 * background is a theme colour and the two have to stay in step.
 */
static inline void dev_taps_init(Window *window) { (void)window; }

/**
 * @brief Drops the tap subscription. Call from deinit.
 */
static inline void dev_taps_deinit(void) {}

/** @} */
