/**
 * @file marks.c
 * @brief Grass tufts and a trunk with its branches, for the scenes that draw them.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/marks.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

void marks_tufts(GContext *ctx, const GPoint *pts, int count, GColor color)
{
    graphics_context_set_stroke_color(ctx, color);
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < count; i++)
    {
        GPoint at = pts[i];
        graphics_draw_line(ctx, at, GPoint(at.x - 2, at.y - 4));
        graphics_draw_line(ctx, at, GPoint(at.x + 1, at.y - 5));
        graphics_draw_line(ctx, at, GPoint(at.x + 3, at.y - 3));
    }
}

void marks_tree(GContext *ctx, GPoint foot, GPoint top, const int8_t (*branches)[4], int count,
    GColor color)
{
    graphics_context_set_stroke_color(ctx, color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, foot, top);

    graphics_context_set_stroke_width(ctx, 1);
    for (int i = 0; i < count; i++)
    {
        GPoint bend = GPoint(top.x + branches[i][0], top.y + branches[i][1]);
        GPoint tip = GPoint(top.x + branches[i][2], top.y + branches[i][3]);
        graphics_draw_line(ctx, top, bend);
        graphics_draw_line(ctx, bend, tip);
    }
}

/** @} */
