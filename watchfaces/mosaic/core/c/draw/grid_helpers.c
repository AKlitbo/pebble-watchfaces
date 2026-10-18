/**
 * @file grid_helpers.c
 * @brief The code for the shared panel building blocks. The offsets in here hold the
 * exact pixel placement the stat panels draw with. See grid_helpers.h.
 * @ingroup mosaic_draw
 */
#include "mosaic/draw/grid_helpers.h"

#include "math/pct.h"
#include "math/scale.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/metrics.h"
#include "mosaic/draw/value_bearing.g.h"
#include "settings_schema.h"
#include <stdio.h>
#include <string.h>

void gh_format_hhmm(char *out, size_t n, int minutes)
{
    if (minutes < 0)
    {
        snprintf(out, n, "--");
        return;
    }

    gridlock_format_clock(out, n, minutes / 60, minutes % 60);
}

GRect gh_value_left(GridCtx *gctx, const char *text, FontId font, int box_w, int value_h, int pad)
{
    // shift by the first glyph's own left-bearing so the visible ink lands at the same
    // inset no matter which character leads (a "1" would otherwise sit tighter than an "8")
    pad += value_bearing_fix(text[0], font);

    // pin it left and centre it top to bottom
    // then take off the font's invisible top padding
    GRect r = grid_anchor(gctx, GSize(box_w, value_h), GAlignLeft, pad, 0);
    r = metric_baseline(font, r);

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), r, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    return r;
}

GRect gh_value_top(GridCtx *gctx, const char *text, FontId font, int value_h, int dx, int dy)
{
    GRect r = grid_anchor(gctx, GSize(gctx->body.size.w, value_h), GAlignTop, dx, dy);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), r, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    return r;
}

