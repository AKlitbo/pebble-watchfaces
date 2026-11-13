/**
 * @file system_battery_pair.c
 * @brief The watch and phone batteries in one 1x2, split down the middle: a watch glyph and its level
 * on the left and a phone glyph and its level on the right, with no percent signs so both fit.
 *
 * The panel takes the battery panel's colours, and the phone's half takes the phone battery panel's,
 * so in Vibrant each reading wears the same colour it has on its own panel. The watch always has a
 * reading. The phone's comes from the Pebble app's Phone plugin, so only its half drops, to OFF while
 * plugins are off and to -- while the phone is away.
 *
 * It sits in this face rather than the family core because the phone's colours come from the grid
 * engine's grid_ctx_use_colors_of, which Sidereel's engine does not have.
 *
 * @ingroup gridlock_mod_system
 */
#include "engine/grid_engine.h"
#include "system_battery_pair.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/icons.h"
#include "mosaic/modules/system/system_battery.h"
#include "math/scale.h"
#include "text/number_format.h"

#include "io/stores/phone_store.h"
#include "io/stores/system_store.h"
#include "ui/fonts.h"

#define GLYPH_W 8    ///< both glyphs' width, the size their icons are rendered at
#define GLYPH_H 14   ///< both glyphs' height
#define GLYPH_GAP 3  ///< between a glyph and its number

/**
 * @brief The watch glyph, a smartwatch with a charging bolt, in the panel's icon colour.
 *
 * @param gctx The grid context, for the icon colour.
 * @param slot The glyph's GLYPH_W by GLYPH_H box.
 */
static void watch_glyph(GridCtx *gctx, GRect slot)
{
    icon_draw_rect(gctx, RESOURCE_ID_ICON_SYSTEM_WATCH_CHARGE, slot);
}

/**
 * @brief The phone glyph, a phone with a charge mark, in the panel's icon colour.
 *
 * @param gctx The grid context, for the icon colour.
 * @param slot The glyph's GLYPH_W by GLYPH_H box.
 */
static void phone_glyph(GridCtx *gctx, GRect slot)
{
    icon_draw_rect(gctx, RESOURCE_ID_ICON_SYSTEM_PHONE_CHARGE, slot);
}

/**
 * @brief One half of the 1x2: a glyph and its number, centred as a pair in their half of the body.
 *
 * @param gctx The grid context, carrying this half's colours.
 * @param x0 Where the half starts, from the body's left edge.
 * @param w How wide the half is.
 * @param glyph Draws the glyph.
 * @param text The number.
 * @param font The number's font.
 */
static void pair_half(GridCtx *gctx, int x0, int w, void (*glyph)(GridCtx *, GRect), const char *text,
                      FontId font)
{
    int tw = battery_text_w(text, font);
    int left = x0 + (w - (GLYPH_W + GLYPH_GAP + tw)) / 2;

    GRect slot = GRect(gctx->body.origin.x + left, gctx->body.origin.y + (gctx->body.size.h - GLYPH_H) / 2,
                       GLYPH_W, GLYPH_H);
    glyph(gctx, slot);

    gh_value_left(gctx, text, font, tw + 2, BATTERY_VALUE_H, left + GLYPH_W + GLYPH_GAP);
}

/**
 * @brief The 1x2: the watch's half on the left, the phone's on the right, and a rule between them.
 *
 * @param gctx The grid context, in the tile's own colours.
 * @param phone A copy wearing the phone battery panel's colours.
 * @param watch_text The watch's number.
 * @param phone_text The phone's number, or -- with no reading.
 */
static void pair_1x2(GridCtx *gctx, GridCtx *phone, const char *watch_text, const char *phone_text)
{
    int half = gctx->body.size.w / 2;

    // Teko 26 keeps both numbers as big as the other 1x2 values. a full battery's three digits can
    // overrun its half, and then both drop to Teko 22 so the pair stays the same size
    int widest = battery_text_w(watch_text, FONT_TEKO_26);
    int phone_w = battery_text_w(phone_text, FONT_TEKO_26);
    if (phone_w > widest)
    {
        widest = phone_w;
    }
    FontId font = (GLYPH_W + GLYPH_GAP + widest > half - 4) ? FONT_TEKO_22 : FONT_TEKO_26;

    pair_half(gctx, 0, half, watch_glyph, watch_text, font);
    pair_half(phone, half, gctx->body.size.w - half, phone_glyph, phone_text, font);

    // a short rule between the halves, in the tile's own caption colour
    int x = gctx->body.origin.x + half;
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_line(gctx->ctx, GPoint(x, gctx->body.origin.y + 4),
                       GPoint(x, gctx->body.origin.y + gctx->body.size.h - 5));
}

// TODO: re-tune for the taller body when gctx->headerless
static void system_battery_pair_body(GridCtx *gctx)
{
    int watch_level = clamp_int(system_store_battery(), 0, 100);
    int phone_level = phone_store_battery();

    char watch_text[8];
    char phone_text[8];
    // no percent signs, so both numbers fit side by side. the phone store already holds its level
    // to 0 to 100, or -1, which comes out as --
    fmt_int_or_dash(watch_text, sizeof(watch_text), watch_level, "%d");
    fmt_int_or_dash(phone_text, sizeof(phone_text), phone_level, "%d");

    // the phone's half is drawn from a copy wearing the phone battery panel's colours
    GridCtx phone = *gctx;
    grid_ctx_use_colors_of(&phone, MOD_SYSTEM_PHONE_BATTERY);

    // PLUGINS OFF has no room in a half, so the phone's half says OFF beside its glyph
    pair_1x2(gctx, &phone, watch_text, phone_store_no_plugin() ? "OFF" : phone_text);
}

const ModuleDef mod_system_battery_pair_def = {
    .label = "BATTERY",
    .sizes = SZ_1x2,
    .features = FEATURE_SYSTEM | FEATURE_PHONE,
    // rides under the battery module's colours for customization, as Status does
    .theme_alias = MOD_SYSTEM_BATTERY,
    .body = system_battery_pair_body
};
