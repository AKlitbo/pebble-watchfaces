/**
 * @file shape.h
 * @brief Fill a shape straight from a table of corners, with no path to allocate.
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

/** @} */
