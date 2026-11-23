/**
 * @file shape.h
 * @brief Fill or outline a shape straight from a table of corners, with no path to allocate.
 *
 * Each call puts a path on the stack over the table, so it lives for that one draw and nothing
 * needs freeing. The SDK only reads the points, so the tables can stay const.
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
 * @brief Fill a shape and leave its edge alone.
 *
 * @param ctx The graphics context.
 * @param pts The corners, in order.
 * @param count How many corners there are.
 * @param fill The colour inside.
 */
void shape_fill(GContext *ctx, const GPoint *pts, int count, GColor fill);

/**
 * @brief Draw a shape's edge 1px wide and leave its inside alone.
 *
 * @param ctx The graphics context.
 * @param pts The corners, in order.
 * @param count How many corners there are.
 * @param line The outline colour.
 */
void shape_outline(GContext *ctx, const GPoint *pts, int count, GColor line);

/**
 * @brief Fill a shape and then draw its edge 1px wide over the fill.
 *
 * It leaves the stroke colour on the outline colour and the width at 1, so linework drawn
 * straight after it in that colour needs neither set again.
 *
 * @param ctx The graphics context.
 * @param pts The corners, in order.
 * @param count How many corners there are.
 * @param fill The colour inside.
 * @param line The outline colour.
 */
void shape_fill_outlined(GContext *ctx, const GPoint *pts, int count, GColor fill, GColor line);

/** @} */
