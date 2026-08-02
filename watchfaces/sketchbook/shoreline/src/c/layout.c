/**
 * @file layout.c
 * @brief Shoreline face: the zone table bound to shared readouts, the drawn scene, and the
 * chrome draw-slot. No shell. The face drives the engine directly from main.
 *
 * Everything visual comes out of one palette, and which palette that is depends on the theme
 * *and* on whether the sun is up, so the zone colours are re-set whenever either moves.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "sketchbook/draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "scene/scene.h"
#include "sketchbook/sky/sky.h"
#include "theme/theme.h"
#include "sketchbook/widgets/widgets.h"
#include "settings_schema.h"
#include "io/stores/system_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

// the palette everything is drawn from, and the daylight it was chosen for. held rather than
// looked up per draw so the scene, the chrome, and the text can't disagree inside one frame
static const Palette *s_pal;
static bool s_night;

// --- Zone Table ---
// the clock on the sand with the date beneath it and the stats row along the bottom. the colours
// here are placeholders: apply_theme_colors re-sets every one from the live palette before the
// first paint
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,  .font_id = FONT_HAND_72, .align = GTextAlignmentCenter, .color = GColorBlack,
                       // the @beats token is wider than HH:MM and overflows Hand 72
                       .font_id_fallback = FONT_HAND_64, .rect_fallback = SLOT_TIME_SM},
    [ZONE_DATE]     = {.rect = SLOT_DATE,  .font_id = FONT_HAND_22, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_18, .rect_fallback = SLOT_DATE},
    [ZONE_WEATHER]  = {.rect = SLOT_TEMP,  .font_id = FONT_HAND_18, .align = GTextAlignmentLeft, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_TEMP_SM},
    [ZONE_HR]       = {.rect = SLOT_HR,    .font_id = FONT_HAND_18, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_HR_SM},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS, .font_id = FONT_HAND_18, .align = GTextAlignmentRight, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_STEPS_SM},
};

/**
 * @brief Point the palette at the current theme and daylight, and colour the zones from it.
 *
 * The clock sits on the sand and everything else sits over the sky, which is why the clock
 * takes `text` and the rest take `dim`.
 */
static void apply_theme_colors(void)
{
    s_night = sketchbook_sky_night();
    s_pal = palette_for_theme(settings_u8(SETTING_THEME), s_night);

    s_zones[ZONE_TIME].color = s_pal->text;
    s_zones[ZONE_DATE].color = s_pal->dim;
    s_zones[ZONE_WEATHER].color = s_pal->dim;
    s_zones[ZONE_HR].color = s_pal->dim;
    s_zones[ZONE_STEPS].color = s_pal->dim;

    // the date-up layouts move it off the sand onto the strip, so it takes the strip's ink
    if (shoreline_layout() != LAYOUT_STANDARD)
    {
        s_zones[ZONE_DATE].color = s_pal->bar_ink;
    }
}

/**
 * @brief Point the time and date zones at whichever layout is in force.
 *
 * The three differ only in where these two sit and how big the clock is, so the zone table
 * carries the standard one and this moves them for the other two. Called before every rebuild,
 * so a settings change lands without duplicating the table.
 */
static void apply_layout(void)
{
    uint8_t layout = shoreline_layout();

    // both date-up layouts share a date: on the strip, small, centred
    if (layout != LAYOUT_STANDARD)
    {
        s_zones[ZONE_DATE].rect = SLOT_DATE_TOP;
        s_zones[ZONE_DATE].font_id = FONT_HAND_18;
        s_zones[ZONE_DATE].rect_fallback = SLOT_DATE_TOP;
        s_zones[ZONE_DATE].font_id_fallback = FONT_HAND_16;
    }
    else
    {
        s_zones[ZONE_DATE].rect = SLOT_DATE;
        s_zones[ZONE_DATE].font_id = FONT_HAND_22;
        s_zones[ZONE_DATE].rect_fallback = SLOT_DATE;
        s_zones[ZONE_DATE].font_id_fallback = FONT_HAND_18;
    }

    // with no heart rate to show, the row is two readouts rather than three, so both move to the
    // centred boxes that keep each glyph and its number together as a pair. a round screen has
    // neither reading left down here to place
#if !defined(PBL_ROUND)
    if (!sketchbook_has_hr())
    {
        s_zones[ZONE_WEATHER].rect = SLOT_TEMP_PAIR;
        s_zones[ZONE_WEATHER].rect_fallback = SLOT_TEMP_PAIR_SM;
        s_zones[ZONE_WEATHER].align = GTextAlignmentCenter;

        s_zones[ZONE_STEPS].rect = SLOT_STEPS_PAIR;
        s_zones[ZONE_STEPS].rect_fallback = SLOT_STEPS_PAIR_SM;
        s_zones[ZONE_STEPS].align = GTextAlignmentCenter;
    }
#endif

    // the clock is what actually differs: same Hand 92 for both roomier layouts, sitting higher
    // when the readouts are still under it
    if (layout == LAYOUT_BIG_CLOCK)
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME_BIG;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_BIG_SM;
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_92);
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else if (layout == LAYOUT_DATE_TOP)
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME_DATETOP;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_DATETOP_SM;
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_92);
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_SM;
        // 82 on the circle rather than 92: at 92 the widest time runs 197px and the glass down
        // there is only 172 wide, so the outer digits ran into the bezel
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_72);
        s_zones[ZONE_TIME].font_id_fallback = PBL_IF_ROUND_ELSE(FONT_HAND_72, FONT_HAND_64);
    }
}

