#include "engine/grid_engine.h"
#include "weather_current.h"
#include "draw/grid_helpers.h"
#include "draw/metrics.h"
#include "draw/wx_icon.h"
#include "io/stores/weather_store.h"
#include "ui/weather/labels.h"
#include "settings_schema.h"
#include <stdio.h>
#include <string.h>

static void weather_current_inner_2x2(GridCtx *gctx)
{
    // the number rides big with the "°C"/"°F" alongside, formatted from the setting like the panels
    int temp_value = weather_store_temp();
    char temp[12];
    if (temp_value == WEATHER_NO_TEMP)
    {
        snprintf(temp, sizeof(temp), "--");
    }
    else
    {
        snprintf(temp, sizeof(temp), "%d%s", temp_value, gridlock_temp_unit_label());
    }

    // the 2x2 is roomy, so show the full readable label ("Partly Cloudy"). the
    // label tables resolve the "_NIGHT" token themselves, so no stripping here.
    const char *raw = weather_store_cond();
    const char *cond = (!raw[0] || !strcmp(raw, "--")) ? "--" : wx_label_long(raw);

    IconMargins margin;
    GBitmap *bmp = wx_icon_get(&margin);
    GSize gs = bmp ? gbitmap_get_bounds(bmp).size : GSize(0, 0);

    // big temperature up in the top-left
    // the -7 nudges the big number up so it looks right
    GRect t_val_rect = grid_anchor(gctx, GSize(gctx->body.size.w, 38), GAlignTopLeft, PANEL_PAD, -7);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, temp, fonts_get(FONT_TEKO_34), t_val_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // the condition text sits under it
    GRect cond_rect = grid_anchor(gctx, GSize(gctx->body.size.w, 14), GAlignLeft, PANEL_PAD, -1);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, cond, fonts_get_system_font(FONT_KEY_GOTHIC_14), cond_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    if (bmp)
    {
        // bottom-right corner like the shared 2x2 stat icon, auto-trimmed so the visible
        // art rests in the corner instead of floating up on its own whitespace
        GRect icon_rect = grid_anchor(gctx, gs, GAlignBottomRight,
                                      -EDGE_PAD(gctx) + margin.e, -5 + margin.s);
        blit_tinted(gctx, bmp, icon_rect);
    }
}

static void weather_current_inner_1x2(GridCtx *gctx)
{
    // the 1x2 is tight, so show the compact label (seeded to the token text, e.g.
    // "PCLDY"). the label tables resolve the "_NIGHT" token, so no stripping here.
    const char *raw = weather_store_cond();
    const char *cond = (!raw[0] || !strcmp(raw, "--")) ? "--" : wx_label_short(raw);

    IconMargins margin;
    GBitmap *bmp = wx_icon_get(&margin);
    GSize gs = bmp ? gbitmap_get_bounds(bmp).size : GSize(0, 0);

    int value_h = 30;
    int text_w = gctx->body.size.w - (gs.w ? gs.w + PANEL_PAD * 2 : PANEL_PAD * 2);

    // use the skinny font if the condition is too wide to fit
    GSize ts = graphics_text_layout_get_content_size(cond, fonts_get(FONT_TEKO_26),
                   GRect(0, 0, 1000, value_h), GTextOverflowModeFill, GTextAlignmentLeft);
    FontId v_font = (ts.w > text_w) ? FONT_TEKO_22 : FONT_TEKO_26;

    gh_value_left(gctx, cond, v_font, text_w, value_h, PANEL_PAD);

    if (bmp)
    {
        // corner-pin the picture EDGE_PAD in from the bottom-right, auto-trimmed, matching
        // the other 1x2 stat icons
        int x = gctx->body.origin.x + gctx->body.size.w - gs.w - EDGE_PAD(gctx) + margin.e;
        int y = gctx->body.origin.y + gctx->body.size.h - gs.h - EDGE_PAD(gctx) + margin.s;
        blit_tinted(gctx, bmp, GRect(x, y, gs.w, gs.h));
    }
}

// TODO: re-tune for the taller body when gctx->headerless
static void weather_current_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x2)
    {
        weather_current_inner_1x2(gctx);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        weather_current_inner_2x2(gctx);
    }
}

void weather_current_cleanup(void)
{
    wx_icon_cleanup();
}

const ModuleDef mod_weather_current_def = {
    .label = "WEATHER",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_WEATHER,
    .body = weather_current_body,
    .cleanup = weather_current_cleanup
};
