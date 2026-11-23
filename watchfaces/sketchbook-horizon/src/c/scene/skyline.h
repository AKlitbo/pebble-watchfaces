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

#include "scene/frame.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the ground under the sky: the city, the river, the bridge and its lamps.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void skyline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather: the channel buoy with the temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void skyline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/// Where this scene's sky and weather sit, for the shared drawing code
extern const SceneGeometry skyline_geometry;

/** @} */
