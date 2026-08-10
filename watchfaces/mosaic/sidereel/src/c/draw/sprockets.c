/**
 * @file sprockets.c
 * @brief Punches the perforation row into the strip.
 *
 * @ingroup watchface-sidereel
 */
#include "sprockets.h"

#include "layout.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

// one hole every PITCH pixels. twelve of them divide the 228px screen exactly, so the row needs
// no partial hole at either end and stays put whatever the reel is doing
#define HOLE_PITCH  (SCREEN_H / 12)  // 19
#define HOLE_W      8
#define HOLE_H      10
#define HOLE_RADIUS 2

void sprockets_draw(GContext *ctx, GRect strip, GColor panel, GColor hole)
{
    graphics_context_set_fill_color(ctx, panel);
    graphics_fill_rect(ctx, strip, 0, GCornerNone);

    int left = strip.origin.x + (strip.size.w - HOLE_W) / 2;

    // the leftover of the pitch is split above and below each hole, which centres the row in the
    // strip without a separate margin to keep in step with the pitch
    int lead = (HOLE_PITCH - HOLE_H) / 2;

    graphics_context_set_fill_color(ctx, hole);

    for (int top = strip.origin.y + lead; top + HOLE_H <= strip.origin.y + strip.size.h;
         top += HOLE_PITCH)
    {
        graphics_fill_rect(ctx, GRect(left, top, HOLE_W, HOLE_H), HOLE_RADIUS, GCornersAll);
    }
}

/** @} */
