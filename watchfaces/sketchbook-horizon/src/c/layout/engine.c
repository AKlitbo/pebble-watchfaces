/**
 * @file engine.c
 * @brief The shared layout: palette, chrome, slot list and ticker response, the same for every
 * scene.
 *
 * One scene is up at a time. It is held here, with the palette and the ground row it was last
 * themed with, between the public calls and the draw-slots the engine fires. The shared sky and
 * weather code reads its geometry and its bolts back from here too.
 *
 * The sky and the weather are painted here for every scene, around the scene's own ground. The
 * night, the condition, its sky recipe, the disc, the minute and where a burst has got to are read
 * once at the top of the frame and handed to the scene. Anything else a scene draws from, such as
 * the tide, the wind or how lit the evening is, it reads for itself.
 *
 * A wet burst under the overcast band, on a scene with nothing of its own moving by day, moves only
 * the rain, the snow and the lightning. Its first frame copies the sky, the clouds and the ground
 * out of the framebuffer, and every later frame copies them back and paints only the weather and
 * the front on top, which cuts those frames down to a fraction of a full paint.
 *
 * @ingroup watchface-sketchbook
 */
#include "layout/engine.h"

#include "layout/zones.h"
#include "scene/scene.h"

#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "sky/sky.h"
#include "fx/fx.h"
#include "anim/anim.h"
#include "widgets/widgets.h"
#include "theme/palette.h"
#include "theme/palette_active.h"
#include "settings_schema.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "system/settings/settings.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the scene being drawn, and the palette and ground row it was themed with. held rather than looked
// up per draw so the scene, the chrome, and the text can't disagree inside one frame
static const SceneDef *s_scene;
static CommonPalette s_pal;
static const void *s_ground;
static bool s_night;
// the theme index the palette was chosen for, so a Shuffle rollover or a day/night/Quiet swap that
// lands on a different one is noticed and repainted
static uint8_t s_theme;

/**
 * @brief The still part of a wet burst's frames, held in the heap while the burst runs.
 *
 * It keeps what it was painted under too, so a change that lands mid-burst, such as a Shuffle
 * rolling over or the weather turning, paints afresh rather than putting back the old picture.
 */
typedef struct
{
    uint8_t *pixels;        ///< Every visible pixel, row after row, or NULL when nothing is held
    const SceneDef *scene;  ///< The scene it was painted for
    const void *ground;     ///< The ground row, which with the theme pins down the palette
    uint8_t theme;          ///< The theme the palette was put together from
    WxCond cond;            ///< The condition it was painted in
    size_t size;            ///< How many bytes the visible pixels take, worked out on the first copy
    bool no_room;           ///< The heap had no room for the copy, so this burst paints in full
} StillCache;

static StillCache s_still;

/**
 * @brief Point the palette at the current theme and daylight, and colour the zones from it.
 *
 * Which theme is up depends on the palette mode: the fixed one, a Shuffle that rotates with the
 * clock, or a day/night/Quiet Time swap. Whichever it lands on still takes its day or night colours
 * from the actual daylight. The sky comes from the shared table and the readouts from the scene's
 * ground row, and the two go together into the palette the frame paints with. The clock sits on the
 * scene's ground, which is why it takes `text` and the readouts beside it take `dim`.
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

    // an out-of-range theme falls back to Sketchbook, matching how the settings parser clamps
    if (s_theme >= THEME_COUNT)
    {
        s_theme = THEME_SKETCHBOOK;
    }

    s_ground = s_scene->ground_for_theme(s_theme, s_night);
    sketchbook_palette_assemble(&s_pal, s_theme, s_night, s_ground);
    const CommonPalette *cp = &s_pal;

    Zone *zones = layout_zones();
    zones[ZONE_TIME].color = cp->text;
    zones[ZONE_WEATHER].color = cp->dim;
    zones[ZONE_HR].color = cp->dim;
    zones[ZONE_STEPS].color = cp->dim;

    // the date-up layouts move it off the ground onto the strip, so it takes the strip's ink
    zones[ZONE_DATE].color = sketchbook_layout() != LAYOUT_STANDARD ? cp->bar_ink : cp->dim;
}

void sketchbook_still_drop(void)
{
    if (s_still.pixels)
    {
        free(s_still.pixels);
        s_still.pixels = NULL;
    }
}

/**
 * @brief Whether nothing behind the weather moves on this frame of a burst.
 *
 * The overcast band holds still and hides the disc. With no stars out and a ground marked still by
 * day, the rain, the snow and the bolts in front are all that move.
 *
 * @param frame This frame's reading.
 * @return True when the sky, the clouds and the ground paint the same as on the burst's other frames.
 */
static bool still_behind(const SceneFrame *frame)
{
    return frame->anim.tick != 0 && !frame->night && frame->sky.clouds == CLOUDS_OVERCAST && s_scene->still_by_day;
}

