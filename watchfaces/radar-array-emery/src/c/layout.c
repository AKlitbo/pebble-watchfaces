/**
 * @file layout.c
 * @brief radar-array-emery face: the zone table plus the WatchfaceDescriptor hooks that wire
 * the shell to this face's Share Tech Mono fonts, baked radar-array frame, and the
 * painted labels + battery gauge.
 */
#include "layout.h"

#include "fonts.h"
#include "zone.h"
#include "theme/theme.h"
#include "widgets/widgets.h"

// --- Zone table ---
// every live slot: geometry from the SLOT_* map, font by registry id, alignment,
// and colour. colours are seeded with the Default palette and re-set per theme by
// apply_theme_colors: readouts take the primary phosphor, the date/meridiem the
// chrome accent
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,     .font_id = FONT_TIME,  .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_MERIDIEM] = {.rect = SLOT_MERIDIEM, .font_id = FONT_XS,    .align = GTextAlignmentCenter, .color = GColorChromeYellow},
    [ZONE_DATE]     = {.rect = SLOT_BANNER,   .font_id = FONT_DATE,  .align = GTextAlignmentCenter, .color = GColorChromeYellow,
                       .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_BANNER_SM},
    [ZONE_WEATHER]  = {.rect = SLOT_WEATHER,  .font_id = FONT_SM,    .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_HR]       = {.rect = SLOT_HR,       .font_id = FONT_VALUE, .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS,    .font_id = FONT_VALUE, .align = GTextAlignmentCenter, .color = GColorGreen},
    [ZONE_LAT]      = {.rect = SLOT_COORD,    .font_id = FONT_COORD, .align = GTextAlignmentCenter, .color = COORD_TEXT_COLOR},
    // ZONE_COND and ZONE_LON are omitted (left zeroed): no condition glyph, and the
    // coordinate readout is a single combined lat/lon string in ZONE_LAT
};

/**
 * @brief Returns the face's zone table.
 *
 * @return A pointer to the face's static zone table.
 */
static const Zone *face_zones(void)
{
    return s_zones;
}

/**
 * @brief Set the zone text colors to the current theme's primary phosphor.
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
// Bitmap layer for the frame
static BitmapLayer *s_frame_layer;
// Bitmap for the frame
static GBitmap *s_frame_bitmap;
// 0xFF = nothing loaded yet (forces first load)
static uint8_t s_loaded_theme = 0xFF;

/**
 * @brief Load the baked frame for a theme.
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
 * @brief The frame bitmap sits at the bottom, the painted overlays just above it.
 *
 * @param parent The parent layer to add the scene to.
 * @param theme The theme setting value.
 */
static void create_scene(Layer *parent, uint8_t theme)
{
    apply_theme_colors(theme);

    s_frame_layer = bitmap_layer_create(layer_get_bounds(parent));
    layer_add_child(parent, bitmap_layer_get_layer(s_frame_layer));
    load_frame(theme);

    widgets_create(parent);
}

/**
 * @brief Swap the frame background and refresh the dynamic zone colors.
 *
 * @param theme The new theme setting value.
 */
static void update_theme(uint8_t theme)
{
    apply_theme_colors(theme);
    load_frame(theme);
}

/**
 * @brief Destroy the scene and unload fonts/bitmaps.
 */
static void destroy_scene(void)
{
    widgets_destroy();
    bitmap_layer_destroy(s_frame_layer);

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    fonts_unload_all();

    // force a reload if the window is recreated
    s_frame_layer = NULL;
    s_loaded_theme = 0xFF;
}

static const WatchfaceDescriptor s_face = {
    .zones = face_zones,
    .load_fonts = load_fonts,
    .create_scene = create_scene,
    .destroy_scene = destroy_scene,
    .update_theme = update_theme,
    .set_battery = widgets_set_battery,
    .set_weather_icon = widgets_set_weather_icon,
    .set_bluetooth = widgets_set_bluetooth,
    .refresh_overlays = widgets_mark_labels_dirty,
};

const WatchfaceDescriptor *radar_array_face(void)
{
    return &s_face;
}
