/**
 * @file panel.h
 * @brief Paints one gridlock panel into a cell.
 *
 * The border, the header block, the checker dither and the label, drawn the way gridlock's engine
 * draws them. There is no layout half beside it: sidereel has no grid to resolve, so the whole
 * job is, given a cell and a module, draw it.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "mosaic/engine/catalog.h"
#include "theme/theme.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/// The header strip's height. A 1x2 cell is 40 tall, so the body under it gets 26
#define HEADER_H 14

/**
 * @brief Paint a module's panel.
 *
 * A module that does not declare the size it is being asked for draws nothing rather than drawing
 * wrong, which is the same gate gridlock's layout applies before it resolves a cell.
 *
 * @param ctx The graphics context.
 * @param bounds The cell, in the layer's own coordinates.
 * @param type Which module to draw.
 * @param size The cell's footprint.
 */
void panel_draw(GContext *ctx, GRect bounds, ModuleType type, ModuleSize size);

/**
 * @brief Fill a rect with the 50% checker the panel headers screen themselves with.
 *
 * Exposed so the face can screen its own chrome with the same dither rather than keep a second
 * copy of it. The pattern is a cached bitmap, so this is a blit and not a per-pixel loop.
 *
 * @param ctx The graphics context.
 * @param area The rect to fill.
 * @param color The colour to lay the pattern in.
 */
void panel_checker(GContext *ctx, GRect area, GColor color);

/** @brief Free the cached checker bitmaps and every module's held picture. */
void panel_cleanup(void);

/** @} */
