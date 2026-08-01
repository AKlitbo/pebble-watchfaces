#include "engine/grid_engine.h"
#include "weather_hilo.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include "draw/metrics.h"
#include "weather_forecast_common.h"
#include "io/stores/weather_store.h"
#include <stdio.h>

#define HILO_FONT FONT_TEKO_26
#define HILO_GAP  3   // arrow to its own number
#define HILO_SEP  8   // centred gap between the high group and the low group

static int text_width(const char *text)
{
    return graphics_text_layout_get_content_size(text, fonts_get(HILO_FONT),
               GRect(0, 0, 1000, 24), GTextOverflowModeFill, GTextAlignmentLeft).w;
}

// draws a number left-aligned at a body-relative x, centred top to bottom. the 30px box
// centres the value and metric_baseline trims the font's invisible top pad so it sits right
static void draw_num(GridCtx *gctx, int rel_x, int box_w, const char *num)
{
    GRect box = grid_anchor(gctx, GSize(box_w + 4, 30), GAlignLeft, rel_x, 0);
    box = metric_baseline(HILO_FONT, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, num, fonts_get(HILO_FONT), box,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void weather_hilo_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    char hi[8], lo[8];
    weather_fmt_temp(hi, sizeof(hi), weather_store_temp_max(), "%d");
    weather_fmt_temp(lo, sizeof(lo), weather_store_temp_min(), "%d");

    int up_w = icon_visible_width(RESOURCE_ID_ICON_WEATHER_HILO_UP);
    int dn_w = icon_visible_width(RESOURCE_ID_ICON_WEATHER_HILO_DOWN);
    int hi_w = text_width(hi);
    int lo_w = text_width(lo);

    // symmetric around the panel centre: the high group ends just left of centre and the
    // low group starts just right of it, so both grow outward evenly and stay off the edges
    int centre = gctx->body.size.w / 2;

    int hi_group = up_w + HILO_GAP + hi_w;
    int hi_start = centre - HILO_SEP / 2 - hi_group;
    if (hi_start < PANEL_PAD)
    {
        hi_start = PANEL_PAD;
    }
    icon_draw_res(gctx, RESOURCE_ID_ICON_WEATHER_HILO_UP, GAlignLeft, hi_start, 0);
    draw_num(gctx, hi_start + up_w + HILO_GAP, hi_w, hi);

    int lo_start = centre + HILO_SEP / 2;
    icon_draw_res(gctx, RESOURCE_ID_ICON_WEATHER_HILO_DOWN, GAlignLeft, lo_start, 0);
    draw_num(gctx, lo_start + dn_w + HILO_GAP, lo_w, lo);
}

const ModuleDef mod_weather_hilo_def = {
    .label = "TEMP HI/LO",
    .sizes = SZ_1x2,
    .features = FEATURE_WEATHER,
    .body = weather_hilo_body
};
