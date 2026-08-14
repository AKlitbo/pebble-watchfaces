/**
 * @file dev.h
 * @brief Per-face dev-walk switches. Keep DEV_MODE 0 for any build you ship. Flip it (plus one
 * walk toggle) to boot the face into a fixed fixture and tap-walk it for screenshots.
 *
 * Two walks: the shared theme walk in lib `dev/dev_walk`, and this face's ops-slot walk in
 * `dev/dev_ops`. Pick one. This file only holds the switches.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

// ===== master switch (0 for any shipped build) =====
#define DEV_MODE 0

// ===== walk toggles (pick one) =====
#define DEV_TAP_WALK_THEMES 0  // tap steps through every theme on fixed data + a fixed clock
#define DEV_TAP_WALK_OPS    0  // tap steps both ops slots through the readout catalog
#define DEV_TAP_WALK_WX     0  // tap steps the condition through every weather glyph

// the harnesses are only reachable through DEV_MODE so a shipping build (0) needs
// neither include nor any walk macro
#if DEV_MODE
#if DEV_TAP_WALK_OPS
#include "dev/dev_ops.h"
#elif DEV_TAP_WALK_WX
#include "dev/dev_wx.h"
#else
#include "dev/dev_walk.h"
#endif
#endif

// ===== the fixed clock every shot shows =====
// the ops walk is the exception: it carries a clock per frame in dev_ops.c, so a sheet of its
// readouts is not the same minute over and over
#define DEV_TIME_HOUR 8   // 08:00 is a wide clock string
#define DEV_TIME_MIN  0

// which walk a tap advances (from the toggles above)
#if DEV_TAP_WALK_THEMES
  #define DEV_WALK_MODE DEV_WALK_THEMES
#else
  #define DEV_WALK_MODE DEV_WALK_NONE
#endif

// the hooks are static inline so a release build (DEV_MODE 0) inlines them to nothing and
// leaves no dev symbol at all. main.c and layout.c stay #if-free while shipping zero dev code

/**
 * @brief Seed the stores from the dev fixture (live=false) when DEV_MODE is on.
 *
 * @return True if it seeded, so main skips the live inits. False in a release build.
 */
static inline bool dev_seed_stores(void)
{
#if DEV_MODE
#if DEV_TAP_WALK_OPS
    dev_ops_seed_stores();
#elif DEV_TAP_WALK_WX
    dev_wx_seed_stores(DEV_TIME_HOUR, DEV_TIME_MIN);
#else
    dev_walk_seed_stores(DEV_TIME_HOUR, DEV_TIME_MIN);
#endif
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
#if DEV_TAP_WALK_OPS
    dev_ops_init(apply_theme);
#elif DEV_TAP_WALK_WX
    dev_wx_init(apply_theme);
#else
    dev_walk_init(DEV_WALK_MODE, apply_theme);
#endif
#else
    (void)apply_theme;
#endif
}

/** @brief Stop the walk (drop the tap) when DEV_MODE is on. A no-op otherwise. */
static inline void dev_stop(void)
{
#if DEV_MODE
#if DEV_TAP_WALK_OPS
    dev_ops_deinit();
#elif DEV_TAP_WALK_WX
    dev_wx_deinit();
#else
    dev_walk_deinit();
#endif
#endif
}

/**
 * @brief What a slot should show: the walk's pick while the ops walk is running, otherwise
 * whatever the user saved.
 *
 * @param slot 0 for the upper slot, 1 for the lower.
 * @param stored The saved pick.
 * @return An OpsId.
 */
static inline uint8_t dev_ops_pick(int slot, uint8_t stored)
{
#if DEV_MODE && DEV_TAP_WALK_OPS
    (void)stored;
    return dev_ops_walk_pick(slot);
#else
    (void)slot;
    return stored;
#endif
}
