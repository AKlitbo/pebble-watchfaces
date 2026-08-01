/**
 * @file weather_now.c
 * @brief The "Weather Now" 1x4 bar. One line across the full width: the condition icon and
 * current temperature on the left, today's high and low on the right, and a short condition
 * word in the middle when there is room for it.
 * @ingroup gridlock_mod_weather
 */
#include "engine/grid_engine.h"
#include "weather_now.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include "draw/metrics.h"
#include "draw/wx_icon.h"
#include "weather_forecast_common.h"
#include "io/stores/weather_store.h"
#include "ui/icon_cache.h"
#include "ui/weather/icons.h"
#include "ui/weather/labels.h"
#include <stdio.h>
#include <string.h>

#define TEMP_FONT   FONT_TEKO_26  // the current temperature number, the hero on the left
#define UNIT_FONT   FONT_STM_12   // the small "°C"/"°F" trailing the number, like a BPM unit
#define HILO_FONT   FONT_TEKO_22  // the smaller high/low pair on the right
#define STATUS_FONT FONT_STM_12   // the short condition code in the small caption font

#define GROUP_GAP     6  // between the temp, hi/lo, and icon groups
#define STATUS_GAP    2  // the condition icon to its status code (they read as a pair)
#define HILO_GAP      3  // an arrow to its own number
#define HILO_SEP      7  // gap between the high group and the low group
#define BOTTOM_MARGIN 5  // how far the shared baseline sits above the panel's bottom edge

static int text_width(const char *text, FontId font)
{
    return graphics_text_layout_get_content_size(text, fonts_get(font),
               GRect(0, 0, 1000, 30), GTextOverflowModeFill, GTextAlignmentLeft).w;
}

// draws left-aligned text at a body-relative x with its cap bottom on the shared baseline, so
// every element in the row rests on the same line just above the panel's bottom edge
static void draw_text_at(GridCtx *gctx, int rel_x, int baseline, const char *text, FontId font, GColor color)
{
    const FontMetric *m = metric_for(font);
    int y = baseline - (m->top_pad + m->cap_h);
    graphics_context_set_text_color(gctx->ctx, color);
    graphics_draw_text(gctx->ctx, text, fonts_get(font),
                       GRect(gctx->body.origin.x + rel_x, y, text_width(text, font) + 8, m->line_h + 4),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

// draws the temperature as a big number with a small "°C"/"°F" suffix bottom-aligned to it
// (the trailing-unit look, like the BPM stat panels), vertically centred. returns the
// body-relative x just past the unit so the icon can follow
// draws the temperature number on the shared baseline with the small "°C"/"°F" bottom-aligned
// to it (3px lift, matching gh_stat_1x2). returns the body-relative x just past the unit
static int draw_temp(GridCtx *gctx, int rel_x, int baseline, const char *num, const char *unit)
{
    int num_w = text_width(num, TEMP_FONT);
    draw_text_at(gctx, rel_x, baseline, num, TEMP_FONT, gctx->color_value);

    int end = rel_x + num_w;
    if (unit[0])
    {
        const FontMetric *tm = metric_for(UNIT_FONT);
        int unit_y = baseline - (tm->top_pad + tm->cap_h) - 3;
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, unit, fonts_get(UNIT_FONT),
                           GRect(gctx->body.origin.x + rel_x + num_w + 1, unit_y, 40, 14),
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);
        end += 1 + text_width(unit, UNIT_FONT);
    }
    return end;
}

// draws the high/low pair (arrows + numbers) starting at a body-relative x, returning the x
// just past the low number
static int draw_hilo(GridCtx *gctx, int rel_x, int baseline)
{
    char hi[8], lo[8];
    weather_fmt_temp(hi, sizeof(hi), weather_store_temp_max(), "%d");
    weather_fmt_temp(lo, sizeof(lo), weather_store_temp_min(), "%d");

    int up_w = icon_visible_width(RESOURCE_ID_ICON_WEATHER_HILO_UP);
    int dn_w = icon_visible_width(RESOURCE_ID_ICON_WEATHER_HILO_DOWN);
    int hi_w = text_width(hi, HILO_FONT);

    // bottom-align the arrows onto the shared baseline too. GAlignBottomLeft anchors the icon
    // box to the body bottom, nudged up by the baseline's margin from that edge
    int arrow_dy = baseline - (gctx->body.origin.y + gctx->body.size.h);
    icon_draw_res(gctx, RESOURCE_ID_ICON_WEATHER_HILO_UP, GAlignBottomLeft, rel_x, arrow_dy);
    draw_text_at(gctx, rel_x + up_w + HILO_GAP, baseline, hi, HILO_FONT, gctx->color_value);
    int lo_start = rel_x + up_w + HILO_GAP + hi_w + HILO_SEP;
    icon_draw_res(gctx, RESOURCE_ID_ICON_WEATHER_HILO_DOWN, GAlignBottomLeft, lo_start, arrow_dy);
    draw_text_at(gctx, lo_start + dn_w + HILO_GAP, baseline, lo, HILO_FONT, gctx->color_value);

    return lo_start + dn_w + HILO_GAP + text_width(lo, HILO_FONT);
}

// draws the short condition code beside the icon in the small caption font, only when it still
// fits the remaining width
static void draw_status(GridCtx *gctx, int rel_x, int baseline, int avail, const char *cond)
{
    const char *label = wx_label_short(cond);
    if (text_width(label, STATUS_FONT) <= avail)
    {
        draw_text_at(gctx, rel_x, baseline, label, STATUS_FONT, gctx->color_subtitle);
    }
}

static void weather_now_draw(GridCtx *gctx)
{
    GRect body = gctx->body;
    int width = body.size.w;
    int baseline = body.origin.y + body.size.h - BOTTOM_MARGIN; // shared bottom line for the row

    // laid out left to right: temperature, high/low, condition icon, then the short status word
    char num[12];
    const char *unit = NULL;
    weather_temp_value(weather_store_temp(), num, sizeof(num), &unit);

    int cursor = PANEL_PAD; // body-relative x, matching the 1x2 stat panels' left inset
    cursor = draw_temp(gctx, cursor, baseline, num, unit ? unit : "") + GROUP_GAP;

    cursor = draw_hilo(gctx, cursor, baseline) + GROUP_GAP;

    IconMargins margin;
    GBitmap *bmp = wx_icon_get(&margin);
    if (bmp)
    {
        // pin the visible art (not the padded bitmap box), its bottom on the shared baseline
        GSize gs = gbitmap_get_bounds(bmp).size;
        int draw_x = body.origin.x + cursor - margin.w;
        int draw_y = baseline - gs.h + margin.s;
        blit_tinted(gctx, bmp, GRect(draw_x, draw_y, gs.w, gs.h));
        cursor += (gs.w - margin.w - margin.e) + STATUS_GAP;
    }

    // the condition name rides last, beside the icon
    const char *raw = weather_store_cond();
    if (raw[0] && strcmp(raw, "--"))
    {
        draw_status(gctx, cursor, baseline, width - EDGE_PAD(gctx) - cursor, raw);
    }
}

static void weather_now_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x4)
    {
        weather_now_draw(gctx);
    }
}

void weather_now_cleanup(void)
{
    wx_icon_cleanup();
}

const ModuleDef mod_weather_now_def = {
    .label = "WEATHER NOW",
    .sizes = SZ_1x4,
    .features = FEATURE_WEATHER,
    .body = weather_now_body,
    .cleanup = weather_now_cleanup
};
