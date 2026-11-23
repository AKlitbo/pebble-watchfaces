/**
 * @file treeline.h
 * @brief The Treeline scene: sky bands, the sun or moon on its arc, two treelines, a log cabin
 * with a lit window, the smoke off its chimney leaning on the wind, and whatever the weather is
 * adding on top.
 *
 * Treeline has no baked background. Everything above the clock is painted from the time, the
 * sunrise and sunset the phone sends, the wind it reports, and the current condition, which is
 * what lets the same clearing read as a still summer morning or a gale after dark.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, treelines, cabin, smoke and weather.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param palette The palette in use, a TreelinePalette.
 */
void treeline_scene_draw(GContext *ctx, GRect bounds, const void *palette);

/// Where this scene's sky and weather sit, for the shared drawing code
extern const SceneGeometry treeline_geometry;

/** @} */
