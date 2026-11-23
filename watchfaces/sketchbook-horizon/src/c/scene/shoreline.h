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

#include "scene/frame.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the ground under the sky: the gulls by day, the sea and the sun or moon's glitter
 * on it, the waves, the boat, the surf, and the beach.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a ShorelineGround.
 */
void shoreline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather: the boat's mast, flying the temperature or a
 * sail.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a ShorelineGround.
 */
void shoreline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/// Where this scene's sky and weather sit, for the shared drawing code
extern const SceneGeometry shoreline_geometry;
/// Where this scene's storm strikes its bolts
extern const BoltSpots shoreline_bolts;

/** @} */
