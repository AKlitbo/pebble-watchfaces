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
#include "theme/skyline.h"

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
#define LM_SCENE_BOTTOM 133  ///< The last rectangle row the scene draws into before the clock

/**
 * @brief Lay water from the horizon down and ripple it, as a river or a bay reads at rest.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param water The water's colour.
 * @param bank The colour of the far bank's edge.
 * @param ripple The colour of the broken marks on the water.
 */
void landmarks_draw_water(GContext *ctx, GRect bounds, GColor water, GColor bank, GColor ripple);

#if defined(PBL_ROUND)
/**
 * @brief Plant a board on two posts with the temperature on it, sized to the reading.
 *
 * The posts go in first, so the board closes over their tops. The rim and the lettering share a
 * colour, which is the one the board is paired with. With no reading there is no sign, rather
 * than a board reading "--".
 *
 * @param ctx The graphics context.
 * @param x The board's centre line.
 * @param pad Air either side of the reading, inside the board.
 * @param radius How round the board's corners are.
 * @param post The posts' colour.
 * @param board The board's fill.
 * @param ink The board's rim and its lettering.
 */
void landmarks_draw_sign(GContext *ctx, int x, int pad, int radius, GColor post, GColor board, GColor ink);
#endif

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

/**
 * @brief Paint the Niagara Falls scene's ground: the river, the falls and their mist, the rainbow
 * by day, the cliffs, and the water below.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void niagara_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Niagara Falls scene: the barrel with the
 * temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void niagara_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/**
 * @brief Paint the Uluru scene's ground: the rock, the sand and its spinifex, the oak, and on the
 * rectangle the tank.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void uluru_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Uluru scene: the tank with the
 * temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void uluru_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/**
 * @brief Paint the Half Dome scene's ground: the valley walls, the dome, the climbers' headlamps
 * after dark, the meadow, the pines and the river.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void half_dome_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Half Dome scene: the sign with the
 * temperature on it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a RidgelineGround.
 */
void half_dome_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/**
 * @brief Paint the Table Mountain scene's ground: the mountain, the cable, the tablecloth when
 * there is cloud about, the city, the bay, and on the rectangle the cable car.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void table_mountain_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame);

#if defined(PBL_ROUND)
/**
 * @brief Paint what stands in front of the weather in the Table Mountain scene: the cable car with
 * the temperature on its side.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param frame This frame's reading. Its ground row is a SkylineGround.
 */
void table_mountain_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame);
#endif

/** @} */
