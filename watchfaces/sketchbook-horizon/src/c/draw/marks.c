/**
 * @file marks.c
 * @brief Grass tufts, for the scenes that draw them.
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

/** @} */
