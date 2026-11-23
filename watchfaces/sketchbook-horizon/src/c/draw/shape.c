/**
 * @file shape.c
 * @brief Fill a shape from a table of corners, through a path on the stack.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/shape.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief A path over a table of corners, to draw and then drop.
 *
 * @param pts The corners, in order.
 * @param count How many corners there are.
 * @return The path, on the caller's stack.
 */
static GPath stack_path(const GPoint *pts, int count)
{
    // the SDK only reads the points, so the cast away from const is safe and the tables can stay
    // in flash
    return (GPath){.num_points = (uint32_t)count, .points = (GPoint *)pts, .rotation = 0, .offset = GPointZero};
}

void shape_fill(GContext *ctx, const GPoint *pts, int count, GColor fill)
{
    GPath path = stack_path(pts, count);

    graphics_context_set_fill_color(ctx, fill);
    gpath_draw_filled(ctx, &path);
}

/** @} */
