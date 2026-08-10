/**
 * @file stock_common.h
 * @brief Bits the stock panels share: the up/down trend colour, the little filled
 * triangle that shows the direction, and the price/percent text formatters. The 2x2
 * quote and the 2x4 watchlist both lean on these so the look stays the same.
 *
 * @ingroup gridlock_mod_stock
 */
#pragma once
#include <pebble.h>
#include "engine/grid_engine.h"

/**
 * @addtogroup gridlock_mod_stock
 * @{
 */

/**
 * @brief The colour for a change. Green for a rise, red for a fall, the subtitle grey
 * for flat.
 *
 * @param gctx The grid context (for the flat colour).
 * @param change_pct The percent change times 100, signed.
 * @return The colour to paint the change in.
 */
GColor stock_trend_color(GridCtx *gctx, int change_pct);

/**
 * @brief Draws a small filled triangle in the box, pointing up for a rise and down for a
 * fall. Nothing is drawn when the change is flat. The caller places the box.
 *
 * @param gctx The grid context.
 * @param box Where the triangle goes.
 * @param change_pct The percent change times 100, signed.
 */
void stock_draw_trend_triangle(GridCtx *gctx, GRect box, int change_pct);

/** @} */
