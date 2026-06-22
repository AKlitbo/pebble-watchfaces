/**
 * @file layout.c
 * @brief stardate-emery face: the zone table plus the WatchfaceDescriptor hooks that wire
 * the shell to this face's fonts, baked LCARS frame, and painted overlays.
 */
#include "layout.h"

#include "fonts.h"
#include "zone.h"
#include "theme/theme.h"
#include "widgets/widgets.h"

// --- Zone table ---
// presentation for every slot: geometry from the SLOT_* map, font by registry id,
// alignment, and colour. Readouts are white on the black field; the lat/lon
// coordinates sit on the coloured left-rail blocks, so they're black like the numerals
static const Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,     .font_id = FONT_TIME,  .align = GTextAlignmentCenter, .color = GColorWhite},
    [ZONE_MERIDIEM] = {.rect = SLOT_MERIDIEM, .font_id = FONT_XS,    .align = GTextAlignmentRight,  .color = GColorWhite},
    [ZONE_DATE]     = {.rect = SLOT_BANNER,   .font_id = FONT_DATE,  .align = GTextAlignmentCenter, .color = GColorWhite,
                       .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_BANNER_SM,
                       .font_id_fallback2 = FONT_DATE_XS, .rect_fallback2 = SLOT_BANNER_XS},
    [ZONE_WEATHER]  = {.rect = SLOT_WEATHER,  .font_id = FONT_SM,    .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_COND]     = {.rect = SLOT_COND,     .font_id = FONT_VALUE, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_HR]       = {.rect = SLOT_HR,       .font_id = FONT_VALUE, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS,    .font_id = FONT_VALUE, .align = GTextAlignmentLeft,   .color = GColorWhite},
    [ZONE_LAT]      = {.rect = SLOT_LAT,      .font_id = FONT_COORD, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
    [ZONE_LON]      = {.rect = SLOT_LON,      .font_id = FONT_COORD, .align = GTextAlignmentRight,  .color = COORD_TEXT_COLOR},
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

// --- Fonts ---
/**
 * @brief Antonio at the sizes each slot needs, registered under their category ids.
 */
static void load_fonts(void)
{
    fonts_register(FONT_TIME,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_62)));
    fonts_register(FONT_DATE,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_36)));
    fonts_register(FONT_DATE_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_32)));
    fonts_register(FONT_DATE_XS, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_28)));
    fonts_register(FONT_SM,      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_20)));
    fonts_register(FONT_VALUE,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_16)));
    fonts_register(FONT_COORD,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_12)));
    fonts_register(FONT_XS,      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10)));
}

// --- Baked LCARS frame ---
static BitmapLayer *s_frame_layer;
static GBitmap *s_frame_bitmap;
// 0xFF = nothing loaded yet (forces first load)
static uint8_t s_loaded_theme = 0xFF;

/**
 * @brief (Re)load the baked frame for a theme.
 *
 * No-op when that theme is already loaded, so it's cheap to call on every
 * settings save.
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
    s_frame_layer = bitmap_layer_create(layer_get_bounds(parent));
    layer_add_child(parent, bitmap_layer_get_layer(s_frame_layer));
    load_frame(theme);

    widgets_create(parent);
}

/**
 * @brief Update the loaded theme.
 *
 * @param theme The new theme setting value.
 */
static void update_theme(uint8_t theme)
{
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

const WatchfaceDescriptor *stardate_emery_face(void)
{
    return &s_face;
}
