/**
 * @file sky.c
 * @brief The sky the layout engine paints over every scene's ground: its bands, its stars, and the
 * sun or moon on its arc, plus how lit the evening is.
 *
 * @ingroup watchface-sketchbook
 */
#include "sky/sky.h"

#include "scene/geometry.h"
#include "theme/palette.h"
#include "clock/moon.h"
#include "clock/solar.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the day to fall back on before the phone has sent a sunrise and sunset. a plain twelve-hour
// day is wrong by an hour or two rather than wrong by half a day, so the scene still reads
#define FALLBACK_RISE (6 * 60)
#define FALLBACK_SET (18 * 60)

/**
 * @brief Today's sunrise and sunset in minutes past midnight, falling back to a plain
 * twelve-hour day for whichever the phone has not sent.
 *
 * Each one falls back on its own, so a real sunrise still places the morning when the sunset is
 * missing.
 *
 * @param rise_out Receives the sunrise.
 * @param set_out Receives the sunset.
 */
static void sun_times(int *rise_out, int *set_out)
{
    int rise = weather_store_sunrise();
    int set = weather_store_sunset();

    *rise_out = rise >= 0 ? rise : FALLBACK_RISE;
    *set_out = set >= 0 ? set : FALLBACK_SET;
}

/** @brief The clock in minutes past midnight. */
static int now_minutes(void)
{
    const struct tm *now = time_store_tm();
    return now->tm_hour * 60 + now->tm_min;
}

bool sketchbook_sky_night(void)
{
    int rise, set;
    sun_times(&rise, &set);

    return solar_day_progress(rise, set, now_minutes()) < 0;
}

int sketchbook_sky_arc_progress(void)
{
    int rise, set;
    sun_times(&rise, &set);
    int now = now_minutes();

    int day = solar_day_progress(rise, set, now);
    if (day >= 0)
    {
        return day;
    }

    int night = solar_night_progress(rise, set, now);
    return night >= 0 ? night : 50;
}

GPoint sketchbook_sky_arc_point(int progress)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    int32_t angle = DEG_TO_TRIGANGLE(180 * progress / 100);

    return GPoint(SKETCHBOOK_ARC_CX - (g->arc_rx * cos_lookup(angle)) / TRIG_MAX_RATIO,
                  g->arc_base_y - (g->arc_ry * sin_lookup(angle)) / TRIG_MAX_RATIO);
}

/**
 * @brief Whether the lower sky has taken over at this pixel of the stippled blend.
 *
 * The one place the dither pattern is decided, so anything drawing over the sky can ask for the
 * same answer the sky itself gave.
 *
 * @param x The pixel's column.
 * @param y The pixel's row, inside the blend band.
 * @return True when the lower sky shows through here.
 */
static bool sky_blend_takes_over(int x, int y)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    int level = ((y - g->sky_band_y) * 8) / g->sky_blend_h;

    // offsetting the pattern per row stops the kept pixels stacking into columns
    return (x + (y - g->sky_band_y) * 3) % 8 < level;
}

/**
 * @brief The sky colour at one pixel, blend and all.
 *
 * @param pal The palette in use.
 * @param x The pixel's column.
 * @param y The pixel's row.
 * @return The colour the sky is painted there.
 */
static GColor sky_colour_at(const CommonPalette *pal, int x, int y)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    if (y < g->sky_band_y)
    {
        return pal->sky_hi;
    }
    if (y >= g->sky_band_y + g->sky_blend_h)
    {
        return pal->sky_lo;
    }

    return sky_blend_takes_over(x, y) ? pal->sky_lo : pal->sky_hi;
}

/**
 * @brief Fill the sky: the upper band, the lower band, and a stippled blend between them.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
static void draw_bands(GContext *ctx, GRect bounds, const CommonPalette *pal)
{
    const SceneGeometry *g = sketchbook_scene_geometry();

    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    int blend_end = g->sky_band_y + g->sky_blend_h;
    graphics_context_set_fill_color(ctx, pal->sky_lo);
    graphics_fill_rect(ctx, GRect(0, blend_end, bounds.size.w, bounds.size.h - blend_end), 0, GCornerNone);

    // the two bands butted straight together read as a stripe ruled across the sky, so the
    // lower one is stippled in over the upper instead, thickening row by row until it takes
    // over. eight levels is all the dither the 64-colour panel can show anyway
    graphics_context_set_stroke_color(ctx, pal->sky_lo);
    for (int y = g->sky_band_y; y < blend_end; y++)
    {
        for (int x = 0; x < bounds.size.w; x++)
        {
            if (sky_blend_takes_over(x, y))
            {
                graphics_draw_pixel(ctx, GPoint(x, y));
            }
        }
    }
}

/**
 * @brief Scatter the star field. Night only.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param stars The scene's own field.
 * @param count How many there are.
 */
