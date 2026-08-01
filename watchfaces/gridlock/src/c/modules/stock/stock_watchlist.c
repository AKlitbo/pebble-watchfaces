/**
 * @file stock_watchlist.c
 * @brief The 2x4 watchlist panel. Splits the body into a 2x2 grid and draws one stock per
 * tile: the ticker over its price with a coloured up/down percent on the price line. An
 * empty slot leaves its tile blank so a two-stock list does not draw dead boxes.
 * @ingroup gridlock_mod_stock
 */
#include "engine/grid_engine.h"
#include "stock_watchlist.h"
#include "text/number_format.h"
#include "stock_common.h"
#include "io/stores/stock_store.h"
#include <stdio.h>

// pad the tile contents in from its edges so they clear the dotted dividers
#define TILE_PAD 3

// draws one stock in its tile: the ticker and its coloured percent share the top line, then
// the price gets the whole width on the line below so even a four-digit price fits
static void draw_tile(GridCtx *gctx, GRect box, const StockSlot *slot)
{
    if (!slot)
    {
        return; // an empty slot leaves a blank tile
    }

    int inner_x = box.origin.x + TILE_PAD;
    int inner_w = box.size.w - TILE_PAD * 2;

    // the two lines sit as one stack, centred top to bottom in the tile
    int stack_top = box.origin.y + (box.size.h - 28) / 2;
    int price_y = stack_top + 14;

    // percent + triangle claim the right end of the top line. work it out first so the ticker
    // knows how much room it has left
    int ticker_right = box.origin.x + box.size.w - TILE_PAD;

    if (slot->ok)
    {
        char pct[16];
        fmt_pct_signed(pct, sizeof(pct), slot->change_pct);
        GSize ps = graphics_text_layout_get_content_size(pct, fonts_get(FONT_STM_12),
                       GRect(0, 0, 1000, 14), GTextOverflowModeFill, GTextAlignmentLeft);
        int pct_x = box.origin.x + box.size.w - TILE_PAD - ps.w;

        graphics_context_set_text_color(gctx->ctx, stock_trend_color(gctx, slot->change_pct));
        graphics_draw_text(gctx->ctx, pct, fonts_get(FONT_STM_12),
                           GRect(pct_x, stack_top, ps.w + 2, 14),
                           GTextOverflowModeFill, GTextAlignmentLeft, NULL);

        if (slot->change_pct != 0)
        {
            GRect tri = GRect(pct_x - 9, stack_top + 3, 6, 7);
            stock_draw_trend_triangle(gctx, tri, slot->change_pct);
            ticker_right = pct_x - 11;
        }
        else
        {
            ticker_right = pct_x - 2;
        }
    }

    // ticker on the left of the top line, clipped so it never runs under the percent
    const char *ticker = slot->symbol[0] ? slot->symbol : "--";
    int ticker_w = ticker_right - inner_x;
    if (ticker_w < 0)
    {
        ticker_w = 0;
    }
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, ticker, fonts_get(FONT_STM_12),
                       GRect(inner_x, stack_top, ticker_w, 14),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);

    // price on the line below, the whole tile width to itself
    char price[16];
    if (slot->ok)
    {
        fmt_hundredths(price, sizeof(price), slot->price_cents);
    }
    else
    {
        snprintf(price, sizeof(price), "--");
    }

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, price, fonts_get(FONT_STM_12),
                       GRect(inner_x, price_y, inner_w, 14),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

// dotted cross between the four tiles, matching the panel border. only when bordered so a
// borderless watchlist stays clean
static void draw_dividers(GridCtx *gctx, int half_w, int half_h)
{
    if (gctx->borderless)
    {
        return;
    }

    GRect b = gctx->body;
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);

    int x_mid = b.origin.x + half_w;
    for (int y = b.origin.y + 1; y < b.origin.y + b.size.h - 1; y += 2)
    {
        graphics_draw_pixel(gctx->ctx, GPoint(x_mid, y));
    }

    int y_mid = b.origin.y + half_h;
    for (int x = b.origin.x + 1; x < b.origin.x + b.size.w - 1; x += 2)
    {
        graphics_draw_pixel(gctx->ctx, GPoint(x, y_mid));
    }
}

static void stock_watchlist_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    const StockStrip *strip = stock_store_strip();
    GRect b = gctx->body;

    // no reading yet so show one centred placeholder instead of a blank tile
    if (strip->count == 0)
    {
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, "--", fonts_get(FONT_STM_14), b,
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        return;
    }

    int half_w = b.size.w / 2;
    int half_h = b.size.h / 2;

    // top-left, top-right, bottom-left, bottom-right. slot() hands back NULL past the
    // filled count so the extra tiles stay blank
    GRect cells[4] = {
        GRect(b.origin.x, b.origin.y, half_w, half_h),
        GRect(b.origin.x + half_w, b.origin.y, b.size.w - half_w, half_h),
        GRect(b.origin.x, b.origin.y + half_h, half_w, b.size.h - half_h),
        GRect(b.origin.x + half_w, b.origin.y + half_h, b.size.w - half_w, b.size.h - half_h),
    };

    for (uint8_t i = 0; i < 4; i++)
    {
        draw_tile(gctx, cells[i], stock_store_slot(i));
    }

    draw_dividers(gctx, half_w, half_h);
}

const ModuleDef mod_stock_watchlist_def = {
    .label = "STOCKS",
    .sizes = SZ_2x4,
    .features = FEATURE_STOCK,
    .theme_alias = MOD_STOCK_QUOTE,
    .body = stock_watchlist_body
};
