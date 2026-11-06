#include "engine/grid_engine.h"
#include "health_hr_graph.h"
#include "mosaic/draw/grid_helpers.h"
#include "math/scale.h"
#include "math/series.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/health_store.h"
#include "health/hr_day.h"
#include <stdio.h>
#include <time.h>

#define HR_SLOTS 60

// the plot is pinned to a fixed window so the trace's height reads as absolute bpm rather than
// just the last hour's own range. the floor is the baseline. the ceiling holds unless the
// hour's high pushes past it. both are tunable
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

    // the last hour's low on the top left and high on the top right, small, to frame the trend
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

        // ceiling holds at HR_CEIL, but grows to the hour's high so a hard peak never clips
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

// --- BPM today (1x4, 2x4) ---

/**
 * Draws today's hours as bars, midnight on the left. The bars stand on the same fixed floor as the
 * HR graph, so a bar's height reads as the beats themselves and two days compare by eye. The
 * ceiling rises past HR_CEIL only when an hour's average does. The hour now is drawn in the icon
 * colour so it stands out while it fills.
 *
 * @param gctx The grid context.
 * @param area Where the bars go.
 * @param day Today's heart rate by the hour.
 */
static void hr_draw_day(GridCtx *gctx, GRect area, const HrDay *day)
{
    time_t now = time(NULL);
    int hour_now = localtime(&now)->tm_hour;
    int ceiling = HR_CEIL;

    for (int hour = 0; hour < HR_DAY_HOURS; hour++)
    {
        int avg = hr_day_hour_avg(day, hour);
        if (avg > ceiling)
        {
            ceiling = avg;
        }
    }

    int col_w = area.size.w / HR_DAY_HOURS;
    int base_y = area.origin.y + area.size.h;

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_line(gctx->ctx, GPoint(area.origin.x, base_y), GPoint(area.origin.x + area.size.w - 1, base_y));

    for (int hour = 0; hour < HR_DAY_HOURS; hour++)
    {
        int avg = hr_day_hour_avg(day, hour);
        if (avg < 0)
        {
            continue;
        }

        // an hour with a reading always shows at least a sliver, even one at or under the floor
        int top = plot_y(area.origin.y, area.size.h, HR_FLOOR, ceiling, clamp_int(avg, HR_FLOOR, ceiling));
        int h = base_y - top;
        if (h < 2)
        {
            h = 2;
        }
        GRect bar = GRect(area.origin.x + hour * col_w, base_y - h, col_w > 1 ? col_w - 1 : 1, h);
        graphics_context_set_fill_color(gctx->ctx, hour == hour_now ? gctx->color_icon : gctx->color_value);
        graphics_fill_rect(gctx->ctx, bar, 0, GCornerNone);
    }
}

/**
 * Today's heart rate. The 1x4 is the bars alone. The 2x4 puts the day's low, average, and high and
 * the live reading across the top, laid out the same as HRV Today so the two read as a pair.
 */
static void health_hr_day_body(GridCtx *gctx)
{
    const HrDay *day = health_store_hr_day();
    GRect area = gctx->body;
    area.origin.x += PANEL_PAD;
    area.size.w -= PANEL_PAD * 2;

    if (gctx->size == MSIZE_1x4)
    {
        area.origin.y += 3;
        area.size.h -= 6;
        hr_draw_day(gctx, area, day);
        return;
    }

    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    int low = -1;
    int avg = -1;
    int high = -1;
    hr_day_stats(day, &low, &avg, &high);
    static const char *const labels[4] = { "LOW", "AVG", "HIGH", "NOW" };
    const int values[4] = { low, avg, high, health_store_hr() };
    int stats_h = gh_stat_row4(gctx, area, labels, values);
    area.origin.y += stats_h;
    area.size.h -= stats_h + 3;
    hr_draw_day(gctx, area, day);
}

const ModuleDef mod_health_hr_day_def = {
    .label = "BPM TODAY",
    .sizes = SZ_1x4 | SZ_2x4,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hr_day_body
};
