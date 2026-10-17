/**
 * @file engine.c
 * @brief The shared layout: palette, chrome, slot list and ticker response, the same for every
 * scene.
 *
 * One scene is up at a time, so the palette it was last themed with and the layout it belongs to
 * are held here between the public calls and the draw-slots the engine fires.
 *
 * @ingroup watchface-sketchbook
 */
#include "layout/engine.h"

#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "sky/sky.h"
#include "widgets/widgets.h"
#include "layout/chrome.h"
#include "theme/palette.h"
#include "theme/palette_active.h"
#include "settings_schema.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the scene being drawn, and the palette it was themed with. held rather than looked up per draw
// so the scene, the chrome, and the text can't disagree inside one frame
static const SceneLayout *s_lay;
static const void *s_pal;
static bool s_night;
// the theme index the palette was chosen for, so a Shuffle rollover or a day/night/Quiet swap that
// lands on a different one is noticed and repainted
static uint8_t s_theme;

/**
 * @brief Point the palette at the current theme and daylight, and colour the zones from it.
 *
 * Which theme is up depends on the palette mode: the fixed one, a Shuffle that rotates with the
 * clock, or a day/night/Quiet Time swap. Whichever it lands on still takes its day or night colours
 * from the actual daylight. The clock sits on the scene's ground and everything else sits over the
 * sky, which is why the clock takes `text` and the rest take `dim`.
 */
static void apply_theme_colors(void)
{
    s_night = sketchbook_sky_night();

    PalettePick pick;
    sketchbook_palette_pick(&pick);
    pick.night_on = s_night;
    pick.quiet_on = quiet_time_is_active();

    const struct tm *now = time_store_tm();
    s_theme = sketchbook_active_theme(&pick, now->tm_year, now->tm_yday, now->tm_hour, now->tm_min);

    s_pal = s_lay->palette_for_theme(s_theme, s_night);
    const CommonPalette *cp = s_pal;

    Zone *zones = s_lay->zones;
    zones[ZONE_TIME].color = cp->text;
    zones[ZONE_DATE].color = cp->dim;
    zones[ZONE_WEATHER].color = cp->dim;
    zones[ZONE_HR].color = cp->dim;
    zones[ZONE_STEPS].color = cp->dim;

    // the date-up layouts move it off the ground onto the strip, so it takes the strip's ink
    if (sketchbook_layout() != LAYOUT_STANDARD)
    {
        zones[ZONE_DATE].color = cp->bar_ink;
    }
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
    s_lay->scene_draw(ctx, bounds, s_pal);
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
    const CommonPalette *cp = s_pal;
    uint8_t theme = settings_u8(SETTING_THEME);

    widgets_draw_top_bar(ctx, bounds);

    // the date only earns its own strip on the layouts that move it up there
    if (sketchbook_layout() != LAYOUT_STANDARD)
    {
        widgets_draw_date_bar(ctx, bounds);
    }
    widgets_draw_battery(ctx, cp->bar_ink, theme, system_store_battery());

    // the row's marks go with the row, and only the big clock drops it
    if (sketchbook_layout() != LAYOUT_BIG_CLOCK)
    {
        widgets_draw_stat_glyphs(ctx, cp, &s_lay->zones[ZONE_WEATHER], &s_lay->zones[ZONE_STEPS]);
    }

    // measured onto the live clock, so it takes the same slot and font. a 24-hour clock has no
    // marker anyway, and the setting drops it on a 12-hour one
    if (sketchbook_show_meridiem())
    {
        GRect slot = s_lay->zones[ZONE_TIME].rect;
        FontId font = s_lay->zones[ZONE_TIME].font_id;

        // the standard clock has room either side, so the marker goes beside the digits. the
        // bigger two have none and drop it into the channel above the colon. on a round screen the
        // standard clock fills the width too, so a marker beside it lands hard against the bezel and
        // it joins the other two
        if (PBL_IF_ROUND_ELSE(false, sketchbook_layout() == LAYOUT_STANDARD))
        {
            widgets_draw_meridiem_beside(ctx, cp->dim, slot, font, s_lay->meridiem_beside_top);
        }
        else
        {
            widgets_draw_meridiem_above(ctx, cp->dim, slot, font, s_lay->meridiem_above_top());
        }
    }

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH), cp->bar_ink);
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH_SLASH), cp->bar_ink);
        widgets_draw_bt(ctx, system_store_bluetooth());
    }

    // the mark is drawn either way where the face bundles both states, so the strip keeps its
    // shape whether or not Quiet Time is holding. the setting still decides if it shows at all
    if (settings_u8(SETTING_QUIET_TIME_ICON))
    {
        widgets_draw_qt_state(ctx, cp->bar_ink, quiet_time_is_active());
    }
}

