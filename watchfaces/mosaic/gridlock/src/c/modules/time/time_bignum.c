/**
 * @file time_bignum.c
 * @brief The big-digit night panels. One draw path centres a number in whatever box the
 * engine hands over and picks the biggest font that still fits, so the same code covers
 * the framed body and the taller headerless tile at both 2x2 and 2x4.
 * @ingroup gridlock_modules
 */
#include "engine/grid_engine.h"
#include "time_bignum.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include "ui/fonts.h"
#include "mosaic/draw/metrics.h"
#include "draw/fonts.h"
#include <string.h>

// fill the tile: only a hair of inset so the digits can run right to the border
#define BIGNUM_PAD 1

// biggest first. the picker walks down until the number fits the box it was given. two glyphs
// in a 2x2 reach TEKO_96, but the five of "HH:MM" in a 2x4 only fit the narrower TEKO_88
static const FontId s_digit_ladder[] = { FONT_TEKO_96, FONT_TEKO_88, FONT_TEKO_72 };

// no extra width margin, the fill is the point
#define BIGNUM_EDGE 0

// biggest ladder font whose digits fit box, by measured width and the font's cap height
static FontId pick_digit_font(const char *text, GRect box)
{
    for (unsigned int i = 0; i < ARRAY_LENGTH(s_digit_ladder); i++)
    {
        FontId font = s_digit_ladder[i];
        // measure in a box wide enough that a big "HH:MM" never wraps, or the wrapped width
        // reads short and we would wrongly pick a font that then overflows the tile
        GSize size = graphics_text_layout_get_content_size(text, fonts_get(font),
                         GRect(0, 0, 1000, 200), GTextOverflowModeFill, GTextAlignmentCenter);
        if (size.w + BIGNUM_EDGE * 2 <= box.size.w && metric_for(font)->cap_h <= box.size.h)
        {
            return font;
        }
    }
    // nothing fit cleanly, fall back to the smallest so it at least draws
    return s_digit_ladder[ARRAY_LENGTH(s_digit_ladder) - 1];
}

// shrinks the box in from every edge so the ink breathes
static GRect inset_box(GRect box)
{
    return GRect(box.origin.x + BIGNUM_PAD, box.origin.y + BIGNUM_PAD,
                 box.size.w - BIGNUM_PAD * 2, box.size.h - BIGNUM_PAD * 2);
}

// centres text in box with the biggest fitting font, ink centred on the cap height
static void draw_digits(GridCtx *gctx, const char *text, GRect box)
{
    FontId font = pick_digit_font(text, box);
    const FontMetric *metric = metric_for(font);

    int y = box.origin.y + (box.size.h - metric->cap_h) / 2 - metric->top_pad;
    // headerless hands over the whole tile, where the digits centre 2px high, so seat them down
    if (gctx->headerless)
    {
        y += 2;
    }
    GRect rect = GRect(box.origin.x, y, box.size.w, metric->line_h);

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), rect,
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

// formats the clock the same way the main readout does (12/24h, leading zero) then splits
// "HH:MM" into its two halves. minutes are always two digits, the hour may be one or two
static void split_clock(char *hour_out, size_t hour_n, char *min_out, size_t min_n)
{
    char buf[8] = "--:--";
    const struct tm *tick = time_store_tm();
    gridlock_format_clock(buf, sizeof(buf), tick->tm_hour, tick->tm_min);

    char *colon = strchr(buf, ':');
    if (colon)
    {
        *colon = '\0';
        strncpy(hour_out, buf, hour_n - 1);
        hour_out[hour_n - 1] = '\0';
        strncpy(min_out, colon + 1, min_n - 1);
        min_out[min_n - 1] = '\0';
    }
    else
    {
        strncpy(hour_out, "--", hour_n - 1);
        hour_out[hour_n - 1] = '\0';
        strncpy(min_out, "--", min_n - 1);
        min_out[min_n - 1] = '\0';
    }
}

static void hour_big_body(GridCtx *gctx)
{
    char hour[6], min[6];
    split_clock(hour, sizeof(hour), min, sizeof(min));
    draw_digits(gctx, hour, inset_box(gctx->body));
}

static void min_big_body(GridCtx *gctx)
{
    char hour[6], min[6];
    split_clock(hour, sizeof(hour), min, sizeof(min));
    draw_digits(gctx, min, inset_box(gctx->body));
}

// the full clock as one big number, colon and all, centred across the whole tile
static void bigclock_body(GridCtx *gctx)
{
    char buf[8] = "--:--";
    const struct tm *tick = time_store_tm();
    gridlock_format_clock(buf, sizeof(buf), tick->tm_hour, tick->tm_min);
    draw_digits(gctx, buf, inset_box(gctx->body));
}

// all three ride under the Digital Clock's colours (see themeHidden in config.ts), so the
// whole big-digit set themes as one under the Time swatch
const ModuleDef mod_time_hour_big_def = {
    .label = "HOUR",
    .sizes = SZ_2x2,
    .features = FEATURE_TIME,
    .theme_alias = MOD_COMPOSITE_DATETIME,
    .body = hour_big_body,
};

const ModuleDef mod_time_min_big_def = {
    .label = "MINUTE",
    .sizes = SZ_2x2,
    .features = FEATURE_TIME,
    .theme_alias = MOD_COMPOSITE_DATETIME,
    .body = min_big_body,
};

const ModuleDef mod_time_bigclock_def = {
    .label = "BIG TIME",
    .sizes = SZ_2x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_COMPOSITE_DATETIME,
    .body = bigclock_body,
};
