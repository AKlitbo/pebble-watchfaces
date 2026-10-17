/**
 * @file shoreline.h
 * @brief The Shoreline scene: sky bands, the sun or moon on its arc and its glitter on the water,
 * the sea, a boat riding the tide, the surf line, and the beach the clock sits on.
 *
 * Shoreline has no baked background. Everything above the clock is painted from the time, the
 * sunrise and sunset the phone sends, the tide, and the current condition, which is what lets the
 * same scene read as a bright low-tide morning or a rough night at high water.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/shoreline.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, glitter, sea, waves, boat, surf, and
 * the weather on top.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void shoreline_scene_draw(GContext *ctx, GRect bounds, const ShorelinePalette *pal);

/** @brief Where this scene's sky and weather sit, for the shared drawing code. */
const SceneGeometry *shoreline_geometry(void);

/** @} */
