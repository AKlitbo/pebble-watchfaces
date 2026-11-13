/**
 * @file system_battery_pair.h
 * @brief The paired battery panel: the watch's battery and the phone's side by side.
 * @ingroup gridlock_mod_system
 */
#pragma once
#include "mosaic/engine/catalog.h"

/**
 * @brief Draws the paired battery panel. A watch glyph with the watch's level and a phone glyph with
 * the phone's, split down the middle at 1x2 and stacked with a gauge each at 2x2.
 */
extern const ModuleDef mod_system_battery_pair_def;
