/**
 * @file landmarks.h
 * @brief The landmark scenes and the pieces they share.
 *
 * A landmark stands a real place on the same horizon Skyline uses, so it shares Skyline's stars,
 * its weather, and where its clock sits. The sky over it is the one every scene shares. Only its
 * ground is borrowed. A landmark on land takes Ridgeline's ground colours and one on water takes
 * Skyline's, so every palette already has a look for it, and the ground or the water under the
 * clock keeps the colours that palette picked for it.
 *
 * A landmark is drawn for the rectangle and carried to the round screen through LM_X and LM_Y,
 * which spread the picture to the wider glass and drop it to the round horizon. A width or a height
 * goes through LM_W and LM_H, which scale it the same way without moving it. What the round screen
 * adds, the holder that carries the temperature, is placed for it directly.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "scene/frame.h"
#include "theme/ridgeline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// a landmark is drawn once, on the rectangle's 200 by 228 grid. the round screen is 30% wider
// and its horizon sits at 131 rather than 108, so these carry a rectangle point across to it
#if defined(PBL_ROUND)
#define LM_X(x) (((x) * 13) / 10)    ///< A column on the rectangle, spread to the round screen
#define LM_Y(y) (((y) * 6) / 5 + 2)  ///< A row on the rectangle, dropped to the round screen
#define LM_H(h) (((h) * 6) / 5)      ///< A height on the rectangle, scaled like a row without its 2px drop
#else
#define LM_X(x) (x)
#define LM_Y(y) (y)
#define LM_H(h) (h)
#endif
#define LM_W(w) LM_X(w)  ///< A width on the rectangle, spread the same way a column is

#define LM_HORIZON 108       ///< The rectangle row the land or the water starts on, as on Skyline

/**
 * @brief Paint the Fuji scene's ground: the cone and its snow, the lake, and the forest along its
 * far shore.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void fuji_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Fuji scene: the boat with the temperature
 * on its sail.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void fuji_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/**
 * @brief Paint the Giza scene's ground: the pyramids, the sand and its dunes, and the palms.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void giza_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Giza scene: the stele with the
 * temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void giza_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/** @} */
