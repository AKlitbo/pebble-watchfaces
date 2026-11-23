/**
 * @file ground.c
 * @brief The ground or the water under a scene, with the edge along its top.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/ground.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

void ground_fill(GContext *ctx, GRect bounds, int top, GColor fill, GColor edge)
{
    graphics_context_set_fill_color(ctx, fill);
    graphics_fill_rect(ctx, GRect(0, top, bounds.size.w, bounds.size.h - top), 0, GCornerNone);

    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, edge);
    graphics_draw_line(ctx, GPoint(0, top), GPoint(bounds.size.w - 1, top));
}

void ground_water(GContext *ctx, GRect bounds, int top, int ripple_top, int bottom,
    GColor water, GColor bank, GColor ripple, AnimPhase anim)
{
    // the far bank is the only hard edge the water has
    ground_fill(ctx, bounds, top, water, bank);

    graphics_context_set_stroke_color(ctx, ripple);
    for (int y = ripple_top; y < bottom; y += 7)
    {
        // in a burst each row slides one gap along, the rows taking turns which way
        int gap = 31 + y % 7;
        int way = ((y - ripple_top) / 7) % 2 ? 1 : -1;

        for (int x = anim_slide(anim, (y * 7) % 13, gap, way); x < bounds.size.w; x += gap)
        {
            graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 2 + y % 3, y));
        }
    }
}

/** @} */