/**
 * @brief The cap line an above-the-colon marker sits on.
 *
 * Only the two Hand 92 layouts put it there, and their digits sit at different heights.
 */
static int meridiem_above_top(void)
{
    if (shoreline_layout() == LAYOUT_BIG_CLOCK)
    {
        return MERIDIEM_TOP_BIG;
    }

    // the round standard layout draws Hand 92 too, at its own height, so it gets its own channel
    // rather than borrowing the date-top one
    return PBL_IF_ROUND_ELSE(
        shoreline_layout() == LAYOUT_STANDARD ? MERIDIEM_TOP : MERIDIEM_TOP_DATETOP,
        MERIDIEM_TOP_DATETOP);
}

/**
 * @brief Patrick Hand at the sizes each slot needs.
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

    // the date only earns its own strip on the layouts that move it up there
    if (shoreline_layout() != LAYOUT_STANDARD)
    {
        widgets_draw_date_bar(ctx, bounds);
    }
    widgets_draw_battery(ctx, s_pal->bar_ink, theme, system_store_battery());

    // the row's marks go with the row, and only the big clock drops it
    if (shoreline_layout() != LAYOUT_BIG_CLOCK)
    {
        widgets_draw_stat_glyphs(ctx, s_pal, &s_zones[ZONE_WEATHER], &s_zones[ZONE_STEPS]);
    }

    // measured onto the live clock, so it takes the same slot and font. a 24-hour clock has no
    // marker anyway, and the setting drops it on a 12-hour one
    if (shoreline_show_meridiem())
    {
        GRect slot = s_zones[ZONE_TIME].rect;
        FontId font = s_zones[ZONE_TIME].font_id;

        // the standard clock has room either side, so the marker goes beside the digits. the
        // bigger two have none and drop it into the channel above the colon
        //
        // on a round screen the standard clock is Hand 92 as well, and at the widest time
        // (07:27, 197px) a marker beside it lands hard against the bezel with nothing to spare.
        // so it joins the other two and takes the colon channel
        if (PBL_IF_ROUND_ELSE(false, shoreline_layout() == LAYOUT_STANDARD))
        {
            widgets_draw_meridiem_beside(ctx, s_pal->dim, slot, font, MERIDIEM_TOP);
        }
        else
        {
            widgets_draw_meridiem_above(ctx, s_pal->dim, slot, font, meridiem_above_top());
        }
    }

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH), s_pal->bar_ink);
        icon_tint(icon_get(RESOURCE_ID_ICON_BLUETOOTH_SLASH), s_pal->bar_ink);
        widgets_draw_bt(ctx, system_store_bluetooth());
    }

    // the mark is drawn either way where the face bundles both states, so the strip keeps its
    // shape whether or not Quiet Time is holding. the setting still decides if it shows at all
    if (settings_u8(SETTING_QUIET_TIME_ICON))
    {
        widgets_draw_qt_state(ctx, s_pal->bar_ink, quiet_time_is_active());
    }
}

void shoreline_setup(Window *window)
{
    apply_theme_colors();
    load_fonts();

    // the scene paints the whole window, so nothing of this ever shows. it just stops a
    // half-built frame flashing white on the way in
    window_set_background_color(window, GColorBlack);
}

uint8_t shoreline_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    apply_layout();

    // the scene is the background, so it goes down first and the chrome sits on it
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_scene};
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_chrome};

    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_TIME],     .text = readout_time};
    out[i++] = (EngineSlot){.zone = &s_zones[ZONE_DATE],     .text = readout_date};

    // dropped rather than blanked in big-clock mode, so they cost nothing: no layer, no store
    // pull, no fit
    if (shoreline_layout() != LAYOUT_BIG_CLOCK)
    {
        // on a round screen the temperature rides the boat's pennant, painted with the scene, so
        // it takes no slot here
#if !defined(PBL_ROUND)
        out[i++] = (EngineSlot){.zone = &s_zones[ZONE_WEATHER],  .text = readout_weather_temp};
#endif
        if (sketchbook_has_hr())
        {
            out[i++] = (EngineSlot){.zone = &s_zones[ZONE_HR],   .text = readout_hr};
        }
        // a round screen shows the temperature alone, so the steps never get a slot
#if !defined(PBL_ROUND)
        out[i++] = (EngineSlot){.zone = &s_zones[ZONE_STEPS],    .text = readout_steps};
#endif
    }

    return i;
}

void shoreline_apply_theme(void)
{
    apply_theme_colors();
}

bool shoreline_daylight_changed(void)
{
    bool was_night = s_night;
    apply_theme_colors();

    return s_night != was_night;
}

void shoreline_teardown(void)
{
    icons_cleanup();
    fonts_unload_all();
}
