/**
 * @file line.c
 * @brief Lines drawn from a table of points, in whatever stroke the caller set.
 *
 * @ingroup watchface-sketchbook
 */
#include "draw/line.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

void line_polyline(GContext *ctx, const GPoint *pts, int count)
{
    for (int i = 0; i + 1 < count; i++)
    {
        graphics_draw_line(ctx, pts[i], pts[i + 1]);
    }
}

void line_dashed(GContext *ctx, const GPoint *pts, int count, int on, int period)
{
    // a period of 0 would never step along the leg, and a dash of 0 would draw nothing anyway
    if (on <= 0 || period <= 0)
    {
        return;
    }

    for (int i = 0; i + 1 < count; i++)
    {
        GPoint from = pts[i];
        GPoint to = pts[i + 1];
        int dx = to.x - from.x;
        int dy = to.y - from.y;
        int span = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);

        for (int at = 0; at < span; at += period)
        {
            int end = at + on < span ? at + on : span;
            graphics_draw_line(ctx,
                GPoint(from.x + (dx * at) / span, from.y + (dy * at) / span),
                GPoint(from.x + (dx * end) / span, from.y + (dy * end) / span));
        }
    }
}

/** @} */
