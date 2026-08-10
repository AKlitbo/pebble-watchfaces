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

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    graphics_draw_rect(gctx->ctx, bar_rect);

    if (level > 0)
    {
        int fill_w = ((bar_rect.size.w - 2) * level) / 100;
        if (fill_w > 0)
        {
            GRect fill_rect = GRect(bar_rect.origin.x + 1, bar_rect.origin.y + 1, fill_w, bar_rect.size.h - 2);
            graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
            graphics_fill_rect(gctx->ctx, fill_rect, 0, GCornerNone);
        }
    }
}
