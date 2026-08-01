#pragma once
#include <pebble.h>
#include "engine/grid_engine.h"
#include "engine/catalog.h"

/**
 * @brief Draws a thin progress bar along the bottom of a body box. It outlines the
 * bar and fills it up to the given level.
 *
 * @param gctx The grid context.
 * @param level How full the bar should be from 0 to 100.
 * @param icon_w The width of the icon sharing the row so the bar leaves room for it.
 */
void draw_progress_bar(GridCtx *gctx, int level, int icon_w);
