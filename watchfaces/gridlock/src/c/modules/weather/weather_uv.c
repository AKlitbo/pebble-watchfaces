#include "weather_uv.h"
#include "draw/stat_panel.h"
#include "io/stores/weather_store.h"
#include "math/scale.h"
#include "ui/fonts.h"
#include <stdio.h>

// the gauge scales a reading over a fixed 0 to 11 so an 11 or higher fills the whole bar
#define UV_SCALE_MAX 11

// EPA-style risk band for a uv index. kept short so it fits as the value's trailing unit
static const char *uv_risk(int uv)
{
    if (uv <= 2)  { return "LOW"; }
    if (uv <= 5)  { return "MOD"; }
    if (uv <= 7)  { return "HIGH"; }
    if (uv <= 10) { return "V.HIGH"; }
    return "EXTREME";
}

// writes the uv reading, or "--" when there is none
static void uv_format(int uv, char *out, size_t n)
{
    if (uv < 0)
    {
        snprintf(out, n, "--");
    }
    else
    {
        snprintf(out, n, "%d", uv);
    }
}

// the 1x2 is a plain stat panel with the risk band as its trailing unit
static void uv_value(char *out, size_t n, const char **unit_out)
{
    int uv = weather_store_uv();
    uv_format(uv, out, n);
    if (uv >= 0)
    {
        *unit_out = uv_risk(uv);
    }
}

static const StatPanel1x2 uv_desc = {
    .value_1x2 = uv_value,
    .icon = &ICON_UV,
    .font = FONT_TEKO_26,
};

// the 2x2 is the battery panel's shape: big number, a caption, then a segmented low to high bar
static void weather_uv_2x2(GridCtx *gctx, int uv, const char *val)
{
    gh_value_top(gctx, val, FONT_TEKO_34, 38, 0, -7);

    gh_caption(gctx, "OF A MAX OF 11+", 27);

    int level = 0;
    if (uv > 0)
    {
        level = clamp_int(uv * 100 / UV_SCALE_MAX, 0, 100);
    }
    GRect gauge_rect = grid_anchor(gctx, GSize(gctx->body.size.w - 16, 9), GAlignTop, 0, 44);
    gh_gauge(gctx, gauge_rect, UV_SCALE_MAX, level);

    // low and high sit under the two ends of the bar
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    GRect ends = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD * 2, 14), GAlignBottomLeft,
                             PANEL_PAD, -EDGE_PAD(gctx) + 2);
    graphics_draw_text(gctx->ctx, "LOW", fonts_get(FONT_STM_12), ends,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    graphics_draw_text(gctx->ctx, "HIGH", fonts_get(FONT_STM_12), ends,
                       GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

static void weather_uv_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x2)
    {
        stat_panel_draw_1x2(gctx, &uv_desc);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        int uv = weather_store_uv();
        char val[12];
        uv_format(uv, val, sizeof(val));
        weather_uv_2x2(gctx, uv, val);
    }
}

const ModuleDef mod_weather_uv_def = {
    .label = "UV INDEX",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_WEATHER,
    .body = weather_uv_body
};