/**
 * @brief Copy every visible pixel between the framebuffer and the held copy, one way or the other.
 *
 * A round screen's framebuffer only stores the pixels inside the circle, so it goes row by row and
 * takes each row's own span. That keeps the copy at the visible pixels rather than the whole run
 * from the first to the last.
 *
 * @param fb The captured framebuffer.
 * @param keep True to copy out of the framebuffer, false to put the copy back.
 */
static void still_copy(GBitmap *fb, bool keep)
{
    GRect bounds = gbitmap_get_bounds(fb);
    uint8_t *at = s_still.pixels;

    for (int y = 0; y < bounds.size.h; y++)
    {
        GBitmapDataRowInfo row = gbitmap_get_data_row_info(fb, y);
        int width = row.max_x - row.min_x + 1;

        if (keep)
        {
            memcpy(at, row.data + row.min_x, width);
        }
        else
        {
            memcpy(row.data + row.min_x, at, width);
        }

        at += width;
    }
}

/**
 * @brief Copy the sky, the clouds and the ground just painted out of the framebuffer, for the
 * burst's later frames.
 *
 * With too little heap for the copy it keeps nothing, and every frame to the end of the burst paints
 * in full rather than asking again.
 *
 * @param ctx The graphics context.
 * @param frame This frame's reading.
 */
static void still_keep(GContext *ctx, const SceneFrame *frame)
{
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    if (!fb)
    {
        return;
    }

    // the framebuffer never changes shape, so its size is only counted once
    if (!s_still.size)
    {
        GRect bounds = gbitmap_get_bounds(fb);
        for (int y = 0; y < bounds.size.h; y++)
        {
            GBitmapDataRowInfo row = gbitmap_get_data_row_info(fb, y);
            s_still.size += row.max_x - row.min_x + 1;
        }
    }

    s_still.pixels = malloc(s_still.size);
    s_still.no_room = !s_still.pixels;
    if (s_still.pixels)
    {
        still_copy(fb, true);
        s_still.scene = s_scene;
        s_still.ground = s_ground;
        s_still.theme = s_theme;
        s_still.cond = frame->cond;
    }

    graphics_release_frame_buffer(ctx, fb);
}

/**
 * @brief Put the held sky, clouds and ground back into the framebuffer, if they were painted under
 * what this frame is.
 *
 * @param ctx The graphics context.
 * @param frame This frame's reading.
 * @return True when they went back, false when this frame has to paint them itself.
 */
static bool still_restore(GContext *ctx, const SceneFrame *frame)
{
    if (!s_still.pixels)
    {
        return false;
    }

    if (s_still.scene != s_scene || s_still.ground != s_ground || s_still.theme != s_theme ||
        s_still.cond != frame->cond)
    {
        sketchbook_still_drop();
        return false;
    }

    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    if (!fb)
    {
        return false;
    }

    still_copy(fb, false);
    graphics_release_frame_buffer(ctx, fb);
    return true;
}

