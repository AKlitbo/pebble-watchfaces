/**
 * @file dev_walk.h
 * @brief Shared dev harness for the frame faces: seeds every store with a fixed fixture (via
 * the store seed mechanism, so no live source overwrites it) and drives two accelerometer-tap
 * "walks" for deterministic screenshots. A theme walk (tap through every theme on fixed data
 * and a fixed clock) and an icon walk (force one theme, tap through every weather condition).
 *
 * lib owns the walks + the fixture + the token list. A face owns only its switches (see each
 * face's src/c/dev/dev.h) and passes them in. Always compiled but never called by a release
 * build, so the linker drops it all.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Which walk a tap advances. Derived by the face from its own toggles.
 */
typedef enum
{
    DEV_WALK_NONE,   // just pin the fixture (no tap handler)
    DEV_WALK_THEMES, // a tap steps to the next theme
    DEV_WALK_ICONS   // a tap steps to the next weather condition token
} DevWalkMode;

/**
 * @brief Init every store from the default fixture with live=false + a pinned clock, so the
 * face shows fixed data no live reading can stomp. Call in place of the live store inits.
 *
 * @param hour The pinned clock hour (0-23).
 * @param min The pinned clock minute.
 */
void dev_walk_seed_stores(int hour, int min);

/**
 * @brief Start the chosen walk: force the initial theme/condition, paint, and (for a real
 * walk) subscribe the accelerometer tap. Call after engine_init.
 *
 * @param mode Which walk to run.
 * @param icon_theme The theme the icon walk sits on (ignored by the theme walk).
 * @param apply_theme The face's own theme-apply hook (re-colours zones + swaps the frame),
 * run before the engine rebuilds so a theme change lands.
 */
void dev_walk_init(DevWalkMode mode, uint8_t icon_theme, void (*apply_theme)(void));

/** @brief Drop the tap subscription. Call from deinit. */
void dev_walk_deinit(void);

/** @} */
