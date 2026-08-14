/**
 * @file layout.c
 * @brief stardate-emery face: the zone table bound to shared readouts, the baked LCARS
 * frame, and the overlays draw-slot (battery + labels + glyphs). No shell. The face drives
 * the engine directly from main.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "dev/dev.h"
#include "draw/fonts.h"
#include "ops/ops.h"
#include "settings_schema.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "widgets/widgets.h"
#include "io/stores/system_store.h"
#include "io/stores/weather_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

// one ops slot's Zone in its normal and no-glyph widths. a catalog entry can hand
// a slot a seven-digit julian day or a phase name like WAN CRES. so each one steps
// down through Antonio 14 to 12 before it would clip
#define OPS_ZONE(x, y, r)                                                    \
    {.rect = OPS_VALUE((x), (y), (r), 0), .font_id = FONT_ANTONIO_16,        \
     .align = GTextAlignmentLeft, .color = GColorWhite,                      \
     .font_id_fallback = FONT_ANTONIO_14,                                    \
     .rect_fallback = OPS_VALUE((x), (y), (r), OPS_DY_FB),                   \
     .font_id_fallback2 = FONT_ANTONIO_12,                                   \
     .rect_fallback2 = OPS_VALUE((x), (y), (r), OPS_DY_FB2)}

#define OPS_ZONE_WIDE(x, y, r)                                               \
    {.rect = OPS_VALUE_WIDE((x), (y), (r), 0), .font_id = FONT_ANTONIO_16,   \
     .align = GTextAlignmentLeft, .color = GColorWhite,                      \
     .font_id_fallback = FONT_ANTONIO_14,                                    \
     .rect_fallback = OPS_VALUE_WIDE((x), (y), (r), OPS_DY_FB),              \
     .font_id_fallback2 = FONT_ANTONIO_12,                                   \
     .rect_fallback2 = OPS_VALUE_WIDE((x), (y), (r), OPS_DY_FB2)}

// --- Zone Table ---
// presentation for every slot: geometry from the SLOT_* map plus font by registry id
// plus alignment and colour. Readouts are white on the black field. the lat/lon coordinates
// sit on the coloured left-rail blocks so they're black like the numerals
static const Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,     .font_id = FONT_ANTONIO_62, .align = GTextAlignmentCenter, .color = GColorWhite},
    [ZONE_MERIDIEM] = {.rect = SLOT_MERIDIEM, .font_id = FONT_ANTONIO_10, .align = GTextAlignmentRight,  .color = GColorWhite},
    [ZONE_DATE]     = {.rect = SLOT_BANNER,   .font_id = FONT_ANTONIO_36, .align = GTextAlignmentCenter, .color = GColorWhite,
                       .font_id_fallback = FONT_ANTONIO_32, .rect_fallback = SLOT_BANNER_SM,
                       .font_id_fallback2 = FONT_ANTONIO_28, .rect_fallback2 = SLOT_BANNER_XS},
    [ZONE_WEATHER]  = {.rect = SLOT_WEATHER,  .font_id = FONT_ANTONIO_20, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_COND]     = {.rect = SLOT_COND,     .font_id = FONT_ANTONIO_16, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_LT]       = OPS_ZONE(OPS_COL_L, OPS_ROW_T, OPS_R_L),
    [ZONE_LT_WIDE]  = OPS_ZONE_WIDE(OPS_COL_L, OPS_ROW_T, OPS_R_L),
    [ZONE_LB]       = OPS_ZONE(OPS_COL_L, OPS_ROW_B, OPS_R_L),
    [ZONE_LB_WIDE]  = OPS_ZONE_WIDE(OPS_COL_L, OPS_ROW_B, OPS_R_L),
    [ZONE_RT]       = OPS_ZONE(OPS_COL_R, OPS_ROW_T, OPS_R_R),
    [ZONE_RT_WIDE]  = OPS_ZONE_WIDE(OPS_COL_R, OPS_ROW_T, OPS_R_R),
    [ZONE_RB]       = OPS_ZONE(OPS_COL_R, OPS_ROW_B, OPS_R_R),
    [ZONE_RB_WIDE]  = OPS_ZONE_WIDE(OPS_COL_R, OPS_ROW_B, OPS_R_R),
    [ZONE_LAT]      = {.rect = SLOT_LAT,      .font_id = FONT_ANTONIO_12, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
    [ZONE_LON]      = {.rect = SLOT_LON,      .font_id = FONT_ANTONIO_12, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
};

// --- Ops slots ---
// the pick is read fresh on every paint rather than cached, so a settings change lands on the
// next rebuild with nothing to invalidate. dev_ops_pick hands back the saved value in any build
// that is not running the screenshot walk
//
// the slots are numbered in the order the walk steps them
// left top then left bottom then right top then right bottom
enum { SLOT_LT, SLOT_LB, SLOT_RT, SLOT_RB };

/**
 * @brief What a slot is showing.
 *
 * A tall pick is only honoured in the upper left, the one place the face draws it. Anywhere else
 * it reads as empty rather than as a labelled panel with nothing under it.
 *
 * @param slot One of the SLOT_* ids.
 * @return An OpsId.
 */
