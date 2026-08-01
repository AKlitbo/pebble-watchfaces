#include "engine/grid_engine.h"
#include "time_weekday_dots.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"

// dot sizing. today gets the full radius and the rest draw one pixel thinner outlines
#define DOT_RADIUS 4

static void weekday_dots_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    // the row runs from the configured first day of the week to six days later
    int first = gridlock_week_start();
    int today = (time_store_tm()->tm_wday - first + 7) % 7;

    int inner_w = gctx->body.size.w - EDGE_PAD(gctx) * 2;
    int step = inner_w / 7;
    GRect strip = grid_anchor(gctx, GSize(step * 7, DOT_RADIUS * 2), GAlignCenter, 0, 0);
    int cy = strip.origin.y + DOT_RADIUS;

    for (int i = 0; i < 7; i++)
    {
        GPoint centre = GPoint(strip.origin.x + i * step + step / 2, cy);
        if (i == today)
        {
            graphics_context_set_fill_color(gctx->ctx, gctx->color_value);
            graphics_fill_circle(gctx->ctx, centre, DOT_RADIUS);
        }
        else
        {
            graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
            graphics_draw_circle(gctx->ctx, centre, DOT_RADIUS - 1);
        }
    }
}

const ModuleDef mod_time_weekday_dots_def = {
    .label = "WEEKDAY",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .body = weekday_dots_body,
};
