/**
 * @file dev.h
 * @brief The dev hooks. The dev plugin's `dev/dev_hooks.h` holds them, and the build sets their
 * switches from `paf build gridlock --dev` and the family's paf.dev.config.json, which also lays out
 * the settings, the readings, and the walks a dev build runs. The hooks are static inline, so a release
 * build leaves no dev symbol at all, and main.c calls them with no preprocessor branch anywhere in it.
 *
 * dev.c beside it shows one panel or one layout for the walks the config lists, which is the only dev
 * code Gridlock keeps.
 *
 * @ingroup gridlock_dev
 */
#pragma once
#include <pebble.h>

#include "dev/dev_hooks.h"
