#include "engine/grid_engine.h"
#include "time_progress.h"
#include "time_progress_calc.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/icons.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/time_store.h"
#include "ui/fonts.h"
#include "clock/date.h"
#include "settings_schema.h"
#include <stdio.h>

#define BAR_GAP 8   // room between a bar and whatever sits beside it on the 1x4
#define BAR_H_WIDE 6 // the 1x4 bar, a touch thicker than the 4px one the taller tiles carry
#define BAR_H_SMALL 3   // the bar under the 1x2 value
#define BAR_H_TALL 4    // the bar along the bottom of the 2x2
#define BAR_H_TALL_WIDE 6 // the same on the 2x4, which has more body to spare
#define BAR_ICON_GAP 6  // between a bottom bar and the corner icon, as in draw_progress_bar
#define BAR_GROW_HEADERLESS 3 // extra bar thickness when the header strip is gone
#define BAR_GROW_BORDERLESS 1 // and when the border is gone
#define VALUE_H_SMALL 22 // the 1x2 value box, one Teko 22 line
#define VALUE_DY_HEADED -5     // nudges that seat the 1x2 value and icon above the bar,
#define VALUE_DY_HEADERLESS 2  // found in the emulator for the headed and headerless bodies
#define ICON_DY_HEADED 1
#define ICON_DY_HEADERLESS 4
#define LINE_H 16   // one caption line on the 2x4

// a progress bar in the accent colour: an outline with the done part filled inside it
static void progress_bar(GridCtx *gctx, GRect bar, int pct)
{
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    graphics_draw_rect(gctx->ctx, bar);
    int fill_w = (bar.size.w - 2) * pct / 100;
    if (fill_w > 0)
    {
        graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
        graphics_fill_rect(gctx->ctx, GRect(bar.origin.x + 1, bar.origin.y + 1, fill_w,
                                            bar.size.h - 2), 0, GCornerNone);
    }
}

// how much thicker a bar gets when the tile has room to spare. a tile with no header owns the
// taller body, and one with no border gives a pixel back, so both let the bar be read at a glance
static int bar_grow(const GridCtx *gctx)
{
    return (gctx->headerless ? BAR_GROW_HEADERLESS : 0) + (gctx->borderless ? BAR_GROW_BORDERLESS : 0);
}

// the bar the two taller tiles carry along their bottom edge, left of the icon. the geometry is
// the shared draw_progress_bar's, at a height of our own, so it can grow with the room
static void progress_bar_bottom(GridCtx *gctx, int pct, int icon_w, int h)
{
    int w = gctx->body.size.w - PANEL_PAD * 2 - icon_w - BAR_ICON_GAP;
    if (w <= 0)
    {
        return;
    }
    GRect bar = grid_anchor(gctx, GSize(w, h), GAlignBottomLeft, PANEL_PAD + 1, -5);
    progress_bar(gctx, bar, pct);
}

// the small tile: the percent and icon on top and a bar under them across the whole tile. a
// headed body is only about 25px tall, so the value steps down to Teko 22 to leave the bar room.
// with no header the tile is taller, so the value keeps Teko 26 and the bar gets thicker, and the
// value and icon are centred in the space the bar leaves rather than left to the stock 1x2
static void progress_draw_1x2(GridCtx *gctx, int pct, const IconSpec *icon)
{
    char val[8];
    snprintf(val, sizeof(val), "%d%%", pct);

    FontId font = gctx->headerless ? FONT_TEKO_26 : FONT_TEKO_22;
    int value_dy = gctx->headerless ? VALUE_DY_HEADERLESS : VALUE_DY_HEADED;
    int icon_dy = gctx->headerless ? ICON_DY_HEADERLESS : ICON_DY_HEADED;

    int iw = icon_size(icon->res).w;
    GRect box = grid_anchor(gctx, GSize(gctx->body.size.w - iw - PANEL_PAD * 2, VALUE_H_SMALL + 4),
                            GAlignTopLeft, EDGE_PAD(gctx), value_dy);
    box = metric_baseline(font, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(font), box, GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);
    icon_draw(gctx, icon, GAlignTopRight, -EDGE_PAD(gctx), icon_dy);

    GRect bar = grid_anchor(gctx, GSize(gctx->body.size.w - EDGE_PAD(gctx) * 2,
                                        BAR_H_SMALL + bar_grow(gctx)), GAlignBottom, 0, -2);
    progress_bar(gctx, bar, pct);
}

// the wide small tile: the percent on the left, the bar across the middle and the icon on the
// right, so a full width row reads the same at a glance as the half width one
static void progress_draw_1x4(GridCtx *gctx, int pct, const IconSpec *icon)
{
    char val[8];
    snprintf(val, sizeof(val), "%d%%", pct);

    int value_h = 30;
    int iw = icon_size(icon->res).w;
    GSize text = graphics_text_layout_get_content_size(val, fonts_get(FONT_TEKO_26),
                     GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);

    GRect base = grid_anchor(gctx, GSize(text.w + 2, value_h), GAlignLeft, EDGE_PAD(gctx), 0);
    base = metric_baseline(FONT_TEKO_26, base);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(FONT_TEKO_26), base, GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    icon_draw(gctx, icon, GAlignRight, -EDGE_PAD(gctx), 0);

    // the bar fills whatever the value and the icon leave, centred on the row
    int left = gctx->body.origin.x + EDGE_PAD(gctx) + text.w + BAR_GAP;
    int right = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx) - iw - BAR_GAP;
    if (right - left < 8)
    {
        return;
    }
    int bar_h = BAR_H_WIDE + bar_grow(gctx);
    GRect bar = GRect(left, gctx->body.origin.y + (gctx->body.size.h - bar_h) / 2,
                      right - left, bar_h);
    progress_bar(gctx, bar, pct);
}

