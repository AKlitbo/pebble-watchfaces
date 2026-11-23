/**
 * @file ground.h
 * @brief The ground or the water a scene lays from one row down to the foot of the screen, with
 * the hard edge along its top.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "anim/motion.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Fill from a row down to the foot of the screen and rule a 1px edge along that row.
 *
 * It leaves the stroke colour on the edge colour and the width at 1.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param top The row the fill and its edge start on.
 * @param fill The ground's colour.
 * @param edge The colour of the line along its top.
 */
void ground_fill(GContext *ctx, GRect bounds, int top, GColor fill, GColor edge);

/**
 * @brief Lay water from a row down, rule its far bank, and scatter broken ripples across it.
 *
 * The ripples run every seventh row from ripple_top until they reach bottom, with a mark of 3 to
 * 5px every 31 to 37 columns. That is enough to read as water and sparse enough to keep the clock
 * that sits on it clear.
 *
 * In a burst each row of ripples slides one gap along, the rows taking turns which way, so the
 * water moves and lands back where it started.
 *
 * It leaves the stroke colour on the ripple colour and the width at 1.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param top The row the water and its bank start on.
 * @param ripple_top The row the first ripples sit on.
 * @param bottom The ripples stop before this row.
 * @param water The water's colour.
 * @param bank The colour of the far bank's edge.
 * @param ripple The colour of the broken marks on the water.
 * @param anim Where the burst has got to.
 */
void ground_water(GContext *ctx, GRect bounds, int top, int ripple_top, int bottom,
    GColor water, GColor bank, GColor ripple, AnimPhase anim);

/** @} */