static uint8_t pick(int slot)
{
    static uint8_t (*const stored[])(void) = {
        lcars_slot_lt, lcars_slot_lb, lcars_slot_rt, lcars_slot_rb,
    };

    uint8_t id = dev_ops_pick(slot, stored[slot]());

    if (ops_is_tall(id) && slot != SLOT_LT)
    {
        return OPS_EMPTY;
    }

    return id;
}

/** @brief Whether the left column is showing the one tall weather block instead of two slots. */
static bool left_is_composite(void)
{
    return ops_is_tall(pick(SLOT_LT));
}

bool stardate_ops_shows_beats(void)
{
    if (pick(SLOT_RT) == OPS_BEATS || pick(SLOT_RB) == OPS_BEATS)
    {
        return true;
    }

    // the tall block fills the left column, so neither left pick is drawn and neither one
    // needs the ticker
    if (left_is_composite())
    {
        return false;
    }

    return pick(SLOT_LT) == OPS_BEATS || pick(SLOT_LB) == OPS_BEATS;
}

/**
 * @brief The zone a slot draws in: the narrow one beside a glyph, or the wide one that takes the
 * glyph's space back when the readout has none.
 *
 * stardate_build runs again on every engine_rebuild, and a slot only changes on a settings
 * change, which rebuilds. So picking the zone here is enough and nothing has to swap it later.
 *
 * @param slot One of the SLOT_* ids.
 * @return The zone to bind.
 */
static const Zone *ops_zone(int slot)
{
    static const uint8_t narrow[] = {ZONE_LT, ZONE_LB, ZONE_RT, ZONE_RB};
    static const uint8_t wide[] = {ZONE_LT_WIDE, ZONE_LB_WIDE, ZONE_RT_WIDE, ZONE_RB_WIDE};

    bool has_icon = ops_icon(ops_entry(pick(slot))) != 0;
    return &s_zones[has_icon ? narrow[slot] : wide[slot]];
}

// one text formatter per slot. the engine's text-slot signature carries no
// argument so the slot each one reads is baked in rather than passed
static void lt_text(char *out, size_t n) { ops_text(ops_entry(pick(SLOT_LT)), out, n); }
static void lb_text(char *out, size_t n) { ops_text(ops_entry(pick(SLOT_LB)), out, n); }
static void rt_text(char *out, size_t n) { ops_text(ops_entry(pick(SLOT_RT)), out, n); }
static void rb_text(char *out, size_t n) { ops_text(ops_entry(pick(SLOT_RB)), out, n); }

/**
 * @brief Antonio at the sizes each slot needs, registered under their category ids.
 */
static void load_fonts(void)
{
    fonts_register(FONT_ANTONIO_62, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_62)));
    fonts_register(FONT_ANTONIO_36, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_36)));
    fonts_register(FONT_ANTONIO_32, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_32)));
    fonts_register(FONT_ANTONIO_28, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_28)));
    fonts_register(FONT_ANTONIO_20, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_20)));
    fonts_register(FONT_ANTONIO_16, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_16)));
    fonts_register(FONT_ANTONIO_14, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_14)));
    fonts_register(FONT_ANTONIO_12, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_12)));
    fonts_register(FONT_ANTONIO_10, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10)));
}

