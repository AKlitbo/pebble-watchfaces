#include "engine/grid_engine.h"
#include "health_hr_graph.h"
#include "draw/grid_helpers.h"
#include "math/scale.h"
#include "math/series.h"
#include "text/number_format.h"
#include "draw/common.h"
#include "draw/metrics.h"
#include "io/stores/health_store.h"
#include <stdio.h>

#define HR_SLOTS 60

// the plot is pinned to a fixed window so the trace's height reads as absolute bpm rather than
// just the day's own range. the floor is the baseline. the ceiling holds unless the day-high
// pushes past it. both are tunable
#define HR_FLOOR 40
#define HR_CEIL 120

// left gutter for the scale numbers. wide enough for a 3-digit value in FONT_STM_12 with a
// small gap before the plot
#define HR_SCALE_W 22

// draws one scale number right-aligned in the left gutter, its box top at box_top
static void hr_scale_label(GridCtx *gctx, int gutter_x, int gutter_w, int box_top, int value)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", value);
    GRect r = GRect(gutter_x, box_top, gutter_w, 14);
    graphics_draw_text(gctx->ctx, buf, fonts_get(FONT_STM_12), r,
                       GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

// draws the sparkline into area against the fixed floor..top window: one point per slot, real
// readings joined by lines and gaps left where there is no data
static void hr_draw_spark(GridCtx *gctx, GRect area, const uint8_t *history, int floor, int top)
{
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_value);
    graphics_context_set_stroke_width(gctx->ctx, 2);

    bool have_prev = false;
    GPoint prev = GPointZero;

    for (int i = 0; i < HR_SLOTS; i++)
    {
        int v = history[i];
        if (v <= 0)
        {
            have_prev = false; // a gap breaks the line
            continue;
        }

        // pin an out-of-window reading to an edge rather than plotting it off-panel
        int cv = clamp_int(v, floor, top);
        int x = area.origin.x + (i * (area.size.w - 1)) / (HR_SLOTS - 1);
        int y = plot_y(area.origin.y, area.size.h, floor, top, cv);
        GPoint p = GPoint(x, y);

        if (have_prev)
        {
            graphics_draw_line(gctx->ctx, prev, p);
        }
        else
        {
            // a lone reading with gaps either side still gets a dot
            graphics_draw_pixel(gctx->ctx, p);
        }

        prev = p;
        have_prev = true;
    }

    graphics_context_set_stroke_width(gctx->ctx, 1);
}

// TODO: re-tune for the taller body when gctx->headerless
static void health_hr_graph_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    const uint8_t *history = health_store_hr_history();
    int lo = 0;
    int hi = 0;
    int last = 0;
    int valid = series_range(history, HR_SLOTS, 0, &lo, &hi, &last);

    // the day's low on the top left and high on the top right, small, to frame the trend
    if (valid >= 1)
    {
        char lobuf[6];
        char hibuf[6];
        snprintf(lobuf, sizeof(lobuf), "%d", lo);
        snprintf(hibuf, sizeof(hibuf), "%d", hi);

        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        GRect row = GRect(gctx->body.origin.x + PANEL_PAD, gctx->body.origin.y,
                          gctx->body.size.w - PANEL_PAD * 2, 14);
        graphics_draw_text(gctx->ctx, lobuf, fonts_get(FONT_STM_12), row,
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);
        graphics_draw_text(gctx->ctx, hibuf, fonts_get(FONT_STM_12), row,
                           GTextOverflowModeFill, GTextAlignmentRight, NULL);
    }

    // the graph fills the lower part of the body, leaving the low/high row up top. one reading
    // is enough: hr_draw_spark dots a lone point and joins a line once there are more
    if (valid >= 1)
    {
        GRect graph = gctx->body;
        graph.origin.x += PANEL_PAD;
        graph.size.w -= PANEL_PAD * 2;
        graph.origin.y += 26;
        graph.size.h -= 28;

        // the scale takes a gutter off the left, so the plot slides right and narrows
        graph.origin.x += HR_SCALE_W;
        graph.size.w -= HR_SCALE_W;

        // ceiling holds at HR_CEIL, but expands to the day-high so a hard peak never clips
        int top = hi > HR_CEIL ? hi : HR_CEIL;

        // two ticks down the gutter: ceiling at the top, floor at the bottom. edge-anchored so
        // the short plot band does not push them off the cell. the floor sits a full line height
        // up from the plot bottom so its glyph clears the panel edge instead of clipping
        int gutter_x = gctx->body.origin.x + PANEL_PAD;
        int gutter_w = HR_SCALE_W - 4;
        int line_h = metric_for(FONT_STM_12)->line_h;
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        hr_scale_label(gctx, gutter_x, gutter_w, graph.origin.y - 1, top);
        hr_scale_label(gctx, gutter_x, gutter_w,
                       graph.origin.y + graph.size.h - line_h - 2, HR_FLOOR);

        hr_draw_spark(gctx, graph, history, HR_FLOOR, top);
    }

    // the current bpm big, centred across the top between the low/high labels. prefer the live
    // reading, but fall back to the most recent one in the history so the number stays in step
    // with the low/high rather than blanking to a dash when the live value is momentarily gone
    int live = health_store_hr();
    int shown = live > 0 ? live : (valid >= 1 ? last : -1);
    char val[8];
    fmt_int_or_dash(val, sizeof(val), shown, "%d");
    gh_graph_value_top(gctx, val);
}

const ModuleDef mod_health_hr_graph_def = {
    .label = "HR GRAPH",
    .sizes = SZ_2x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hr_graph_body
};
