#include "engine/grid_engine.h"
#include "composite_datetime.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/icons.h"
#include "io/stores/time_store.h"
#include "io/stores/system_store.h"
#include "system/settings/settings.h"
#include "settings_schema.h"
#include "ui/fonts.h"
#include "text/text_case.h"
#include "mosaic/draw/metrics.h"
#include <string.h>
#include <stdio.h>

// the formatted date only changes at midnight (or when the format setting changes) so build it
// once and reuse it. strftime plus the upcasing on every repaint is wasted work otherwise
static const char *cached_date(const struct tm *tick)
{
    static char s_date[32];
    static char s_fmt[24] = "";
    static int  s_yday = -1;

    const char *fmt = settings_str(SETTING_DATE_FORMAT);
    if (tick->tm_yday != s_yday || strcmp(fmt, s_fmt) != 0)
    {
        strftime(s_date, sizeof(s_date), fmt, tick);
        text_to_upper(s_date);
        s_yday = tick->tm_yday;
        strncpy(s_fmt, fmt, sizeof(s_fmt) - 1);
        s_fmt[sizeof(s_fmt) - 1] = '\0';
    }
    return s_date;
}

// the 1x4 bar: Bluetooth top left, the big clock on the right, the date block on the left. the
// engine has already drawn the border and handed over the full tile (this size is headerless)
static void draw_bar(GridCtx *gctx)
{
    GContext *ctx = gctx->ctx;
    GColor value = gctx->color_value;
    GColor subtitle = gctx->color_subtitle;

    const struct tm *tick = time_store_tm();
    char time_buf[8] = "--:--";
    bool meridiem = gridlock_format_clock(time_buf, sizeof(time_buf), tick->tm_hour, tick->tm_min);
    const char *date_buf = cached_date(tick);

    // top left corner is the Bluetooth icon (shared cache, freed by icons_cleanup)
    GBitmap *glyph = icon_get(system_store_bluetooth() ? RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_ON
                                                       : RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_OFF);
    if (glyph)
    {
        GRect dst = grid_anchor(gctx, gbitmap_get_bounds(glyph).size, GAlignTopLeft, 2, 5);
        blit_tinted(gctx, glyph, dst);
    }

    // measure the time text first
    GSize ts = graphics_text_layout_get_content_size(time_buf, fonts_get(FONT_TEKO_54), GRect(0, 0, gctx->body.size.w, 42), GTextOverflowModeFill, GTextAlignmentLeft);
    int am_w = 0;
    if (meridiem)
    {
        bool is_am = tick->tm_hour < 12;
        am_w = graphics_text_layout_get_content_size(is_am ? "AM" : "PM", fonts_get(FONT_STM_12), GRect(0, 0, 30, 14), GTextOverflowModeFill, GTextAlignmentLeft).w;
    }

    int gap = 0; // Teko font has ~3px built-in right padding, so 0 gap = 3px visual gap
    int total_w = ts.w + gap + am_w;

    // right block the big clock, vertically centered visually
    // nudged an extra 1px left to ensure exactly 4px visual padding from the right edge
    GRect time_rect = grid_anchor(gctx, GSize(total_w, 42), GAlignRight, -PANEL_PAD - 1, -16);

    // the time drawn
    graphics_context_set_text_color(ctx, value);
    graphics_draw_text(ctx, time_buf, fonts_get(FONT_TEKO_54), time_rect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    if (meridiem)
    {
        bool is_am = tick->tm_hour < 12;
        // am/pm sits flush against the time text's bounding box (which has 3px visual padding)
        GRect am_rect = time_rect;
        am_rect.origin.x += ts.w + gap;
        am_rect.size.w = am_w;
        am_rect.size.h = 14;

        const FontMetric *vm = metric_for(FONT_TEKO_54);
        const FontMetric *tm = metric_for(FONT_STM_12);
        // align to the bottom baseline of the big text, exactly like gh_stat_1x2
        am_rect.origin.y = time_rect.origin.y + vm->top_pad + vm->cap_h - (tm->top_pad + tm->cap_h) - 1;

        graphics_context_set_text_color(ctx, subtitle);
        graphics_draw_text(ctx, is_am ? "AM" : "PM", fonts_get(FONT_STM_12), am_rect, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }

    // left block the date
    GRect date_rect = GRect(gctx->body.origin.x + 17, gctx->body.origin.y + 5, 44, 34);
    FontId date_font = FONT_STM_14;
    GSize date_size = graphics_text_layout_get_content_size(date_buf, fonts_get(date_font), date_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);

    if (date_size.h > date_rect.size.h)
    {
        date_font = FONT_STM_12;
        date_size = graphics_text_layout_get_content_size(date_buf, fonts_get(date_font), date_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter);
    }

    if (date_size.h > date_rect.size.h)
    {
        date_size.h = date_rect.size.h; // cap it to strictly respect the padded box
    }

    date_rect.origin.y += (date_rect.size.h - date_size.h) / 2 - 2;

    graphics_context_set_text_color(ctx, value);
    graphics_draw_text(ctx, date_buf, fonts_get(date_font), date_rect,
                       GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

// the 2x2 block: Bluetooth top-left, am/pm small in the top-right corner so the clock can use a
// bigger font across the full width, the clock centred with the date under it
static void draw_square(GridCtx *gctx)
{
    const struct tm *tick = time_store_tm();

    char time_buf[8] = "--:--";
    bool meridiem = gridlock_format_clock(time_buf, sizeof(time_buf), tick->tm_hour, tick->tm_min);
    const char *ampm = meridiem ? (tick->tm_hour < 12 ? "AM" : "PM") : NULL;

    const char *date_buf = cached_date(tick);

    // top left corner is the Bluetooth icon, the small 10px variant so it stays subtle on the
    // tighter 2x2 tile (shared cache, freed by icons_cleanup)
    GBitmap *glyph = icon_get(system_store_bluetooth() ? RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_ON_SM
                                                       : RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_OFF_SM);
    if (glyph)
    {
        GRect dst = grid_anchor(gctx, gbitmap_get_bounds(glyph).size, GAlignTopLeft, 3, 4);
        blit_tinted(gctx, glyph, dst);
    }

    if (ampm)
    {
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        GRect corner = GRect(gctx->body.origin.x, gctx->body.origin.y,
                             gctx->body.size.w - PANEL_PAD - 1, 14);
        graphics_draw_text(gctx->ctx, ampm, fonts_get(FONT_STM_12), corner,
                           GTextOverflowModeFill, GTextAlignmentRight, NULL);
    }

    // the clock big and centred, the date under it in the bigger Share Tech Mono date font
    gh_value_top(gctx, time_buf, FONT_TEKO_46, 46, 0, -1);

    GRect date_rect = GRect(gctx->body.origin.x, gctx->body.origin.y + gctx->body.size.h - 24,
                            gctx->body.size.w, 16);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, date_buf, fonts_get(FONT_STM_14), date_rect,
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

// the split is by size, not by header: the 1x4 is always headerless (headerless_sizes) and
// draws the bar, while the 2x2 draws the block either way
static void composite_datetime_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x4)
    {
        draw_bar(gctx);
    }
    else
    {
        draw_square(gctx);
    }
}

const ModuleDef mod_composite_datetime_def = {
    .label = "TIME",
    .sizes = SZ_1x4 | SZ_2x2,
    .features = FEATURE_TIME | FEATURE_SYSTEM,
    .headerless_sizes = SZ_1x4,
    .body = composite_datetime_body
};