// --- Baked LCARS frame ---
static BitmapLayer *s_frame_layer;
static GBitmap *s_frame_bitmap;
static uint8_t s_loaded_theme = 0xFF;  // 0xFF = nothing loaded yet (forces first load)

/**
 * @brief (Re)load the baked frame for a theme.
 *
 * @param theme The theme setting value.
 */
static void load_frame(uint8_t theme)
{
    if (theme == s_loaded_theme && s_frame_bitmap)
    {
        return;
    }

    s_loaded_theme = theme;

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    s_frame_bitmap = gbitmap_create_with_resource(bg_resource_for_theme(theme));

    if (s_frame_layer)
    {
        bitmap_layer_set_bitmap(s_frame_layer, s_frame_bitmap);
    }
}

/**
 * @brief Overlays draw-slot: the battery gauge, the bar labels, and the glyphs.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_overlays(GContext *ctx, GRect bounds, const void *data)
{
    // the bar word and the glyph both follow each slot's pick so they get resolved together here
    // and handed down. widgets stays a painter that is told what to draw
    OpsChrome chrome = {.left_composite = left_is_composite()};

    for (int slot = SLOT_LT; slot <= SLOT_RB; slot++)
    {
        const OpsReadout *entry = ops_entry(pick(slot));
        chrome.label[slot] = ops_label(entry);
        chrome.icon[slot] = ops_icon(entry);
    }

    // bars first: the label holder boxes are painted over the top of them
    widgets_draw_bars(ctx, &chrome);
    widgets_draw_battery(ctx, system_store_battery());
    widgets_draw_labels(ctx, &chrome);

    // honour the show/hide settings. the store owns the connect/disconnect vibe, and the
    // quiet-time glyph reads the live SDK state (no change event, so it refreshes on the tick)
    bool bt_show = settings_u8(SETTING_BLUETOOTH_ICON);
    bool qt_show = settings_u8(SETTING_QUIET_TIME_ICON);
    widgets_draw_glyphs(ctx, &chrome, weather_store_cond(),
                        bt_show, system_store_bluetooth(), qt_show, quiet_time_is_active());
}

void stardate_setup(Window *window)
{
    load_fonts();

    Layer *root = window_get_root_layer(window);
    window_set_background_color(window, GColorBlack);

    // the frame bitmap sits at the bottom under the engine's slot layers
    s_frame_layer = bitmap_layer_create(layer_get_bounds(root));
    layer_add_child(root, bitmap_layer_get_layer(s_frame_layer));
    load_frame(settings_u8(SETTING_THEME));

    widgets_load();
}

uint8_t stardate_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    // overlays first so they sit under the text readouts
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_overlays};

    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_TIME],     .text = readout_time};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_MERIDIEM], .text = readout_meridiem};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_DATE],     .text = readout_date};

    // the left column is either the one tall weather block or two slots and never
    // both. either way it costs two text layers so the whole face stays inside
    // ENGINE_MAX_SLOTS
    if (left_is_composite())
    {
        out[i++] = (EngineSlot){.zone = &s_zones[ZONE_COND],    .text = readout_weather_cond};
        out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER], .text = readout_weather_temp};
    }
    else
    {
        out[i++] = (EngineSlot){.zone = ops_zone(SLOT_LT), .text = lt_text};
        out[i++] = (EngineSlot){.zone = ops_zone(SLOT_LB), .text = lb_text};
    }

    out[i++] = (EngineSlot){.zone = ops_zone(SLOT_RT),       .text = rt_text};
    out[i++] = (EngineSlot){.zone = ops_zone(SLOT_RB),       .text = rb_text};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_LAT],      .text = readout_lat};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_LON],      .text = readout_lon};

    return i;
}

void stardate_apply_theme(void)
{
    load_frame(settings_u8(SETTING_THEME));
}

void stardate_teardown(void)
{
    widgets_unload();
    icons_cleanup();
    bitmap_layer_destroy(s_frame_layer);
    s_frame_layer = NULL;

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    fonts_unload_all();
    s_loaded_theme = 0xFF;  // force a reload if the window is recreated
}