static void draw_stars(GContext *ctx, const CommonPalette *pal, const SketchbookStar *stars, int count)
{
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < count; i++)
    {
        GPoint at = GPoint(stars[i].x, stars[i].y);

        // the faint ones sit back in the star colour and the bright ones come forward in the
        // disc colour, which is what stops the field reading as one flat spray of dots
        GColor color = stars[i].size == 0 ? pal->star : pal->disc;
        graphics_context_set_fill_color(ctx, color);
        graphics_context_set_stroke_color(ctx, color);

        if (stars[i].size == 2)
        {
            graphics_draw_line(ctx, GPoint(at.x - 3, at.y), GPoint(at.x + 3, at.y));
            graphics_draw_line(ctx, GPoint(at.x, at.y - 3), GPoint(at.x, at.y + 3));
            graphics_fill_circle(ctx, at, 1);
        }
        else
        {
            graphics_fill_circle(ctx, at, stars[i].size);
        }
    }
}

/**
 * @brief Draw the arc the disc travels, as a broken guide line.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_arc(GContext *ctx, const CommonPalette *pal)
{
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);

    // every other span is skipped, which dashes the line without needing a dash routine
    for (int p = 0; p < 100; p += 8)
    {
        graphics_draw_line(ctx, sketchbook_sky_arc_point(p), sketchbook_sky_arc_point(p + 4));
    }
}

/**
 * @brief Draw the sun: a filled disc, an ink outline, and eight uneven rays.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 */
static void draw_sun(GContext *ctx, const CommonPalette *pal, GPoint at)
{
    // uneven ray lengths, one per eighth turn, so the burst looks scrawled rather than stamped
    static const int16_t ray_len[] = {9, 6, 10, 7, 9, 6, 10, 7};

    graphics_context_set_stroke_color(ctx, pal->disc);
    graphics_context_set_stroke_width(ctx, 3);

    for (unsigned i = 0; i < ARRAY_LENGTH(ray_len); i++)
    {
        int32_t angle = DEG_TO_TRIGANGLE(45 * (int)i);
        int32_t sin_a = sin_lookup(angle);
        int32_t cos_a = cos_lookup(angle);
        int inner = SKETCHBOOK_DISC_R + 4;
        int outer = inner + ray_len[i];

        graphics_draw_line(ctx,
            GPoint(at.x + (inner * sin_a) / TRIG_MAX_RATIO, at.y - (inner * cos_a) / TRIG_MAX_RATIO),
            GPoint(at.x + (outer * sin_a) / TRIG_MAX_RATIO, at.y - (outer * cos_a) / TRIG_MAX_RATIO));
    }

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, SKETCHBOOK_DISC_R);

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_circle(ctx, at, SKETCHBOOK_DISC_R);
    graphics_context_set_stroke_width(ctx, 1);
}

/**
 * @brief Whether the shadow covers this pixel, which is what carves the lit shape.
 *
 * The dark part of a crescent or gibbous is a second circle laid over the disc, and how far it
 * is offset is what decides the phase. The quarters are a straight half instead, since an
 * offset circle can only ever give a curved edge.
 *
 * @param phase The glyph index, 0 new through 4 full to 7 waning crescent.
 * @param at The disc's centre.
 * @param x The pixel's column.
 * @param y The pixel's row.
 * @return True when this pixel is in shadow.
 */