/**
 * @brief Patrick Hand at the sizes each slot needs, registered under their role ids.
 */
static void load_fonts(void)
{
    // one clock size per platform: the rectangle draws 92 on its roomier layouts, the circle
    // draws 82 on its only one, and loading both would hold a font in heap that never renders
#if defined(PBL_ROUND)
    fonts_register(FONT_HAND_82, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_82)));
#else
    fonts_register(FONT_HAND_92, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_92)));
#endif
    fonts_register(FONT_HAND_72, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_72)));
    fonts_register(FONT_HAND_64, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_64)));
    fonts_register(FONT_HAND_22, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_22)));
    fonts_register(FONT_HAND_18, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_18)));
    fonts_register(FONT_HAND_16, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HAND_16)));
}

void layout_setup(const SceneLayout *lay, Window *window)
{
    s_lay = lay;
    apply_theme_colors();
    load_fonts();
    if (lay->scene_init)
    {
        lay->scene_init();
    }

    // the scene paints the whole window, so nothing of this ever shows. it just stops a
    // half-built frame flashing white on the way in
    window_set_background_color(window, GColorBlack);
}

// a build writes up to seven slots without reading max, so the engine's cap has to hold them
_Static_assert(ENGINE_SLOTS_MAX >= 7, "a scene build writes up to seven slots");

uint8_t layout_build(const SceneLayout *lay, EngineSlot *out, uint8_t max, GRect bounds)
{
    s_lay = lay;
    uint8_t i = 0;

    lay->apply_layout();

    // the scene is the background, so it goes down first and the chrome sits on it
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_scene};
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_chrome};

    out[i++] = (EngineSlot){.zone = &lay->zones[ZONE_TIME], .text = readout_time};
    out[i++] = (EngineSlot){.zone = &lay->zones[ZONE_DATE], .text = readout_date};

    // dropped rather than blanked in big-clock mode, so they cost nothing: no layer, no store
    // pull, no fit
    if (sketchbook_layout() != LAYOUT_BIG_CLOCK)
    {
        // on a round screen the temperature is written into the scene, so it takes no slot here
#if !defined(PBL_ROUND)
        out[i++] = (EngineSlot){.zone = &lay->zones[ZONE_WEATHER], .text = readout_weather_temp};
#endif

        if (sketchbook_has_hr())
        {
            out[i++] = (EngineSlot){.zone = &lay->zones[ZONE_HR], .text = readout_hr};
        }
        // a round screen shows the temperature alone, so the steps never get a slot
#if !defined(PBL_ROUND)
        out[i++] = (EngineSlot){.zone = &lay->zones[ZONE_STEPS], .text = readout_steps};
#endif
    }

    return i;
}

void layout_apply_theme(const SceneLayout *lay)
{
    s_lay = lay;
    apply_theme_colors();
}

bool layout_palette_changed(const SceneLayout *lay)
{
    s_lay = lay;
    bool was_night = s_night;
    uint8_t was_theme = s_theme;
    apply_theme_colors();

    // a repaint is enough unless the palette really moved, which is the sun coming up or going down,
    // a Shuffle rolling to the next theme, or a day/night/Quiet swap landing on a different one
    return s_night != was_night || s_theme != was_theme;
}

void layout_teardown(const SceneLayout *lay)
{
    icons_cleanup();
    if (lay->scene_deinit)
    {
        lay->scene_deinit();
    }
    fonts_unload_all();
}

/** @} */
