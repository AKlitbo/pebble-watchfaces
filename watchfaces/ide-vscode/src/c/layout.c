/**
 * @file layout.c
 * @brief ide-vscode-emery face: the zone table bound to shared readouts, the baked VS Code
 * frame, and the overlays draw-slot (battery + weather glyph + bluetooth). No shell. The
 * face drives the engine directly from main.
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
// the readouts: the hero clock with the date centred beneath it plus the terminal 2x2
// grid (weather condition / temp on the left and heart rate / steps on the right). colours
// are seeded with the Dark+ palette and re-set per theme by apply_theme_colors
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]    = {.rect = SLOT_TIME, .font_id = FONT_TIME,  .align = GTextAlignmentCenter, .color = GColorVividCerulean,
                      // the @beats token is wider than HH:MM and overflows Teko 78. drop to 72 so it fits
                      .font_id_fallback = FONT_TIME_SM, .rect_fallback = SLOT_TIME},
    [ZONE_DATE]    = {.rect = SLOT_DATE,  .font_id = FONT_DATE, .align = GTextAlignmentCenter, .color = GColorRajah,
                      .font_id_fallback = FONT_VALUE, .rect_fallback = SLOT_DATE},
    [ZONE_COND]    = {.rect = SLOT_COND,  .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorMediumAquamarine,
                      .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_COND},
    [ZONE_WEATHER] = {.rect = SLOT_TEMP,  .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorMediumAquamarine},
    [ZONE_HR]      = {.rect = SLOT_HR,    .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorRed},
    [ZONE_STEPS]   = {.rect = SLOT_STEPS, .font_id = FONT_VALUE, .align = GTextAlignmentLeft, .color = GColorCeleste},
};

/**
 * @brief Set the editor zone text colours to the current theme's palette.
 *
 * @param theme The theme setting value.
 */
static void apply_theme_colors(uint8_t theme)
{
    s_zones[ZONE_TIME].color    = time_color_for_theme(theme);
    s_zones[ZONE_DATE].color    = date_color_for_theme(theme);
    s_zones[ZONE_COND].color    = stats_color_for_theme(theme);
    s_zones[ZONE_WEATHER].color = stats_color_for_theme(theme);
    s_zones[ZONE_HR].color      = hr_color_for_theme(theme);
    s_zones[ZONE_STEPS].color   = steps_color_for_theme(theme);
}

/**
 * @brief The hero clock uses condensed Teko (tall + narrow). The terminal readouts use
 * Share Tech Mono at the sizes each slot needs.
 */
static void load_fonts(void)
{
    fonts_register(FONT_TIME,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_78)));
    fonts_register(FONT_TIME_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_72)));
    fonts_register(FONT_DATE,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_20)));
    fonts_register(FONT_VALUE,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_14)));
    fonts_register(FONT_DATE_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_12)));
}

// --- Baked VS Code frame ---
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
 * @brief Overlays draw-slot: the battery, the weather condition glyph, and the bluetooth glyph.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_overlays(GContext *ctx, GRect bounds, const void *data)
{
    widgets_draw_battery(ctx, system_store_battery());
    widgets_draw_weather(ctx, weather_store_cond());

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        widgets_draw_bt(ctx, system_store_bluetooth());
    }
}

void vscode_setup(Window *window)
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

uint8_t vscode_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    // overlays first so they sit under the text readouts
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_overlays};

    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_TIME],    .text = readout_time};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_DATE],    .text = readout_date};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_COND],    .text = readout_weather_cond};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER], .text = readout_weather_temp};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_HR],      .text = readout_hr};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_STEPS],   .text = readout_steps};

    return i;
}

void vscode_apply_theme(void)
{
    uint8_t theme = settings_u8(SETTING_THEME);
    apply_theme_colors(theme);
    load_frame(theme);
}

void vscode_teardown(void)
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
