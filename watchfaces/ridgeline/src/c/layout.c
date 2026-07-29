/**
 * @file layout.c
 * @brief Ridgeline face: the zone table bound to shared readouts, the drawn scene, and the
 * chrome draw-slot. No shell. The face drives the engine directly from main.
 *
 * Everything visual comes out of one palette, and which palette that is depends on the theme
 * *and* on whether the sun is up, so the zone colours are re-set whenever either moves.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "scene/scene.h"
#include "theme/theme.h"
#include "widgets/widgets.h"
#include "io/stores/system_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

// the palette everything is drawn from, and the daylight it was chosen for. held rather than
// looked up per draw so the scene, the chrome, and the text can't disagree inside one frame
static const Palette *s_pal;
static bool s_night;

// --- Zone Table ---
// the clock over the mountains with the date beneath it and the stats row along the bottom. the
// colours here are placeholders: apply_theme_colors re-sets every one from the live palette
// before the first paint
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME, .font_id = FONT_TIME, .align = GTextAlignmentCenter, .color = GColorBlack,
                       // the @beats token is wider than HH:MM and overflows Hand 72
                       .font_id_fallback = FONT_TIME_SM, .rect_fallback = SLOT_TIME_SM},
    [ZONE_DATE]     = {.rect = SLOT_DATE,  .font_id = FONT_DATE,     .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_DATE_SM, .rect_fallback = SLOT_DATE},
    [ZONE_WEATHER]  = {.rect = SLOT_TEMP,  .font_id = FONT_VALUE,    .align = GTextAlignmentLeft, .color = GColorBlack,
                       .font_id_fallback = FONT_XS, .rect_fallback = SLOT_TEMP_SM},
    [ZONE_HR]       = {.rect = SLOT_HR,    .font_id = FONT_VALUE,    .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_XS, .rect_fallback = SLOT_HR_SM},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS, .font_id = FONT_VALUE,    .align = GTextAlignmentRight, .color = GColorBlack,
                       .font_id_fallback = FONT_XS, .rect_fallback = SLOT_STEPS_SM},
};

/**
 * @brief Point the palette at the current theme and daylight, and colour the zones from it.
 *
 * The clock sits on the land and everything else sits over the sky, which is why the clock
 * takes `text` and the rest take `dim`.
 */
static void apply_theme_colors(void)
{
    s_night = scene_night();
    s_pal = palette_for(settings_u8(SETTING_THEME), s_night);

    s_zones[ZONE_TIME].color = s_pal->text;
    s_zones[ZONE_DATE].color = s_pal->dim;
    s_zones[ZONE_WEATHER].color = s_pal->dim;
    s_zones[ZONE_HR].color = s_pal->dim;
    s_zones[ZONE_STEPS].color = s_pal->dim;
}

/**
 * @brief Patrick Hand at the sizes each slot needs, registered under their role ids.
 */
static void load_fonts(void)
{
    fonts_register(FONT_TIME,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_72)));
    fonts_register(FONT_TIME_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_64)));
    fonts_register(FONT_DATE,    fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_22)));
    fonts_register(FONT_DATE_SM, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_18)));
    fonts_register(FONT_VALUE,   fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_18)));
    fonts_register(FONT_XS,      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_16)));
}

/**
 * @brief Scene draw-slot: the whole painted background, from sky to weather.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_scene(GContext *ctx, GRect bounds, const void *data)
{
    scene_draw(ctx, bounds, s_pal);
}

/**
 * @brief Chrome draw-slot: the battery gauge, the AM/PM marker, the stats-row glyphs, and the
 * bluetooth and Quiet Time status icons.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_chrome(GContext *ctx, GRect bounds, const void *data)
{
    uint8_t theme = settings_u8(SETTING_THEME);

    widgets_draw_top_bar(ctx, bounds);
    widgets_draw_battery(ctx, s_pal->bar_ink, theme, system_store_battery());
    widgets_draw_stat_glyphs(ctx, s_pal);
    widgets_draw_meridiem(ctx, s_pal->dim);

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH), s_pal->bar_ink);
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH_SLASH), s_pal->bar_ink);
        widgets_draw_bt(ctx, system_store_bluetooth());
    }

    // the muted speaker only shows while Quiet Time is actually holding, so an empty slot
    // beside bluetooth is the normal state
    if (settings_u8(SETTING_QUIET_TIME_ICON) && quiet_time_is_active())
    {
        widgets_draw_qt(ctx, s_pal->bar_ink);
    }
}

void ridgeline_setup(Window *window)
{
    apply_theme_colors();
    load_fonts();
    scene_init();

    // the scene paints the whole window, so nothing of this ever shows. it just stops a
    // half-built frame flashing white on the way in
    window_set_background_color(window, GColorBlack);
}

uint8_t ridgeline_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    // the scene is the background, so it goes down first and the chrome sits on it
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_scene};
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_chrome};

    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_TIME],     .text = readout_time};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_DATE],     .text = readout_date};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER],  .text = readout_weather_temp};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_HR],       .text = readout_hr};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_STEPS],    .text = readout_steps};

    return i;
}

void ridgeline_apply_theme(void)
{
    apply_theme_colors();
}

bool ridgeline_daylight_changed(void)
{
    bool was_night = s_night;
    apply_theme_colors();

    return s_night != was_night;
}

void ridgeline_teardown(void)
{
    icons_cleanup();
    scene_deinit();
    fonts_unload_all();
}
