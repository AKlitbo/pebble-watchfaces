/**
 * @file frame.h
 * @brief What the engine works out once a frame and hands to everything that paints the scene.
 *
 * The sky, the weather and many of the scenes read the same few things: whether it is after dark,
 * what the condition is doing and what that does to the sky, where the sun or moon sits, and the
 * minute. The layout engine reads those once before it paints, so the sky, the weather and the
 * scene agree on them inside one frame. Anything else a scene draws from, such as the tide, the
 * wind or how lit the evening is, it reads for itself.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "fx/conditions.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief One frame's reading of the clock, the sun and the weather.
 *
 * The palette is the scene's own, passed as `const void *` since each scene has its own palette
 * type. The shared sky and weather code reads it through its leading CommonPalette block, and a
 * scene's own draws cast it back to the full palette.
 */
typedef struct SceneFrame
{
    const void *palette;  ///< The scene's palette for this hour
    bool night;           ///< After dark, by the sunrise and sunset the sky follows
    WxCond cond;          ///< The condition, for a scene with its own rule over it
    SketchbookSky sky;    ///< What that condition does to the sky
    GPoint disc;          ///< Where the sun or moon sits on its arc
    int minute;           ///< The clock minute, which shuffles the falling weather
} SceneFrame;

/**
 * @brief One of a scene's draws, handed the frame the engine worked out.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading.
 */
typedef void (*SceneDraw)(GContext *ctx, GRect bounds, const SceneFrame *frame);

/** @} */
