/**
 * @file dev.h
 * @brief Per-face dev-walk switches. Keep DEV_MODE 0 for any build you ship. Flip it (plus the
 * walk toggle) to boot the face into a fixed fixture and tap-walk themes for screenshots.
 * The logic lives in lib `dev/dev_walk`. This only holds the switches.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

// ===== master switch (0 for any shipped build) =====
#define DEV_MODE 0

// the tap-walk screenshot harness lives in the shared lib's dev/dev_walk. nothing outside
// DEV_MODE touches it, so a shipping build (0) needs neither the include nor the walk macro
#if DEV_MODE
#include "dev/dev_walk.h"
#endif

// ===== walk toggle =====
#define DEV_TAP_WALK_THEMES 0  // tap steps through every theme on fixed data + a fixed clock

// ===== theme walk: the fixed clock every shot shows =====
#define DEV_TIME_HOUR 8   // 08:00 is a wide clock string
#define DEV_TIME_MIN  0

// which walk a tap advances (from the toggle above)
#if DEV_TAP_WALK_THEMES
  #define DEV_WALK_MODE DEV_WALK_THEMES
#else
  #define DEV_WALK_MODE DEV_WALK_NONE
#endif

// the hooks are static inline so a release build (DEV_MODE 0) inlines them to nothing and
// leaves no dev symbol at all. main.c stays #if-free while shipping zero dev code

/**
 * @brief Seed the stores from the dev fixture (live=false) when DEV_MODE is on.
 *
 * @return True if it seeded, so main skips the live inits. False in a release build.
 */
static inline bool dev_seed_stores(void)
{
#if DEV_MODE
    dev_walk_seed_stores(DEV_TIME_HOUR, DEV_TIME_MIN);
    return true;
#else
    return false;
#endif
}

/**
 * @brief Start the walk (force theme, first paint, tap subscription) when DEV_MODE is on. A
 * no-op otherwise. Call after engine_init.
 *
 * @param apply_theme The face's theme-apply hook.
 */
static inline void dev_start(void (*apply_theme)(void))
{
#if DEV_MODE
    dev_walk_init(DEV_WALK_MODE, apply_theme);
#else
    (void)apply_theme;
#endif
}

/** @brief Stop the walk (drop the tap) when DEV_MODE is on. A no-op otherwise. */
static inline void dev_stop(void)
{
#if DEV_MODE
    dev_walk_deinit();
#endif
}
