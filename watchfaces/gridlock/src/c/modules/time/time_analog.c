#include "engine/grid_engine.h"
#include "time_analog.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include "ui/icon_cache.h"
#include <pebble.h>

// ---------------------------------------------------------------------------
// analog clock
//
// the dial chrome is a baked plate: one 2-bit palettized png per style and tile height, kept in
// the resource pack (free) instead of drawn live in code. draw_analog blits the plate, recolours
// it to the theme by swapping the palette, then draws the hands over it. the plates carry
// transparent, accent, subtitle, and value in palette slots 0..3, so one plate follows mono
// vibrant and custom just by rewriting slots 1..3, the same trick the header checker uses.
// ---------------------------------------------------------------------------

// the plate resource ids run contiguously in pebble.appinfo.json media order, TALL then SHORT for
// each style, so the id is plain arithmetic and needs no lookup table (a table would cost bytes)
_Static_assert(RESOURCE_ID_ANALOG_S8_SHORT - RESOURCE_ID_ANALOG_S0_TALL == 17,
               "analog plates must stay contiguous, TALL then SHORT per style");

/**
 * @brief The hour and minute hands plus the centre hub in the value colour. reach is the
 * minute-hand length. The hour hand is 6/10 of it, the same proportion every dial uses.
 */
static void analog_hands(GridCtx *gctx, GPoint c, int reach, int tail)
{
    const struct tm *t = time_store_tm();
    int32_t min_a = (t->tm_min * TRIG_MAX_ANGLE) / 60;
    int32_t hour_a = (((t->tm_hour % 12) * 60 + t->tm_min) * TRIG_MAX_ANGLE) / (12 * 60);

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_value);

    // minute hand: long and thin. tail is a counterweight behind the centre (0 for no tail)
    graphics_context_set_stroke_width(gctx->ctx, 2);
    graphics_draw_line(gctx->ctx,
        (GPoint){ c.x - sin_lookup(min_a) * tail / TRIG_MAX_RATIO, c.y + cos_lookup(min_a) * tail / TRIG_MAX_RATIO },
        (GPoint){ c.x + sin_lookup(min_a) * reach / TRIG_MAX_RATIO, c.y - cos_lookup(min_a) * reach / TRIG_MAX_RATIO });

    // hour hand: short and thick
    graphics_context_set_stroke_width(gctx->ctx, 3);
    int hl = reach * 6 / 10;
    graphics_draw_line(gctx->ctx,
        (GPoint){ c.x - sin_lookup(hour_a) * tail / TRIG_MAX_RATIO, c.y + cos_lookup(hour_a) * tail / TRIG_MAX_RATIO },
        (GPoint){ c.x + sin_lookup(hour_a) * hl / TRIG_MAX_RATIO, c.y - cos_lookup(hour_a) * hl / TRIG_MAX_RATIO });

    graphics_context_set_fill_color(gctx->ctx, gctx->color_value);
    graphics_fill_circle(gctx->ctx, c, 2);

    // reset so a later slot painted with this context is not left thick
    graphics_context_set_stroke_width(gctx->ctx, 1);
}

/**
 * @brief Draws the analog face by blitting its baked plate and laying the hands over it. tall
 * picks the full-tile plate (headerless) or the shorter body plate (under a header). The plate is
 * recoloured to the theme through its palette, so the panel follows mono, vibrant, and custom.
 */
static void draw_analog(GridCtx *gctx, GRect area, bool tall)
{
    uint32_t res = RESOURCE_ID_ANALOG_S0_TALL + gridlock_analog_style() * 2 + (tall ? 0 : 1);
    GBitmap *plate = icon_get(res);
    if (plate)
    {
        GColor *pal = gbitmap_get_palette(plate);
        if (pal)
        {
            // slot 0 stays transparent (GCompOpSet leaves it see-through)
            // so we only recolour the three inks
            pal[1] = gctx->color_accent;
            pal[2] = gctx->color_subtitle;
            pal[3] = gctx->color_value;
        }
        graphics_context_set_compositing_mode(gctx->ctx, GCompOpSet);
        graphics_draw_bitmap_in_rect(gctx->ctx, plate, area);
    }

    // the same hands sit on every plate: centred, length taken from the tile's short side
    GPoint c = GPoint(area.origin.x + area.size.w / 2, area.origin.y + area.size.h / 2);
    int reach = (area.size.h < area.size.w ? area.size.h : area.size.w) / 2 - 5;
    analog_hands(gctx, c, reach, 0);
}

// the engine widens gctx->body to the whole tile when the header is dropped, so the same rect
// serves both looks and only the plate has to change
static void time_analog_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_2x2)
    {
        draw_analog(gctx, gctx->body, gctx->headerless);
    }
}

const ModuleDef mod_time_analog_def = {
    .label = "CLOCK",
    .sizes = SZ_2x2,
    .features = FEATURE_TIME,
    .body = time_analog_body,
};
