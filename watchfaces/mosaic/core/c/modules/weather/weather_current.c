#include "engine/grid_engine.h"
#include "weather_current.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/metrics.h"
#include "mosaic/draw/wx_icon.h"
#include "io/stores/weather_store.h"
#include "weather/wx_label.h"
#include "settings_schema.h"
#include <stdio.h>
#include <string.h>

// how wide a line of text draws, so the 2x2 can tell whether its glyph clears the words
static int text_width(const char *text, GFont font)
{
    return graphics_text_layout_get_content_size(text, font, GRect(0, 0, 200, 40), GTextOverflowModeFill,
                                                 GTextAlignmentLeft).w;
}

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

    // the 2x2 is roomy, so it shows the sky in words ("Partly Cloudy"), which the phone sends. a
    // reading from before the phone sent any, such as a screenshot seed, falls back to the token
    const char *raw = weather_store_cond();
    char word[16];
    wx_label_short(word, sizeof(word), raw);
    const char *label = weather_store_cond_label();
    const char *cond = !raw[0] ? "--" : (label[0] ? label : word);

    // big temperature up in the top-left
    // the -7 nudges the big number up so it looks right
    GRect t_val_rect = grid_anchor(gctx, GSize(gctx->body.size.w, 38), GAlignTopLeft, PANEL_PAD, -7);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, temp, fonts_get(FONT_TEKO_34), t_val_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // the condition text sits under it
    GRect cond_rect = grid_anchor(gctx, GSize(gctx->body.size.w, 14), GAlignLeft, PANEL_PAD, -1);
    GFont cond_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, cond, cond_font, cond_rect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    // the 40px glyph fills the room under the temperature. a tall one, such as the sun, reaches up
    // into the text's rows, and then it is only kept when both texts end short of it. otherwise
    // the panel draws the 24px glyph, so the words are never cut and the art never touches them
    IconMargins margin;
    GBitmap *bmp = wx_icon_get_big(&margin);
    GRect icon_rect = GRect(0, 0, 0, 0);
    if (bmp)
    {
        GSize gs = gbitmap_get_bounds(bmp).size;
        icon_rect = grid_anchor(gctx, gs, GAlignBottomRight, -EDGE_PAD(gctx) + margin.e, -5 + margin.s);

        int art_top = icon_rect.origin.y + margin.n;
        int art_left = icon_rect.origin.x + margin.w;
        // Teko 34's digits end about 34px into their box
        int temp_bottom = t_val_rect.origin.y + 34;
        int temp_right = t_val_rect.origin.x + text_width(temp, fonts_get(FONT_TEKO_34));
        int cond_bottom = cond_rect.origin.y + cond_rect.size.h;
        int cond_right = cond_rect.origin.x + text_width(cond, cond_font);

        bool hits_temp = art_top < temp_bottom && temp_right > art_left - 2;
        bool hits_cond = art_top < cond_bottom && cond_right > art_left - 2;
        if (hits_temp || hits_cond)
        {
            bmp = wx_icon_get(&margin);
            if (bmp)
            {
                gs = gbitmap_get_bounds(bmp).size;
                icon_rect = grid_anchor(gctx, gs, GAlignBottomRight, -EDGE_PAD(gctx) + margin.e, -5 + margin.s);
            }
        }
    }

    if (bmp)
    {
        blit_tinted(gctx, bmp, icon_rect);
    }
}

static void weather_current_inner_1x2(GridCtx *gctx)
{
    // the 1x2 is tight, so it shows the token itself, such as "PCLDY", less its night suffix
    const char *raw = weather_store_cond();
    char word[16];
    wx_label_short(word, sizeof(word), raw);
    const char *cond = !raw[0] ? "--" : word;

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
        // the other 1x2 stat icons. a headerless one centres it on the value instead
        int x = gctx->body.origin.x + gctx->body.size.w - gs.w - EDGE_PAD(gctx) + margin.e;
        int y = gctx->headerless ? gh_icon_y_middle(gctx, gs.h, margin)
                                 : gctx->body.origin.y + gctx->body.size.h - gs.h - EDGE_PAD(gctx) + margin.s;
        blit_tinted(gctx, bmp, GRect(x, y, gs.w, gs.h));
    }
}

// TODO: grow the value for the taller body when gctx->headerless, as gh_stat_1x2 does
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