// the wide tall tile: the big percent on the left, the two captions stacked on the right, and
// the bar and icon along the bottom the way the 2x2 has them
static void progress_draw_2x4(GridCtx *gctx, int pct, const char *line1, const char *line2,
                              const IconSpec *icon)
{
    char val[8];
    snprintf(val, sizeof(val), "%d%%", pct);

    GRect value_box = grid_anchor(gctx, GSize(gctx->body.size.w / 2, 38), GAlignTopLeft, EDGE_PAD(gctx), -7);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(FONT_TEKO_34), value_box, GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    // the captions sit on the right, in Gothic like every other caption since the mono font
    // makes an O look like a D this small
    int right = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx);
    GRect line_a = GRect(right - 90, gctx->body.origin.y + 2, 90, LINE_H);
    GRect line_b = GRect(right - 90, gctx->body.origin.y + 2 + LINE_H, 90, LINE_H);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, line1, fonts_get_system_font(FONT_KEY_GOTHIC_14), line_a, GTextOverflowModeFill,
                       GTextAlignmentRight, NULL);
    graphics_draw_text(gctx->ctx, line2, fonts_get_system_font(FONT_KEY_GOTHIC_14), line_b, GTextOverflowModeFill,
                       GTextAlignmentRight, NULL);

    icon_draw(gctx, icon, GAlignBottomRight, -EDGE_PAD(gctx), -5);
    progress_bar_bottom(gctx, pct, icon_size(icon->res).w, BAR_H_TALL_WIDE + bar_grow(gctx));
}

// the small tile is the percent with a little % after it, the big tile the percent over a bar
// with two captions saying how far through the period that is in its own units. the two full
// width tiles follow the same idea at their own proportions
static void progress_draw(GridCtx *gctx, int pct, const char *line1, const char *line2,
                          const IconSpec *icon)
{
    char val[8];

    switch (gctx->size)
    {
    case MSIZE_1x2:
        progress_draw_1x2(gctx, pct, icon);
        break;
    case MSIZE_2x2:
        snprintf(val, sizeof(val), "%d%%", pct);
        gh_stat_2x2(gctx, val, line1, line2, icon, -1);
        progress_bar_bottom(gctx, pct, icon_size(icon->res).w, BAR_H_TALL + bar_grow(gctx));
        break;
    case MSIZE_1x4:
        progress_draw_1x4(gctx, pct, icon);
        break;
    case MSIZE_2x4:
        progress_draw_2x4(gctx, pct, line1, line2, icon);
        break;
    default:
        break;
    }
}

static void year_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int year = t->tm_year + 1900;
    int pct = progress_year_pct(year, t->tm_yday, t->tm_hour, t->tm_min, t->tm_sec);

    // days gone plus days left is the year's full length
    int day = date_day_of_year(t->tm_yday);
    int total = day + date_days_left_in_year(year, t->tm_yday);

    char line2[24];
    snprintf(line2, sizeof(line2), "DAY %d/%d", day, total);
    progress_draw(gctx, pct, "OF THE YEAR", line2, &ICON_DATE_TIME);
}

static void day_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int pct = progress_day_pct(t->tm_hour, t->tm_min, t->tm_sec);

    // the hour we are inside, so 00:30 is hour 1 of 24
    char line2[24];
    snprintf(line2, sizeof(line2), "HOUR %d/24", t->tm_hour + 1);
    progress_draw(gctx, pct, "OF THE DAY", line2, &ICON_CLOCK);
}

static void hour_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int pct = progress_hour_pct(t->tm_min, t->tm_sec);

    char line2[24];
    snprintf(line2, sizeof(line2), "%d/60 MIN", t->tm_min);
    progress_draw(gctx, pct, "OF THE HOUR", line2, &ICON_CLOCK);
}

static void week_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int first = gridlock_week_start();
    int pct = progress_week_pct(t->tm_wday, first, t->tm_hour, t->tm_min, t->tm_sec);

    char line2[24];
    snprintf(line2, sizeof(line2), "DAY %d/7", progress_week_day(t->tm_wday, first));
    progress_draw(gctx, pct, "OF THE WEEK", line2, &ICON_DATE_TIME);
}

const ModuleDef mod_time_year_progress_def = {
    .label = "THIS YEAR",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4 | SZ_2x4,
    .features = FEATURE_TIME,
    .body = year_progress_body,
};

// the other three borrow the year's colour so the set reads as one family
const ModuleDef mod_time_day_progress_def = {
    .label = "TODAY",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4 | SZ_2x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_YEAR_PROGRESS,
    .body = day_progress_body,
};

const ModuleDef mod_time_hour_progress_def = {
    .label = "THIS HOUR",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4 | SZ_2x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_YEAR_PROGRESS,
    .body = hour_progress_body,
};

const ModuleDef mod_time_week_progress_def = {
    .label = "THIS WEEK",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4 | SZ_2x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_YEAR_PROGRESS,
    .body = week_progress_body,
};