void gh_caption(GridCtx *gctx, const char *text, int y)
{
    // Gothic for captions. Share Tech Mono's square O looks like a D this small.
    // the box is inset a PANEL_PAD each side and the text ellipsizes, so a long caption
    // (an event title) clips inside the panel instead of spilling out to the border
    GRect r = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD * 2, 14), GAlignTop, 0, y);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14), r,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void gh_graph_value_top(GridCtx *gctx, const char *text)
{
    // the big value crowns the plot, sat just above the body so it clears the low/high labels
    const FontMetric *vm = metric_for(FONT_TEKO_26);
    int y = gctx->body.origin.y - vm->top_pad - 2;
    GRect r = GRect(gctx->body.origin.x, y, gctx->body.size.w, vm->cap_h + vm->top_pad + 8);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, text, fonts_get(FONT_TEKO_26), r,
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

void gh_gauge(GridCtx *gctx, GRect area, int segments, int level)
{
    if (segments <= 0)
    {
        // nothing to draw and it keeps us off a divide by zero
        return;
    }

    const int gap = 2;
    int seg_w = segment_width(area.size.w, gap, segments);
    if (seg_w < 1)
    {
        return;
    }

    // an integer seg_w leaves a few px of remainder
    // centre the row of cells so the bar is not shifted left with dead space on the right
    int drawn = segments * seg_w + gap * (segments - 1);
    int x0 = area.origin.x + (area.size.w - drawn) / 2;

    int filled = segments_filled(level, segments);

    for (int i = 0; i < segments; i++)
    {
        GRect cell = GRect(x0 + i * (seg_w + gap), area.origin.y, seg_w, area.size.h);

        graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
        graphics_draw_rect(gctx->ctx, cell);

        if (i < filled)
        {
            graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
            graphics_fill_rect(gctx->ctx, cell, 0, GCornerNone);
        }
    }
}

void gh_icon_bottom_of(GridCtx *gctx, const IconSpec *icon, GRect value_rect, FontId value_font)
{
    GSize is = icon_size(icon->res);
    if (is.w == 0)
    {
        return;
    }

    // auto-trim pins the visible art, not the full bitmap: +e kills the right margin so the
    // art sits EDGE_PAD in from the right border, +s kills the bottom margin
    IconMargins trim = icon_margins(icon->res);
    int x = gctx->body.origin.x + gctx->body.size.w - is.w - EDGE_PAD(gctx) + trim.e + ICON_DX(icon);

    int y;
    if (gctx->size == MSIZE_1x2)
    {
        // 1x2 stat icons corner-pin: visible art sits a clean EDGE_PAD up from the bottom
        // border, matching the 2x2 corner icon instead of riding the value baseline
        y = gctx->body.origin.y + gctx->body.size.h - is.h - EDGE_PAD(gctx) + trim.s + ICON_DY(icon);
    }
    else
    {
        // the 2x2 mini-rows stack two icons on their own value baselines, so those stay
        // pinned to the value's visible bottom
        const FontMetric *m = metric_for(value_font);
        int text_bottom = value_rect.origin.y + m->top_pad + m->cap_h;
        y = text_bottom - is.h + trim.s + ICON_DY(icon);

        // tall weather icons poke out the top if we sit them on the value bottom
        // so middle the visible art top to bottom instead
        if (y + trim.n < gctx->body.origin.y)
        {
            int opaque_h = is.h - trim.n - trim.s;
            y = gctx->body.origin.y + (gctx->body.size.h - opaque_h) / 2 - trim.n + ICON_DY(icon);
        }
    }

    icon_draw_rect(gctx, icon->res, GRect(x, y, is.w, is.h));
}

void gh_stat_2x2(GridCtx *gctx, const char *val, const char *line1, const char *line2,
                 const IconSpec *icon, int progress_pct)
{
    // value up top and centred. it drops from the big font to the small one
    // if it would be too wide
    FontId v_font = FONT_TEKO_34;
    int    v_h = 38;
    int    v_dy = -7;
    GSize  ts = graphics_text_layout_get_content_size(val, fonts_get(v_font),
                    GRect(0, 0, 1000, v_h), GTextOverflowModeFill, GTextAlignmentCenter);
    if (ts.w > gctx->body.size.w - PANEL_PAD * 2)
    {
        v_font = FONT_TEKO_26;
        v_h = 30;
        v_dy = -2;
    }
    gh_value_top(gctx, val, v_font, v_h, 0, v_dy);

    gh_caption(gctx, line1, 27);
    gh_caption(gctx, line2, 39);

    // icon in the bottom-right corner. EDGE_PAD in plus the icon's own nudge via icon_draw
    icon_draw(gctx, icon, GAlignBottomRight, -EDGE_PAD(gctx), -5);
    draw_progress_bar(gctx, progress_pct, icon_size(icon->res).w);
}

void gh_value_top_time(GridCtx *gctx, const char *time_str, bool meridiem, bool is_am,
                       FontId font, int v_h, int v_dy)
{
    const char *am = is_am ? "AM" : "PM";

    GSize ts = graphics_text_layout_get_content_size(time_str, fonts_get(font),
                   GRect(0, 0, 1000, v_h), GTextOverflowModeFill, GTextAlignmentLeft);
    int am_w = meridiem ? graphics_text_layout_get_content_size(am, fonts_get(FONT_STM_12),
                              GRect(0, 0, 30, 14), GTextOverflowModeFill, GTextAlignmentLeft).w
                        : 0;

    GRect group = grid_anchor(gctx, GSize(ts.w + am_w, v_h), GAlignTop, 0, v_dy);

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, time_str, fonts_get(font), group,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    if (meridiem)
    {
        // sit the am/pm flush after the time, aligned to the big text's baseline (see the clock)
        const FontMetric *vm = metric_for(font);
        const FontMetric *tm = metric_for(FONT_STM_12);
        GRect am_rect = group;
        am_rect.origin.x += ts.w;
        am_rect.size.w = am_w;
        am_rect.size.h = 14;
        am_rect.origin.y = group.origin.y + vm->top_pad + vm->cap_h - (tm->top_pad + tm->cap_h) - 1;

        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, am, fonts_get(FONT_STM_12), am_rect,
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
}

void gh_stat_time_2x2(GridCtx *gctx, const char *time_str, bool meridiem, bool is_am,
                      const char *line1, const char *line2, const IconSpec *icon, int progress_pct)
{
    // the value is the time plus a small am/pm, centred as one group up top. it drops from the
    // big font to the small one if the pair would be too wide, matching gh_stat_2x2
    FontId v_font = FONT_TEKO_34;
    int    v_h = 38;
    int    v_dy = -7;
    const char *am = is_am ? "AM" : "PM";

    GSize ts = graphics_text_layout_get_content_size(time_str, fonts_get(v_font),
                   GRect(0, 0, 1000, v_h), GTextOverflowModeFill, GTextAlignmentLeft);
    int am_w = meridiem ? graphics_text_layout_get_content_size(am, fonts_get(FONT_STM_12),
                              GRect(0, 0, 30, 14), GTextOverflowModeFill, GTextAlignmentLeft).w
                        : 0;

    if (ts.w + am_w > gctx->body.size.w - PANEL_PAD * 2)
    {
        v_font = FONT_TEKO_26;
        v_h = 30;
        v_dy = -2;
    }

    gh_value_top_time(gctx, time_str, meridiem, is_am, v_font, v_h, v_dy);

    gh_caption(gctx, line1, 27);
    gh_caption(gctx, line2, 39);

    icon_draw(gctx, icon, GAlignBottomRight, -EDGE_PAD(gctx), -5);
    draw_progress_bar(gctx, progress_pct, icon_size(icon->res).w);
}

void gh_stat_goal_2x2(GridCtx *gctx, const char *val, const char *goal_str, const char *caption,
                      const IconSpec *icon, int value, int goal)
{
    char of_str[24];
    snprintf(of_str, sizeof(of_str), "OF %s", goal_str);
    gh_stat_2x2(gctx, val, of_str, caption, icon, pct_of(value, goal));
}

// the value font ladder walks up to four measure passes to find the largest rung that clears
// the icon. the choice only changes when the value string or the tile geometry changes, so a
// small cache keeps it off the repaint path. each entry pins the inputs that pick the rung
typedef struct
{
    char    val[24];
    int16_t avail;
    int16_t trail_w;
    uint8_t has_icon;
    uint8_t headerless;
    FontId  base;   // the caller's font, which is itself a rung, so it picks the answer too
    FontId  top;    // the ladder's first rung and its length. the plain 1x2 and the bar 1x2 climb
    uint8_t rungs;  // different ladders from the same base, so they must not share an entry
    FontId  font;   // the rung that won
    int16_t text_w; // measured width of val in the chosen font, needed to place the trailing unit
    bool    used;
} FontFitEntry;

#define FONT_FIT_CACHE 8
static FontFitEntry s_fit_cache[FONT_FIT_CACHE];
static uint8_t s_fit_next; // round robin slot to replace on a miss

/**
 * @brief Picks the value font off the rung ladder, caching the choice per value and geometry.
 *
 * @param val The value string.
 * @param rungs The font rungs, largest first.
 * @param rung_count How many rungs.
 * @param avail Width the value has to fit in.
 * @param trail_w Width of the trailing unit folded into the fit test.
 * @param has_icon Whether an icon shares the tile (only then does the ladder step down).
 * @param headerless Whether the taller no-header rung is in play (part of the cache key).
 * @param base The caller's own font. It sits on the ladder, so two tiles alike in every other way
 *   but this one get different answers and must not share an entry.
 * @param text_w_out Set to the measured width of val in the chosen font.
 * @return The chosen font.
 */
static FontId fit_value_font(const char *val, const FontId *rungs, int rung_count, int avail,
                             int trail_w, bool has_icon, bool headerless, FontId base,
                             int *text_w_out)
{
    for (int i = 0; i < FONT_FIT_CACHE; i++)
    {
        FontFitEntry *entry = &s_fit_cache[i];
        if (entry->used && entry->avail == avail && entry->trail_w == trail_w
            && entry->has_icon == (has_icon ? 1 : 0) && entry->headerless == (headerless ? 1 : 0)
            && entry->base == base && entry->top == rungs[0] && entry->rungs == rung_count
            && strcmp(entry->val, val) == 0)
        {
            *text_w_out = entry->text_w;
            return entry->font;
        }
    }

    FontId draw_font = rungs[0];
    GSize  ts = graphics_text_layout_get_content_size(val, fonts_get(draw_font),
                    GRect(0, 0, 1000, 38), GTextOverflowModeFill, GTextAlignmentLeft);
    for (int i = 1; has_icon && ts.w + trail_w > avail && i < rung_count; i++)
    {
        draw_font = rungs[i];
        ts = graphics_text_layout_get_content_size(val, fonts_get(draw_font),
                 GRect(0, 0, 1000, 38), GTextOverflowModeFill, GTextAlignmentLeft);
    }

    FontFitEntry *slot = &s_fit_cache[s_fit_next];
    s_fit_next = (s_fit_next + 1) % FONT_FIT_CACHE;
    strncpy(slot->val, val, sizeof(slot->val) - 1);
    slot->val[sizeof(slot->val) - 1] = '\0';
    slot->avail = (int16_t)avail;
    slot->trail_w = (int16_t)trail_w;
    slot->has_icon = has_icon ? 1 : 0;
    slot->headerless = headerless ? 1 : 0;
    slot->base = base;
    slot->top = rungs[0];
    slot->rungs = (uint8_t)rung_count;
    slot->font = draw_font;
    slot->text_w = (int16_t)ts.w;
    slot->used = true;

    *text_w_out = ts.w;
    return draw_font;
}

// the bar under a 1x2 value, and the gap it leaves before the corner icon
#define BAR_1X2_H 3
#define BAR_ICON_GAP 6

// the 1x4 bar, a touch thicker than the 4px one the taller tiles carry, and the room it leaves
// between itself and the value and icon either side
#define BAR_1X4_H 6
#define BAR_1X4_GAP 8

// the box a 1x2 bar tile's value sits in, one Teko 22 line plus its pad
#define BAR_VALUE_H 26

// nudges that seat the value above the bar, found in the emulator for the headed and headerless
// bodies
#define BAR_VALUE_DY_HEADED -5
#define BAR_VALUE_DY_HEADERLESS 2

// the gap between a value and the small unit after it
#define TRAIL_GAP 1

// how wide a trailing unit is with the gap before it, or 0 for none
static int trailing_width(const char *trailing)
{
    if (!trailing || !trailing[0])
    {
        return 0;
    }

    return TRAIL_GAP + graphics_text_layout_get_content_size(trailing, fonts_get(FONT_STM_12),
                           GRect(0, 0, 1000, 14), GTextOverflowModeFill, GTextAlignmentLeft).w;
}

// draws a trailing unit (FONT_STM_12, Share Tech Mono 12) just past the value, bottom-aligned
// to the value's cap bottom so it stays put across value font steps. -3 trims the unit's extra
// descender room so it sits flush on the value baseline. it draws in the width it measured, so
// a long unit (UV 11 reads EXTREME) is never clipped by a fixed box
static void draw_trailing(GridCtx *gctx, const char *trailing, int trail_w, GRect value_box,
                          FontId value_font, int text_w)
{
    const FontMetric *vm = metric_for(value_font);
    const FontMetric *tm = metric_for(FONT_STM_12);
    int trail_y = value_box.origin.y + vm->top_pad + vm->cap_h - (tm->top_pad + tm->cap_h) - 3;
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, trailing, fonts_get(FONT_STM_12),
                       GRect(value_box.origin.x + text_w + TRAIL_GAP, trail_y, trail_w - TRAIL_GAP, 14),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

void gh_progress_bar(GridCtx *gctx, GRect bar, int pct)
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

void gh_bar_1x2(GridCtx *gctx, const char *val, const char *trailing, const IconSpec *icon, int pct)
{
    int icon_w = icon ? icon_size(icon->res).w : 0;
    int avail = gctx->body.size.w - icon_w - PANEL_PAD * 2;
    int trail_w = trailing_width(trailing);

    // a headed body is only about 25px tall, so the value tops out at Teko 22 to leave the bar
    // room. with no header the tile is taller, so it starts at Teko 26 and steps down to fit
    FontId rungs[3];
    int rung_count = 0;
    if (gctx->headerless)
    {
        rungs[rung_count++] = FONT_TEKO_26;
        rungs[rung_count++] = FONT_TEKO_24;
    }
    rungs[rung_count++] = FONT_TEKO_22;

    int text_w = 0;
    FontId font = fit_value_font(val, rungs, rung_count, avail, trail_w, icon_w != 0,
                                 gctx->headerless, rungs[0], &text_w);

    // a value that still crowds the icon on the smallest rung drops its unit. the number is what
    // the tile is for, and the unit would land on the icon. the ladder runs again without the
    // unit, so the value gets back whatever size it fits at on its own
    if (trail_w && text_w + trail_w > avail)
    {
        trail_w = 0;
        font = fit_value_font(val, rungs, rung_count, avail, 0, icon_w != 0, gctx->headerless,
                              rungs[0], &text_w);
    }

    // the value draws full width, the same as gh_stat_1x2. clipping it to avail would cut a
    // long value off rather than let it run to the icon's edge
    int value_dy = gctx->headerless ? BAR_VALUE_DY_HEADERLESS : BAR_VALUE_DY_HEADED;
    GRect box = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD, BAR_VALUE_H), GAlignTopLeft,
                            EDGE_PAD(gctx), value_dy);
    box = metric_baseline(font, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(font), box, GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    if (trail_w)
    {
        draw_trailing(gctx, trailing, trail_w, box, font, text_w);
    }

    if (icon)
    {
        gh_icon_bottom_of(gctx, icon, box, font);
    }

    // the bar stops short of the icon, the same as the 2x2 bar does
    int bar_w = gctx->body.size.w - EDGE_PAD(gctx) * 2 - icon_w - (icon ? BAR_ICON_GAP : 0);
    if (bar_w <= 0)
    {
        return;
    }
    GRect bar = grid_anchor(gctx, GSize(bar_w, BAR_1X2_H), GAlignBottomLeft,
                            EDGE_PAD(gctx), -2);
    gh_progress_bar(gctx, bar, pct);
}

void gh_bar_1x4(GridCtx *gctx, const char *val, const char *trailing, const IconSpec *icon, int pct)
{
    const int value_h = 30;

    int icon_w = icon ? icon_size(icon->res).w : 0;
    int trail_w = trailing_width(trailing);
    GSize text = graphics_text_layout_get_content_size(val, fonts_get(FONT_TEKO_26),
                     GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);

    GRect box = grid_anchor(gctx, GSize(text.w + 2, value_h), GAlignLeft, EDGE_PAD(gctx), 0);
    box = metric_baseline(FONT_TEKO_26, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(FONT_TEKO_26), box, GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    if (trail_w)
    {
        draw_trailing(gctx, trailing, trail_w, box, FONT_TEKO_26, text.w);
    }

    if (icon)
    {
        icon_draw(gctx, icon, GAlignRight, -EDGE_PAD(gctx), 0);
    }

    // the bar fills whatever the value and the icon leave, centred on the row
    int left = gctx->body.origin.x + EDGE_PAD(gctx) + text.w + trail_w + BAR_1X4_GAP;
    int right = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx) - icon_w
                - (icon ? BAR_1X4_GAP : 0);
    if (right - left < 8)
    {
        return;
    }
    GRect bar = GRect(left, gctx->body.origin.y + (gctx->body.size.h - BAR_1X4_H) / 2,
                      right - left, BAR_1X4_H);
    gh_progress_bar(gctx, bar, pct);
}

void gh_stat_1x2(GridCtx *gctx, const char *val, const char *trailing, FontId font,
                 const IconSpec *icon)
{
    // content budget: from the left pad across to the icon. PANEL_PAD*2 is the left pad
    // plus the icon's matching right inset (same as gh_mini_row)
    int icon_w = icon ? icon_size(icon->res).w : 0;
    int avail  = gctx->body.size.w - icon_w - PANEL_PAD * 2;

    // the trailing unit (AM/PM, MIN, ...) is pinned just past the value, so it slides left
    // when the value font shrinks. measure it once and fold it into the collision test
    int trail_w = trailing_width(trailing);

    // pick the largest value font that clears the icon, then step down: Teko 26 -> 24 -> 22,
    // mirroring the date ladder in time_date.c. no-header tiles are taller, so they get an
    // extra bigger rung on top (Teko 34). short values grow into the space, long ones fall
    // back to the same sizes as a headed tile so they still clear the icon
    FontId rungs[4];
    int rung_count = 0;
    if (gctx->headerless)
    {
        rungs[rung_count++] = FONT_TEKO_34;
    }
    rungs[rung_count++] = font;          // usually Teko 26
    rungs[rung_count++] = FONT_TEKO_24;
    rungs[rung_count++] = FONT_TEKO_22;

    int    text_w = 0;
    FontId draw_font = fit_value_font(val, rungs, rung_count, avail, trail_w,
                                      icon_w != 0, gctx->headerless, font, &text_w);

    // the big rung is taller, so its box needs the taller height to centre right
    int value_h = (draw_font == FONT_TEKO_34) ? 38 : 30;

    // value still draws full width. the font choice (not truncation) keeps it off the icon
    int box_w = gctx->body.size.w - PANEL_PAD;

    GRect r = gh_value_left(gctx, val, draw_font, box_w, value_h, PANEL_PAD);

    if (trail_w)
    {
        draw_trailing(gctx, trailing, trail_w, r, draw_font, text_w);
    }

    if (icon)
    {
        gh_icon_bottom_of(gctx, icon, r, draw_font);
    }
}

void gh_mini_row(GridCtx *gctx, const char *label, const char *value, const char *trailing,
                 const IconSpec *icon, int top)
{
    int iw = icon ? icon_size(icon->res).w : 0;

    // caption label in XS in the subtitle colour
    GRect label_r = grid_anchor(gctx, GSize(gctx->body.size.w, 14), GAlignTopLeft, PANEL_PAD, top);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, label, fonts_get(FONT_STM_12), label_r,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // value in SM. leave room for the icon unless a unit follows the value
    GSize ts = graphics_text_layout_get_content_size(value, fonts_get(FONT_TEKO_26),
                   GRect(0, 0, gctx->body.size.w, 30), GTextOverflowModeFill, GTextAlignmentLeft);
    int box_w = (trailing && trailing[0]) ? ts.w + 2 : gctx->body.size.w - iw - PANEL_PAD * 2;

    GRect val_r = grid_anchor(gctx, GSize(box_w, 30), GAlignTopLeft, PANEL_PAD, top + 6);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, value, fonts_get(FONT_TEKO_26), val_r,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    if (trailing && trailing[0])
    {
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, trailing, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                           GRect(val_r.origin.x + ts.w + 2, val_r.origin.y + 12, 40, 14),
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }

    if (icon)
    {
        gh_icon_bottom_of(gctx, icon, val_r, FONT_TEKO_26);
    }
}
