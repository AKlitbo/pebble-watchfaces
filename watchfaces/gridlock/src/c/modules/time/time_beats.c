#include "engine/grid_engine.h"
#include "time_beats.h"
#include "draw/grid_helpers.h"
#include "draw/metrics.h"
#include "math/pct.h"
#include "draw/common.h"
#include "system/units/units.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include <stdio.h>

// beats run 0 to 999 across the day so we treat 1000 as a full day for the progress bar
#define BEATS_PER_DAY 1000

static void time_beats_1x2(GridCtx *gctx)
{
    char val[16];
    snprintf(val, sizeof(val), "@%03d", units_swatch_beats());

    int value_h = 30;
    int iw = icon_size(ICON_GLOBE.res).w;
    int text_w = gctx->body.size.w - (iw ? iw + PANEL_PAD * 2 : PANEL_PAD * 2);

    GRect base = grid_anchor(gctx, GSize(text_w, value_h), GAlignLeft, PANEL_PAD, 0);
    base = metric_baseline(FONT_TEKO_26, base);

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);

    // the '@' sits 1px higher than the numbers
    GSize at_sz = graphics_text_layout_get_content_size("@", fonts_get(FONT_TEKO_26),
                      GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);
    GRect at_rect = base;
    at_rect.origin.y -= 1;
    graphics_draw_text(gctx->ctx, "@", fonts_get(FONT_TEKO_26), at_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    GRect num_rect = base;
    num_rect.origin.x += at_sz.w;
    num_rect.size.w -= at_sz.w;
    graphics_draw_text(gctx->ctx, val + 1, fonts_get(FONT_TEKO_26), num_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    gh_icon_bottom_of(gctx, &ICON_GLOBE, base, FONT_TEKO_26);
}

// draws the big beats value centred up top with the '@' lifted a pixel like the 1x2 does.
// this is gh_value_top's placement but split in two so the '@' can ride a touch higher
static void beats_value_centered(GridCtx *gctx, const char *val)
{
    const int value_h = 38;
    GRect box = grid_anchor(gctx, GSize(gctx->body.size.w, value_h), GAlignTop, 0, -7);

    GSize full = graphics_text_layout_get_content_size(val, fonts_get(FONT_TEKO_34),
                     GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);
    GSize at_sz = graphics_text_layout_get_content_size("@", fonts_get(FONT_TEKO_34),
                     GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);

    int left = box.origin.x + (box.size.w - full.w) / 2;

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);

    GRect at_rect = GRect(left, box.origin.y - 2, at_sz.w + 2, value_h);
    graphics_draw_text(gctx->ctx, "@", fonts_get(FONT_TEKO_34), at_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    GRect num_rect = GRect(left + at_sz.w, box.origin.y, full.w - at_sz.w + 4, value_h);
    graphics_draw_text(gctx->ctx, val + 1, fonts_get(FONT_TEKO_34), num_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

// the 2x2 reads like a goal: big beats value on top then "OF 1000" and the real clock time.
// same pieces as gh_stat_goal_2x2 but the value is hand drawn so the '@' keeps its lift
static void time_beats_2x2(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    char clock[16];
    bool h12 = gridlock_format_clock(clock, sizeof(clock), t->tm_hour, t->tm_min);

    // second caption is the wall clock time in brackets so beats sits next to normal time
    char caption[24];
    if (h12)
    {
        snprintf(caption, sizeof(caption), "(%s%s)", clock, t->tm_hour < 12 ? "AM" : "PM");
    }
    else
    {
        snprintf(caption, sizeof(caption), "(%s)", clock);
    }

    char val[16];
    snprintf(val, sizeof(val), "@%03d", units_swatch_beats());

    beats_value_centered(gctx, val);
    gh_caption(gctx, "OF 1000", 27);
    gh_caption(gctx, caption, 39);
    icon_draw(gctx, &ICON_GLOBE, GAlignBottomRight, -EDGE_PAD(gctx), -5);
    draw_progress_bar(gctx, pct_of(units_swatch_beats(), BEATS_PER_DAY), icon_size(ICON_GLOBE.res).w);
}

// TODO: re-tune the 1x2 for the taller body when gctx->headerless
static void time_beats_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x2)
    {
        time_beats_1x2(gctx);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        time_beats_2x2(gctx);
    }
}

const ModuleDef mod_time_beats_def = {
    .label = "BEATS",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_TIME,
    .body = time_beats_body
};
