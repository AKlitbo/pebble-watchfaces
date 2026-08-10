/**
 * @file stock_common.c
 * @brief Shared stock drawing helpers. See the header for the shape.
 * @ingroup gridlock_mod_stock
 */
#include "modules/stock/stock_common.h"
#include <stdio.h>
#include <stdlib.h>

GColor stock_trend_color(GridCtx *gctx, int change_pct)
{
    if (change_pct > 0)
    {
        return GColorGreen;
    }
    if (change_pct < 0)
    {
        return GColorRed;
    }

    return gctx->color_subtitle;
}

void stock_draw_trend_triangle(GridCtx *gctx, GRect box, int change_pct)
{
    // flat means no direction to show
    if (change_pct == 0)
    {
        return;
    }

    int left = box.origin.x;
    int right = box.origin.x + box.size.w;
    int top = box.origin.y;
    int bottom = box.origin.y + box.size.h;
    int mid_x = box.origin.x + box.size.w / 2;

    GPoint points[3];
    if (change_pct > 0)
    {
        // apex up, base along the bottom
        points[0] = GPoint(mid_x, top);
        points[1] = GPoint(left, bottom);
        points[2] = GPoint(right, bottom);
    }
    else
    {
        // apex down, base along the top
        points[0] = GPoint(left, top);
        points[1] = GPoint(right, top);
        points[2] = GPoint(mid_x, bottom);
    }

    GPathInfo info = { .num_points = 3, .points = points };
    GPath *path = gpath_create(&info);
    if (!path)
    {
        return;
    }

    graphics_context_set_fill_color(gctx->ctx, stock_trend_color(gctx, change_pct));
    gpath_draw_filled(gctx->ctx, path);
    gpath_destroy(path);
}

