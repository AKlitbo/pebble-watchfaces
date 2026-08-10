#include "engine/grid_engine.h"
#include "time_date.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/time_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "text/text_case.h"
#include <stdio.h>
#include <string.h>

// true when val fits across avail in this font. only the width matters so the measured height
// goes nowhere. the 1000x30 box is just somewhere roomy to lay the text out in
static bool date_fits(const char *val, GFont font, int avail)
{
    GSize ts = graphics_text_layout_get_content_size(val, font, GRect(0, 0, 1000, 30),
                   GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
    return ts.w <= avail;
}

static void time_date_inner_1x2(GridCtx *gctx, const char *val)
{
    const int pad = PANEL_PAD;
    int avail = gctx->body.size.w - pad * 2 - 2;

    // tries four sizes biggest first and keeps the first one that fits
    // Teko 26 then Teko 22 then Gothic 18 bold then STM 12
    // the Teko and STM ones get their baseline tweak from metrics
    // the system font one uses its own little nudge instead
    GFont   font;
    FontId  metric_id = FONT_COUNT; // FONT_COUNT means use the explicit y_offset instead
    int     value_h = 30;
    int     y_offset = 0;

    if (date_fits(val, fonts_get(FONT_TEKO_26), avail))
    {
        font = fonts_get(FONT_TEKO_26);
        metric_id = FONT_TEKO_26;
    }
    else if (date_fits(val, fonts_get(FONT_TEKO_22), avail))
    {
        font = fonts_get(FONT_TEKO_22);
        metric_id = FONT_TEKO_22;
    }
    else if (date_fits(val, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), avail))
    {
        font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
        value_h = 20;
        y_offset = -3;
    }
    else
    {
        font = fonts_get(FONT_STM_12);
        metric_id = FONT_STM_12;
        value_h = 14;
    }

    GRect value_rect = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD * 2, value_h), GAlignLeft, pad, y_offset);
    if (metric_id != FONT_COUNT)
    {
        value_rect = metric_baseline(metric_id, value_rect);
    }

    graphics_draw_text(gctx->ctx, val, font, value_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

// TODO: re-tune for the taller body when gctx->headerless
static void time_date_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    char val[24] = "--";
    strftime(val, sizeof(val), settings_str(SETTING_DATE_FORMAT), time_store_tm());
    text_to_upper(val);

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    time_date_inner_1x2(gctx, val);
}

const ModuleDef mod_time_date_def = {
    .label = "DATE",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .body = time_date_body
};
