/**
 * @file frame.h
 * @brief What the engine works out once a frame and hands to everything that paints the scene.
 *
 * The sky, the weather and many of the scenes read the same few things: whether it is after dark,
 * what the condition is doing and what that does to the sky, where the sun or moon sits, and the
 * minute. The layout engine reads those once before it paints, so the sky, the weather and the
 * scene agree on them inside one frame. Where a burst has got to rides along too, so everything that
 * moves reads the same frame of it. Anything else a scene draws from, such as the tide, the wind or
 * how lit the evening is, it reads for itself.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "anim/motion.h"
#include "fx/conditions.h"
#include "theme/palette.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief One frame's reading of the clock, the sun and the weather.
 *
 * The palette is the shared sky put together with the scene's readouts, and is all the shared sky
 * and weather code reads. The ground is the scene's own row, passed as `const void *` since each
 * scene has its own ground type, and the scene's draws cast it back.
 */
typedef struct SceneFrame
{
    const CommonPalette *palette;  ///< The palette for this hour
    const void *ground;            ///< The scene's ground row for this hour
    bool night;                    ///< After dark, by the sunrise and sunset the sky follows
    WxCond cond;                   ///< The condition, for a scene with its own rule over it
    SketchbookSky sky;             ///< What that condition does to the sky
    GPoint disc;                   ///< Where the sun or moon sits on its arc
    int minute;                    ///< The clock minute, which shuffles the falling weather
    AnimPhase anim;                ///< Where a burst has got to, with its tick at 0 when still
    bool lit;                      ///< A strike lights this frame, with the palette a step lighter
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
