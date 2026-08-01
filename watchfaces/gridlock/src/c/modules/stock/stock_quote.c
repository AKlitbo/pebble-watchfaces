/**
 * @file stock_quote.c
 * @brief The 2x2 single stock panel. Reads the first slot from the stock store and shows
 * the ticker over the price with a coloured up/down percent underneath. A failed fetch
 * carries a short status word in the ticker slot so the panel shows "RATE LIMIT" and
 * friends instead of a stale number.
 * @ingroup gridlock_mod_stock
 */
#include "engine/grid_engine.h"
#include "stock_quote.h"
#include "text/number_format.h"
#include "stock_common.h"
#include "draw/metrics.h"
#include "io/stores/stock_store.h"
#include <stdio.h>

// TODO: re-tune for the taller body when gctx->headerless
static void stock_quote_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    const StockSlot *slot = stock_store_slot(0);
    int body_w = gctx->body.size.w;
    int text_w = body_w - PANEL_PAD * 2;

    // ticker up top. a failed fetch put its status word in here so it shows either way
    const char *ticker = (slot && slot->symbol[0]) ? slot->symbol : "--";
    GRect trect = grid_anchor(gctx, GSize(text_w, 16), GAlignTopLeft, PANEL_PAD, 0);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, ticker, fonts_get(FONT_STM_14), trect,
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    // price big under the ticker. no live reading shows a plain --
    char price[16];
    if (slot && slot->ok)
    {
        fmt_hundredths(price, sizeof(price), slot->price_cents);
    }
    else
    {
        snprintf(price, sizeof(price), "--");
    }

    // step the font down for an expensive ticker so a four-digit price does not clip
    FontId price_font = FONT_TEKO_34;
    GSize price_size = graphics_text_layout_get_content_size(price, fonts_get(FONT_TEKO_34),
                           GRect(0, 0, 1000, 38), GTextOverflowModeFill, GTextAlignmentLeft);
    if (price_size.w > text_w)
    {
        price_font = FONT_TEKO_26;
    }

    GRect prect = grid_anchor(gctx, GSize(text_w, 38), GAlignTopLeft, PANEL_PAD, 14);
    prect = metric_baseline(price_font, prect);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, price, fonts_get(price_font), prect,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // percent along the bottom with the trend triangle ahead of it, both in the trend
    // colour. only when there is a live reading
    if (slot && slot->ok)
    {
        char pct[16];
        fmt_pct_signed(pct, sizeof(pct), slot->change_pct);

        // keep the row clear of the bottom border. the Teko descenders reach lower than the
        // cap box so sit it a touch higher than the plain bottom margin would
        int row_y = gctx->body.origin.y + gctx->body.size.h - 28;
        int text_x = gctx->body.origin.x + PANEL_PAD;

        // the triangle sits just left of the number and nudges the text across when shown
        if (slot->change_pct != 0)
        {
            GRect tri = GRect(text_x, row_y + 6, 9, 9);
            stock_draw_trend_triangle(gctx, tri, slot->change_pct);
            text_x += 13;
        }

        int pct_w = gctx->body.origin.x + gctx->body.size.w - PANEL_PAD - text_x;
        GRect pr = GRect(text_x, row_y, pct_w, 20);
        pr = metric_baseline(FONT_TEKO_22, pr);
        graphics_context_set_text_color(gctx->ctx, stock_trend_color(gctx, slot->change_pct));
        graphics_draw_text(gctx->ctx, pct, fonts_get(FONT_TEKO_22), pr,
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
}

const ModuleDef mod_stock_quote_def = {
    .label = "STOCK",
    .sizes = SZ_2x2,
    .features = FEATURE_STOCK,
    .body = stock_quote_body
};
