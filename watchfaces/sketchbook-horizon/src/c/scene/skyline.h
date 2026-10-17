/**
 * @file skyline.h
 * @brief The Skyline scene: sky bands, the sun or moon on its arc, a city standing back, a
 * suspension bridge over the river, and the lamps that come on at dusk.
 *
 * Skyline has no baked background. Everything above the clock is painted from the time, the
 * sunrise and sunset the phone sends, and the current condition, which is what lets the same
 * bridge read as a flat daytime silhouette or a string of lights on black water.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/skyline.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the whole scene: sky, stars, arc, disc, city, bridge, lamps and the water.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void skyline_scene_draw(GContext *ctx, GRect bounds, const SkylinePalette *pal);

/** @brief Where this scene's sky and weather sit, for the shared drawing code. */
const SceneGeometry *skyline_geometry(void);

/** @} */