static bool moon_shadow_covers(int phase, GPoint at, int x, int y)
{
    int dx;
    int dy = y - at.y;

    switch (phase)
    {
        case 0: return true;        // new: nothing lit, so only the outline is left behind
        case 2: return x < at.x;    // first quarter: the left half is dark
        case 6: return x > at.x;    // last quarter: the right half is dark
        case 1: dx = x - (at.x - (SKETCHBOOK_DISC_R * 6) / 10); break;   // waxing crescent
        case 3: dx = x - (at.x - (SKETCHBOOK_DISC_R * 14) / 10); break;  // waxing gibbous
        case 5: dx = x - (at.x + (SKETCHBOOK_DISC_R * 14) / 10); break;  // waning gibbous
        case 7: dx = x - (at.x + (SKETCHBOOK_DISC_R * 6) / 10); break;   // waning crescent
        default: return false;      // full: the whole disc stays lit
    }

    return dx * dx + dy * dy <= SKETCHBOOK_DISC_R * SKETCHBOOK_DISC_R;
}

/**
 * @brief Draw the moon at tonight's phase.
 *
 * The lit shape is carved rather than drawn: the whole disc goes down first, then the shadow is
 * painted back over it in whatever the sky behind it is.
 *
 * That last part is per pixel rather than one flat fill. The disc sits over the stippled blend
 * between the two sky bands for most of its arc, and no single colour matches a dither, so a
 * flat shadow reads as a patch of the wrong blue floating on the moon.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 */
static void draw_moon(GContext *ctx, const CommonPalette *pal, GPoint at)
{
    // the phase follows the time store's clock rather than time(NULL), so a dev build's pinned
    // date shows its own moon. mktime turns the local clock back into the moment it stands for
    struct tm now = *time_store_tm();
    int phase = moon_glyph_index(mktime(&now), 8);

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, SKETCHBOOK_DISC_R);

    for (int y = at.y - SKETCHBOOK_DISC_R; y <= at.y + SKETCHBOOK_DISC_R; y++)
    {
        for (int x = at.x - SKETCHBOOK_DISC_R; x <= at.x + SKETCHBOOK_DISC_R; x++)
        {
            int dx = x - at.x;
            int dy = y - at.y;

            if (dx * dx + dy * dy > SKETCHBOOK_DISC_R * SKETCHBOOK_DISC_R)
            {
                continue; // outside the disc, so it is sky already
            }
            if (!moon_shadow_covers(phase, at, x, y))
            {
                continue; // lit, so leave the disc colour showing
            }

            graphics_context_set_stroke_color(ctx, sky_colour_at(pal, x, y));
            graphics_draw_pixel(ctx, GPoint(x, y));
        }
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_circle(ctx, at, SKETCHBOOK_DISC_R);
    graphics_context_set_stroke_width(ctx, 1);
}

/**
 * @brief Draw the sun, or the moon at tonight's phase.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 * @param night True to draw the moon rather than the sun.
 */
static void draw_disc(GContext *ctx, const CommonPalette *pal, GPoint at, bool night)
{
    if (night)
    {
        draw_moon(ctx, pal, at);
    }
    else
    {
        draw_sun(ctx, pal, at);
    }
}

void sketchbook_sky_draw(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const SceneGeometry *g = sketchbook_scene_geometry();

    const CommonPalette *pal = frame->palette;

    draw_bands(ctx, bounds, pal);

    if (frame->night)
    {
        draw_stars(ctx, pal, g->stars, g->star_count);
    }

    draw_arc(ctx, pal);

    if (frame->sky.celestial != CEL_HIDDEN)
    {
        draw_disc(ctx, pal, frame->disc, frame->night);
    }
}

int sketchbook_sky_lit(void)
{
    // the lights take the two times as a pair. one missing sends both back to the plain day
    int rise = weather_store_sunrise();
    int set = weather_store_sunset();
    if (rise < 0 || set < 0)
    {
        rise = FALLBACK_RISE;
        set = FALLBACK_SET;
    }

    // solar_night_progress already handles the night wrapping past midnight and how short a
    // summer night is. all that is added here is the shape over it
    int night = solar_night_progress(rise, set, now_minutes());
    if (night < 0)
    {
        return 0;  // broad daylight
    }

    // up fast over the first fifth of the night, then a slow decline as the place turns in
    if (night < 20)
    {
        return (night * 100) / 20;
    }

    return 100 - ((night - 20) * 45) / 80;
}

int sketchbook_sky_light_rank(int a, int b)
{
    // the multiply and shift scatters neighbours that would otherwise light in a row
    // the multiplies are done unsigned, so a large coordinate wraps rather than overflowing
    unsigned mixed = ((unsigned)a * 73856093u) ^ ((unsigned)b * 19349663u);
    return (int)((mixed >> 13) % 100);
}

/** @} */
