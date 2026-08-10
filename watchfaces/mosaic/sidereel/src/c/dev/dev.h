/**
 * @file dev.h
 * @brief Per-face dev-walk switches. Keep DEV_MODE 0 for any build you ship. Flip it (plus the
 * walk toggle) to boot the face into a fixed fixture and tap-walk themes for screenshots.
 * The logic lives in lib `dev/dev_walk`. This only holds the switches.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "system/settings/setting_values.h"
#include "system/settings/settings.h"
#include "theme/theme.h"

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

// ===== fixture overrides =====
// quiet time is a real watch state the emulator never enters, so the pennant glyph would never
// appear in a shot. this forces it on for the walk
#define DEV_FORCE_QUIET 1

// the look a shot should show. without these a screenshot depends on whatever the phone last
// pushed, so the theme and the clock format are pinned rather than left to the stored settings
#define DEV_FORCE_THEME    THEME_VIBRANT
#define DEV_FORCE_TIME_FMT TIME_FORMAT_24H

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
 * @brief Pins the theme and clock format a shot should show, when DEV_MODE is on.
 *
 * Call straight after settings_init, before the face loads its fonts or resolves a palette, so
 * everything downstream reads the pinned values rather than whatever the phone last pushed. A
 * no-op in a release build, where the stored settings are the whole truth.
 */
static inline void dev_force_settings(void)
{
#if DEV_MODE
    settings_set_u8(SETTING_THEME, DEV_FORCE_THEME);
    settings_set_u8(SETTING_TIME_FORMAT, DEV_FORCE_TIME_FMT);
#endif
}

/**
 * @brief Overrides a struct tm with the dev fixture's fixed clock when DEV_MODE is on.
 *
 * A panel that builds its own tm (a second time zone, say) calls this so a screenshot walk shows
 * the same time everywhere. A no-op in a release build.
 *
 * @param t The time to override.
 */
static inline void dev_force_time(struct tm *t)
{
#if DEV_MODE
    // the same fixed clock dev_walk_seed_stores pins, so a panel that builds its own tm reads
    // the hour and minute every other panel is showing
    t->tm_hour = DEV_TIME_HOUR;
    t->tm_min = DEV_TIME_MIN;
#else
    (void)t;
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

/**
 * @brief Whether to draw the pennant's quiet-time glyph regardless of settings or watch state.
 *
 * Both halves of the real condition are off by default: the setting ships off, and the emulator
 * never enters quiet time. So the walk forces the whole thing rather than either half.
 *
 * @return True only under DEV_MODE with the override on.
 */
static inline bool dev_force_quiet(void)
{
#if DEV_MODE && DEV_FORCE_QUIET
    return true;
#else
    return false;
#endif
}