/**
 * @brief Scene draw-slot: the whole painted background, from the sky to the scene's front.
 *
 * The sky, the clouds, the scene's ground, the weather and the scene's front, in that order. Night
 * is read fresh here rather than taken from the theme, so the sky always agrees with the clock it
 * is drawn at.
 *
 * A lightning strike paints its frame with the sky and the clouds a step lighter, from a copy of
 * the palette, so the next frame and the chrome still paint with the real one.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_scene(GContext *ctx, GRect bounds, const void *data)
{
    WxCond cond = sketchbook_cond_parse(weather_store_cond());
    SceneFrame frame = {
        .palette = &s_pal,
        .ground = s_ground,
        .night = sketchbook_sky_night(),
        .cond = cond,
        .sky = sketchbook_sky_recipe(cond),
        .disc = sketchbook_sky_arc_point(sketchbook_sky_arc_progress()),
        .minute = time_store_tm()->tm_min,
        .anim = anim_phase(),
    };

    CommonPalette flash;
    frame.lit = frame.sky.bolt && sketchbook_fx_flash(frame.anim);
    if (frame.lit)
    {
        flash = s_pal;
        sketchbook_fx_flash_palette(&flash);
        frame.palette = &flash;
    }

    // a strike's frame is a step lighter all over, so it paints in full and keeps nothing
    bool still = still_behind(&frame) && !frame.lit;

    // the still frame between bursts lets go of what the last burst held, and the next burst asks
    // the heap afresh
    if (frame.anim.tick == 0)
    {
        sketchbook_still_drop();
        s_still.no_room = false;
    }

    graphics_context_set_antialiased(ctx, true);

    if (!still || !still_restore(ctx, &frame))
    {
        sketchbook_sky_draw(ctx, bounds, &frame);

        // clouds sit in front of the disc but behind the scene's ground, so a low sun still goes
        // down behind the mountains, the sea, the city or the landmark
        sketchbook_fx_draw_clouds(ctx, &frame);

        // the birds fly in front of the clouds and behind the ground, unless the scene has its own
        if (!s_scene->own_birds)
        {
            sketchbook_fx_draw_birds(ctx, &frame);
        }

        s_scene->ground(ctx, bounds, &frame);

        if (still && !s_still.pixels && !s_still.no_room)
        {
            still_keep(ctx, &frame);
        }
    }

    sketchbook_fx_draw_weather(ctx, &frame);

    if (s_scene->front)
    {
        s_scene->front(ctx, bounds, &frame);
    }
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
    const CommonPalette *cp = &s_pal;

    widgets_draw_top_bar(ctx, bounds);

    // the gauge warns in whichever palette is on screen, Shuffle and the night and Quiet Time
    // palettes included
    widgets_draw_battery(ctx, cp, system_store_battery());

    // the row's marks go with the row, and only the big clock drops it. a round screen has no row
#if !defined(PBL_ROUND)
    if (sketchbook_layout() != LAYOUT_BIG_CLOCK)
    {
        widgets_draw_stat_glyphs(ctx, cp, &layout_zones()[ZONE_WEATHER], &layout_zones()[ZONE_STEPS],
            layout_zones_has_hr());
    }
#endif

    // measured onto the live clock, so it takes the same slot and font. a 24-hour clock has no
    // marker anyway, and the setting drops it on a 12-hour one
    if (sketchbook_show_meridiem())
    {
        const Zone *clock_zone = &layout_zones()[ZONE_TIME];
        GRect slot = clock_zone->rect;
        FontId font = clock_zone->font_id;

        // the standard clock has room either side, so the marker goes beside the digits. the
        // bigger two have none and drop it into the channel above the colon. on a round screen the
        // standard clock fills the width too, so a marker beside it lands hard against the bezel and
        // it joins the other two
        if (PBL_IF_ROUND_ELSE(false, sketchbook_layout() == LAYOUT_STANDARD))
        {
            widgets_draw_meridiem_beside(ctx, cp->dim, slot, font, LAYOUT_MERIDIEM_BESIDE_TOP);
        }
        else
        {
            widgets_draw_meridiem_above(ctx, cp->dim, slot, font, layout_meridiem_above_top());
        }
    }

    // honour the show/hide setting. the store owns the connect/disconnect vibe
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        widgets_draw_bt(ctx, cp->bar_ink, system_store_bluetooth());
    }

    // the mark is drawn either way, so the strip keeps its shape whether or not Quiet Time is
    // holding. the setting still decides if it shows at all
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
    fonts_load(FONT_HAND_82, RESOURCE_ID_FONT_HAND_82);
#else
    fonts_load(FONT_HAND_92, RESOURCE_ID_FONT_HAND_92);
#endif
    fonts_load(FONT_HAND_72, RESOURCE_ID_FONT_HAND_72);
    // 64 is the fallback under the rectangle's Hand 72 clock. the circle falls back from 82 to 72
#if !defined(PBL_ROUND)
    fonts_load(FONT_HAND_64, RESOURCE_ID_FONT_HAND_64);
#endif
    fonts_load(FONT_HAND_22, RESOURCE_ID_FONT_HAND_22);
    fonts_load(FONT_HAND_18, RESOURCE_ID_FONT_HAND_18);
    fonts_load(FONT_HAND_16, RESOURCE_ID_FONT_HAND_16);
}

const SceneGeometry *sketchbook_scene_geometry(void)
{
    // nothing is up until sketchbook_setup runs, so a read before then gets Ridgeline's
    return (s_scene ? s_scene : sketchbook_scene_def(SCENE_RIDGELINE))->geometry;
}

const BoltSpots *sketchbook_scene_bolts(void)
{
    // a read before sketchbook_setup gets Ridgeline's, as the geometry does
    return (s_scene ? s_scene : sketchbook_scene_def(SCENE_RIDGELINE))->bolts;
}

void sketchbook_setup(Window *window)
{
    s_scene = sketchbook_scene_def(sketchbook_scene());
    apply_theme_colors();
    load_fonts();

    // the scene paints the whole window, so nothing of this ever shows. it just stops a
    // half-built frame flashing white on the way in
    window_set_background_color(window, GColorBlack);
}

// a build writes up to seven slots, and the appinfo sets ENGINE_SLOTS_MAX to exactly seven. this
// stops the build if the cap ever drops below them. a new slot past seven is caught by next_slot
_Static_assert(ENGINE_SLOTS_MAX >= 7, "a scene build writes up to seven slots");

/**
 * @brief Where the build writes its next slot, or a spare when the engine has no room left.
 *
 * The engine hands over room for max slots. Writing one more would run off the end of its arrays,
 * so a slot past the cap goes into the spare, which nothing reads, and is named in the log.
 *
 * @param out The engine's slot list.
 * @param max How many slots it has room for.
 * @param count How many are taken so far, moved on when this one fits.
 * @return The slot to write.
 */
