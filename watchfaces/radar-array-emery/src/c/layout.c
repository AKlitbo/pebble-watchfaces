/**
 * @file layout.c
 * @brief radar-array-emery face: the zone table bound to shared readouts, the baked frame,
 * and the overlays draw-slot (labels + battery + bluetooth). No shell. The face drives the
 * engine directly from main.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "widgets/widgets.h"
#include "io/stores/system_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

// --- Zone table ---
// every live slot: geometry from the SLOT_* map plus font by registry id plus alignment
// and colour. colours are seeded with the Default palette and re-set per theme by
// apply_theme_colors: readouts take the primary phosphor and the date/meridiem the chrome accent
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,     .font_id = FONT_TIME,  .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_MERIDIEM] = {.rect = SLOT_MERIDIEM, .font_id = FONT_XS,    .align = GTextAlignmentCenter, .color = GColorChromeYellow},
    [ZONE_DATE]     = {.rect = SLOT_BANNER,   .font_id = FONT_DATE,  .align = GTextAlignmentCenter, .color = GColorChromeYellow,
                       .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_BANNER_SM},
    [ZONE_WEATHER]  = {.rect = SLOT_WEATHER,  .font_id = FONT_SM,    .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_HR]       = {.rect = SLOT_HR,       .font_id = FONT_VALUE, .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS,    .font_id = FONT_VALUE, .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_LAT]      = {.rect = SLOT_COORD,    .font_id = FONT_COORD, .align = GTextAlignmentCenter, .color = COORD_TEXT_COLOR},
    // ZONE_COND and ZONE_LON are omitted: no condition glyph and the coordinate readout is
    // a single combined lat/lon string the phone sends in ZONE_LAT
};

/**
 * @brief Set the zone text colours to the current theme's phosphor and accent.
 *
 * @param theme The theme setting value.
 */
static void apply_theme_colors(uint8_t theme)
{
    GColor primary = primary_color_for_theme(theme);
    GColor accent = panel_accent_for_theme(theme);

    s_zones[ZONE_TIME].color = primary;
    s_zones[ZONE_WEATHER].color = primary;
    s_zones[ZONE_HR].color = primary;
    s_zones[ZONE_STEPS].color = primary;
    s_zones[ZONE_LAT].color = primary;
    s_zones[ZONE_MERIDIEM].color = accent;
    s_zones[ZONE_DATE].color = accent;
}

static void load_fonts(void)
{
    fonts_register(FONT_TIME,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_40)));
    fonts_register(FONT_DATE,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_18)));
    fonts_register(FONT_DATE_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_17)));
    fonts_register(FONT_SM,      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_18)));
    fonts_register(FONT_VALUE,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_18)));
    fonts_register(FONT_COORD,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_14)));
    fonts_register(FONT_XS,      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_12)));
}

// --- Baked radar-array frame ---
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
 * @brief Overlays draw-slot: the static labels, the battery gauge, and the bluetooth glyph.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_overlays(GContext *ctx, GRect bounds, const void *data)
{
    widgets_draw_labels(ctx);
    widgets_draw_battery(ctx, system_store_battery());

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        widgets_draw_bt(ctx, system_store_bluetooth());
    }
}

void radar_setup(Window *window)
{
    uint8_t theme = settings_u8(SETTING_THEME);
    apply_theme_colors(theme);
    load_fonts();

    Layer *root = window_get_root_layer(window);
    window_set_background_color(window, GColorBlack);

    // the frame bitmap sits at the bottom under the engine's slot layers
    s_frame_layer = bitmap_layer_create(layer_get_bounds(root));
    layer_add_child(root, bitmap_layer_get_layer(s_frame_layer));
    load_frame(theme);
}

uint8_t radar_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    // overlays first so they sit under the text readouts
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_overlays};

    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_TIME],     .text = readout_time};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_MERIDIEM], .text = readout_meridiem};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_DATE],     .text = readout_date};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER],  .text = readout_weather_temp};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_HR],       .text = readout_hr};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_STEPS],    .text = readout_steps};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_LAT],      .text = readout_lat};

    return i;
}

void radar_apply_theme(void)
{
    uint8_t theme = settings_u8(SETTING_THEME);
    apply_theme_colors(theme);
    load_frame(theme);
}

void radar_teardown(void)
{
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
