/**
 * @file marks.h
 * @brief Small pen marks a scene draws: grass tufts.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Tuft the ground at each point, so it reads as grass rather than blank ground.
 *
 * Three short strokes fanning up off a point, which is how the rest of the face draws. An ordered
 * dot fill covers the same ground but reads as a printing screen rather than as grass. It leaves
 * the stroke colour on the tufts' colour and the width at 1.
 *
 * @param ctx The graphics context.
 * @param pts Where each tuft grows from.
 * @param count How many tufts there are.
 * @param color The tufts' colour.
 */
void marks_tufts(GContext *ctx, const GPoint *pts, int count, GColor color);

/** @} */