static EngineSlot *next_slot(EngineSlot *out, uint8_t max, uint8_t *count)
{
    static EngineSlot s_spare;

    if (*count >= max)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "slot %d is past the engine's %d. Raise ENGINE_SLOTS_MAX in the appinfo",
            *count + 1, max);
        return &s_spare;
    }

    return &out[(*count)++];
}

uint8_t sketchbook_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    Zone *zones = layout_zones();
    layout_zones_apply(s_scene->clock_only_y);

    // the scene is the background, so it goes down first and the chrome sits on it
    // the scene carries its own tag, so a burst's frame repaints it alone and leaves the readouts as
    // they are. the SDK still redraws every layer on the window, so the chrome reads the battery and
    // Bluetooth again, but the text slots draw the text they hold rather than reading their stores
    // and fitting it again
    *next_slot(out, max, &i) = (EngineSlot){.frame = bounds, .draw = draw_scene, .tags = SKETCHBOOK_TAG_SCENE};
    *next_slot(out, max, &i) = (EngineSlot){.frame = bounds, .draw = draw_chrome, .tags = SKETCHBOOK_TAG_REST};

    *next_slot(out, max, &i) = (EngineSlot){.zone = &zones[ZONE_TIME], .text = readout_time,
                                            .tags = SKETCHBOOK_TAG_REST};
    *next_slot(out, max, &i) = (EngineSlot){.zone = &zones[ZONE_DATE], .text = readout_date,
                                            .tags = SKETCHBOOK_TAG_REST};

    // dropped rather than blanked in big-clock mode, so they cost nothing: no layer, no store
    // pull, no fit
    //
    // a round screen has no row down here at all. its temperature is written into the scene, its
    // steps have no room, and it has no heart rate sensor
#if !defined(PBL_ROUND)
    if (sketchbook_layout() != LAYOUT_BIG_CLOCK)
    {
        *next_slot(out, max, &i) = (EngineSlot){.zone = &zones[ZONE_WEATHER], .text = readout_weather_temp,
                                                .tags = SKETCHBOOK_TAG_REST};

        if (layout_zones_has_hr())
        {
            *next_slot(out, max, &i) = (EngineSlot){.zone = &zones[ZONE_HR], .text = readout_hr,
                                                    .tags = SKETCHBOOK_TAG_REST};
        }
        *next_slot(out, max, &i) = (EngineSlot){.zone = &zones[ZONE_STEPS], .text = readout_steps,
                                                .tags = SKETCHBOOK_TAG_REST};
    }
#endif

    return i;
}

void sketchbook_apply_theme(void)
{
    apply_theme_colors();
}

bool sketchbook_palette_changed(void)
{
    bool was_night = s_night;
    uint8_t was_theme = s_theme;
    apply_theme_colors();

    // a repaint is enough unless the palette really moved, which is the sun coming up or going down,
    // a Shuffle rolling to the next theme, or a day/night/Quiet swap landing on a different one
    bool palette_moved = s_night != was_night || s_theme != was_theme;

#if defined(PBL_ROUND)
    return palette_moved;
#else
    // a sensor that came or went since the row was laid out means three readouts or two
    return palette_moved || sketchbook_has_hr() != layout_zones_has_hr();
#endif
}

bool sketchbook_scene_reselect(void)
{
    const SceneDef *next = sketchbook_scene_def(sketchbook_scene());
    if (next == s_scene)
    {
        return false;
    }

    s_scene = next;
    apply_theme_colors();
    return true;
}

uint8_t sketchbook_burst_ticks(void)
{
    WxCond cond = sketchbook_cond_parse(weather_store_cond());
    SketchbookSky sky = sketchbook_sky_recipe(cond);

    // a dry overcast day on a scene with nothing of its own moving has no disc, no stars, nothing
    // falling and a band that holds still, so a burst there would repaint the same frame over and
    // over. it gets no burst at all
    if (sky.clouds == CLOUDS_OVERCAST && sky.precip == PRECIP_NONE && !sky.bolt && !sky.wash &&
        s_scene->still_by_day && !sketchbook_sky_night())
    {
        return 0;
    }

    // a storm runs longer, so the lightning has room to strike three times
    return sky.bolt ? ANIM_STORM_TICKS : ANIM_TICKS;
}

void sketchbook_teardown(void)
{
    sketchbook_still_drop();
    icons_cleanup();
    fonts_unload_all();
}

/** @} */
