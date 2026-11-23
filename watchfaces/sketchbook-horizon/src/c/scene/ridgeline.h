/**
 * @file ridgeline.h
 * @brief The Ridgeline scene: sky bands, the sun or moon on its arc, two mountain ridges, and
 * whatever the weather is adding on top.
 *
 * Ridgeline has no baked background. Everything above the clock is painted from the time, the
 * sunrise and sunset the phone sends, and the current condition, which is what lets the same scene
 * read as a clear morning, an overcast afternoon, or a snowy night.
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
 * @brief Paint the ground under the sky: the two ridges, capped in snow when it snows, and on a
 * round screen the meadow and the trail.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void ridgeline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather: the trail sign with the temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void ridgeline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/// Where this scene's sky and weather sit, for the shared drawing code
extern const SceneGeometry ridgeline_geometry;

/** @} */
