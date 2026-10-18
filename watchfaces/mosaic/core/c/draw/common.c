#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/common.h"
#include "ui/fonts.h"
#include "mosaic/draw/icons.h"

void draw_progress_bar(GridCtx *gctx, int level, int icon_w)
{
    if (level < 0) return;
    int bar_h = 4;
    int gap = 6;
    int bar_w = gctx->body.size.w - PANEL_PAD * 2 - icon_w - gap;
    if (bar_w <= 0) return;

    // sit it bottom left with PANEL_PAD (+1 taste nudge) of space from the left and 5px up
    GRect bar_rect = grid_anchor(gctx, GSize(bar_w, bar_h), GAlignBottomLeft, PANEL_PAD + 1, -5);

    gh_progress_bar(gctx, bar_rect, level);
}
