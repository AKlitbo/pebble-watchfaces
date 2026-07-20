#include "engine/grid_engine.h"
#include "health_steps_graph.h"
#include "draw/grid_helpers.h"
#include "draw/metrics.h"
#include "io/stores/health_store.h"
#include "math/scale.h"
#include "math/series.h"
#include "settings_schema.h"
#include "text/number_format.h"
#include <stdio.h>

// draws one filled bar per hour into area, sharing the full width so the strip gets denser as
// the day fills up. a quiet hour still gets a 1px stub so the baseline reads across
static void steps_draw_bars(GridCtx *gctx, GRect area, const uint16_t *hourly, int hours, int hi)
{
    if (hours <= 0)
    {
        return;
    }

    int denom = hi > 0 ? hi : 1;
    int gap = 1;
    int bar_w = segment_width(area.size.w, gap, hours);
    if (bar_w < 1)
    {
        // too many hours to space out, so pack the bars edge to edge
        bar_w = 1;
        gap = 0;
    }

    graphics_context_set_fill_color(gctx->ctx, gctx->color_value);
    for (int i = 0; i < hours; i++)
    {
        int h = fraction_px(area.size.h - 1, hourly[i], denom);
        if (h < 1)
        {
            h = 1;
        }
        int x = area.origin.x + i * (bar_w + gap);
        int y = area.origin.y + area.size.h - h;
        graphics_fill_rect(gctx->ctx, GRect(x, y, bar_w, h), 0, GCornerNone);
    }
}

// TODO: re-tune for the taller body when gctx->headerless
static void health_steps_graph_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    const uint16_t *hourly = health_store_step_hourly();
    int hours = health_store_step_hours();

    // the bars fill the lower part of the body, leaving the total and goal line up top
    if (hours >= 1)
    {
        int hi = series_max_u16(hourly, hours);
        GRect graph = gctx->body;
        graph.origin.x += PANEL_PAD;
        graph.size.w -= PANEL_PAD * 2;
        graph.origin.y += 34;
        graph.size.h -= 36;
        steps_draw_bars(gctx, graph, hourly, hours, hi);
    }

    // today's total big, centred across the top
    int total = health_store_steps();
    char val[12];
    if (total >= 0)
    {
        number_group(val, sizeof(val), total);
    }
    else
    {
        snprintf(val, sizeof(val), "--");
    }
    gh_graph_value_top(gctx, val);

    // the step goal under the total, small, so the bars read as progress toward it
    char goalbuf[16];
    char goalline[24];
    number_group(goalbuf, sizeof(goalbuf), gridlock_goal_steps());
    snprintf(goalline, sizeof(goalline), "OF %s", goalbuf);
    gh_caption(gctx, goalline, 20);
}

const ModuleDef mod_health_steps_graph_def = {
    .label = "STEPS GRAPH",
    .sizes = SZ_2x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_STEPS,
    .body = health_steps_graph_body
};
