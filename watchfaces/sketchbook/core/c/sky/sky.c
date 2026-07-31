/**
 * @file sky.c
 * @brief The sky every Sketchbook face draws: its bands, its stars, and the sun or moon on its arc.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/sky/sky.h"

#include "sketchbook/config.h"
#include "clock/clockstr.h"
#include "clock/moon.h"
#include "clock/solar.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup family-sketchbook
 * @{
 */

// the day to fall back on before the phone has sent a sunrise and sunset. a plain twelve-hour
// day is wrong by an hour or two rather than wrong by half a day, so the scene still reads
#define FALLBACK_RISE (6 * 60)
#define FALLBACK_SET (18 * 60)


/**
 * @brief Today's sunrise and sunset in minutes past midnight, falling back to a plain
 * twelve-hour day when the phone has not sent them.
 *
 * @param rise_out Receives the sunrise.
 * @param set_out Receives the sunset.
 */
static void sun_times(int *rise_out, int *set_out)
{
    int rise = clockstr_minutes(weather_store_sunrise());
    int set = clockstr_minutes(weather_store_sunset());

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

/**
 * @brief How far the sun (or the moon, after dark) has got along its arc, from 0 at the left
 * horizon to 100 at the right.
 *
 * @return The progress from 0 to 100.
 */
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

/**
 * @brief Where on the arc a given progress lands.
 *
 * A half-ellipse over the horizon: progress 0 puts the disc on the left horizon, 50 at the
 * apex, and 100 on the right.
 *
 * @param progress How far along, 0 to 100.
 * @return The disc's centre.
 */
GPoint sketchbook_sky_arc_point(int progress)
{
    int32_t angle = DEG_TO_TRIGANGLE(180 * progress / 100);

    return GPoint(SKETCHBOOK_ARC_CX - (SKETCHBOOK_ARC_RX * cos_lookup(angle)) / TRIG_MAX_RATIO,
                  SKETCHBOOK_ARC_BASE_Y - (SKETCHBOOK_ARC_RY * sin_lookup(angle)) / TRIG_MAX_RATIO);
}

/**
 * @brief Fill the sky: the upper band, the lower band, and a stippled blend between them.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
void sketchbook_sky_draw_bands(GContext *ctx, GRect bounds, const Palette *pal)
{
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    int blend_end = SKETCHBOOK_SKY_BAND_Y + SKETCHBOOK_SKY_BLEND_H;
    graphics_context_set_fill_color(ctx, pal->sky_lo);
    graphics_fill_rect(ctx, GRect(0, blend_end, bounds.size.w, bounds.size.h - blend_end), 0, GCornerNone);

    // the two bands butted straight together read as a stripe ruled across the sky, so the
    // lower one is stippled in over the upper instead, thickening row by row until it takes
    // over. eight levels is all the dither the 64-colour panel can show anyway
    graphics_context_set_stroke_color(ctx, pal->sky_lo);
    for (int y = SKETCHBOOK_SKY_BAND_Y; y < blend_end; y++)
    {
        int level = ((y - SKETCHBOOK_SKY_BAND_Y) * 8) / SKETCHBOOK_SKY_BLEND_H;
        for (int x = 0; x < bounds.size.w; x++)
        {
            // offsetting the pattern per row stops the kept pixels stacking into columns
            if ((x + (y - SKETCHBOOK_SKY_BAND_Y) * 3) % 8 < level)
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
 */
void sketchbook_sky_draw_stars(GContext *ctx, const Palette *pal, const SketchbookStar *stars, int count)
{
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < count; i++)
    {
        GPoint at = GPoint(stars[i].x, stars[i].y);

        // the faint ones sit back in the dim colour and the bright ones come forward in the
        // disc colour, which is what stops the field reading as one flat spray of dots
        GColor color = stars[i].size == 0 ? pal->dim : pal->disc;
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
void sketchbook_sky_draw_arc(GContext *ctx, const Palette *pal)
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
static void draw_sun(GContext *ctx, const Palette *pal, GPoint at)
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
 * @brief Draw the moon at tonight's phase.
 *
 * The lit shape is carved rather than drawn: the whole disc goes down first, then a
 * sky-coloured circle (or half-rect at the quarters) is laid over it, and how far that
 * overlay is offset is what turns a crescent into a gibbous.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 */
static void draw_moon(GContext *ctx, const Palette *pal, GPoint at)
{
    // the overlay has to match whatever sky is behind the disc, or the shadow shows as a patch
    GColor behind = at.y < SKETCHBOOK_SKY_BAND_Y ? pal->sky_hi : pal->sky_lo;
    int phase = moon_glyph_index(time(NULL), 8);

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, SKETCHBOOK_DISC_R);

    graphics_context_set_fill_color(ctx, behind);
    switch (phase)
    {
        case 0:  // new: nothing lit, so only the outline is left behind
            graphics_fill_circle(ctx, at, SKETCHBOOK_DISC_R);
            break;
        case 1:  // waxing crescent: lit down the right edge
            graphics_fill_circle(ctx, GPoint(at.x - (SKETCHBOOK_DISC_R * 6) / 10, at.y), SKETCHBOOK_DISC_R);
            break;
        case 2:  // first quarter: the left half is dark
            graphics_fill_rect(ctx, GRect(at.x - SKETCHBOOK_DISC_R, at.y - SKETCHBOOK_DISC_R, SKETCHBOOK_DISC_R, SKETCHBOOK_DISC_R * 2 + 1), 0, GCornerNone);
            break;
        case 3:  // waxing gibbous: only a sliver of the left edge is dark
            graphics_fill_circle(ctx, GPoint(at.x - (SKETCHBOOK_DISC_R * 14) / 10, at.y), SKETCHBOOK_DISC_R);
            break;
        case 5:  // waning gibbous
            graphics_fill_circle(ctx, GPoint(at.x + (SKETCHBOOK_DISC_R * 14) / 10, at.y), SKETCHBOOK_DISC_R);
            break;
        case 6:  // last quarter: the right half is dark
            graphics_fill_rect(ctx, GRect(at.x + 1, at.y - SKETCHBOOK_DISC_R, SKETCHBOOK_DISC_R, SKETCHBOOK_DISC_R * 2 + 1), 0, GCornerNone);
            break;
        case 7:  // waning crescent: lit down the left edge
            graphics_fill_circle(ctx, GPoint(at.x + (SKETCHBOOK_DISC_R * 6) / 10, at.y), SKETCHBOOK_DISC_R);
            break;
        default: // full: the whole disc stays lit
            break;
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_circle(ctx, at, SKETCHBOOK_DISC_R);
    graphics_context_set_stroke_width(ctx, 1);
}

void sketchbook_sky_draw_disc(GContext *ctx, const Palette *pal, GPoint at, bool night)
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

/** @} */
