/**
 * @file dev.h
 * @brief Sidereel's dev overrides. The dev plugin's `dev/dev_hooks.h` holds the harness hooks, and
 * the build sets their switches from `paf build sidereel --dev` and the family's paf.dev.config.json.
 * This only adds the look a shot pins and the forced Quiet Time glyph. dev.c beside it shows one panel
 * on its own for the panel walk the config lists.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "dev/dev_hooks.h"
#include "system/settings/setting_values.h"
#include "system/settings/settings.h"
#include "theme/theme.h"

// ===== fixture overrides =====
// quiet time is a real watch state the emulator never enters, so the pennant glyph would never
// appear in a shot. this forces it on for the walk
#define DEV_FORCE_QUIET 1

// the look a shot should show. without these a screenshot depends on whatever the phone last
// pushed, so the theme and the clock format are pinned rather than left to the stored settings
#define DEV_FORCE_THEME    THEME_VIBRANT
#define DEV_FORCE_TIME_FMT TIME_FORMAT_24H

// the hooks are static inline so a release build (DEV_MODE 0) inlines them to nothing and
// leaves no dev symbol at all. main.c stays #if-free while shipping zero dev code

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
