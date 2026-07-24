/**
 * @file layout.c
 * @brief stardate-emery face: the zone table bound to shared readouts, the baked LCARS
 * frame, and the overlays draw-slot (battery + labels + glyphs). No shell. The face drives
 * the engine directly from main.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "widgets/widgets.h"
#include "io/stores/system_store.h"
#include "io/stores/weather_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

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
    [ZONE_HR]       = {.rect = SLOT_HR,       .font_id = FONT_ANTONIO_16, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS,    .font_id = FONT_ANTONIO_16, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_LAT]      = {.rect = SLOT_LAT,      .font_id = FONT_ANTONIO_12, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
    [ZONE_LON]      = {.rect = SLOT_LON,      .font_id = FONT_ANTONIO_12, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
};

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
    widgets_draw_battery(ctx, system_store_battery());
    widgets_draw_labels(ctx);

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    bool bt_show = settings_u8(SETTING_BLUETOOTH_ICON);
    widgets_draw_glyphs(ctx, weather_store_cond(), bt_show, system_store_bluetooth());
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
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER],  .text = readout_weather_temp};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_COND],     .text = readout_weather_cond};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_HR],       .text = readout_hr};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_STEPS],    .text = readout_steps};
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
