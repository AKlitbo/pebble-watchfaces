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

#include "scene/frame.h"
#include "scene/geometry.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Paint the ground under the sky: the chimney smoke, the two treelines, the clearing and
 * the cabin, with snow along them when it snows.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a TreelineGround.
 */
void treeline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather: the board with the temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a TreelineGround.
 */
void treeline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/// Where this scene's sky and weather sit, for the shared drawing code
extern const SceneGeometry treeline_geometry;

/** @} */
