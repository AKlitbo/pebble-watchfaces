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
 * @brief Paint the lower sky's share of one row of the stippled blend, between two columns.
 *
 * The one place the dither pattern is decided, so anything drawing over the sky gets the same
 * pixels the sky itself had. In every eight columns the lower sky takes the first few, more of
 * them the further down the blend the row is, so each row is a string of short runs and each run
 * goes down as one fill. It paints in the fill colour already set.
 *
 * @param ctx The graphics context.
 * @param g The scene's geometry.
 * @param y The row, inside the blend band.
 * @param left The first column to paint.
 * @param right The last column to paint.
 */
static void fill_blend_row(GContext *ctx, const SceneGeometry *g, int y, int left, int right)
{
    int into = y - g->sky_band_y;
    int level = (into * 8) / g->sky_blend_h;
    if (level == 0)
    {
        return;
    }

    // a column left of the screen is never seen, and starting at 0 keeps the modulo below from
    // going negative
    if (left < 0)
    {
        left = 0;
    }

    // the lower sky takes a column where (x + into * 3) % 8 < level, so each run starts where that
    // sum is a whole number of eights. offsetting the pattern per row stops the runs stacking into
    // columns. the first run can start left of the span and is cut to it
    for (int x = left - (left + into * 3) % 8; x <= right; x += 8)
    {
        int from = x < left ? left : x;
        int to = x + level - 1 < right ? x + level - 1 : right;
        if (from <= to)
        {
            graphics_fill_rect(ctx, GRect(from, y, to - from + 1, 1), 0, GCornerNone);
        }
    }
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
    for (int y = g->sky_band_y; y < blend_end; y++)
    {
        fill_blend_row(ctx, g, y, 0, bounds.size.w - 1);
    }
}

/**
 * @brief Scatter the star field. Night only.
 *
 * In a burst each star takes its own turn to dip. A cross pulls its arms in, a bright dot drops to
 * a faint one, and a faint one winks out for a frame. Each keeps its own place in an eight frame
 * beat, so the field never pulses all at once.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param stars The scene's own field.
 * @param count How many there are.
 * @param anim Where the burst has got to.
 */
static void draw_stars(GContext *ctx, const CommonPalette *pal, const SketchbookStar *stars, int count,
    AnimPhase anim)
{
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < count; i++)
    {
        GPoint at = GPoint(stars[i].x, stars[i].y);
        int size = stars[i].size;

        if (anim.tick)
        {
            int beat = (anim.tick + sketchbook_sky_light_rank(i, 11)) % 8;

            if (size == 2 && beat < 2)
            {
                size = 1;
            }
            else if (size == 1 && beat == 3)
            {
                size = 0;
            }
            else if (size == 0 && beat == 5)
            {
                continue;
            }
        }

        // the faint ones sit back in the star colour and the bright ones come forward in the
        // disc colour, which is what stops the field reading as one flat spray of dots
        GColor color = size == 0 ? pal->star : pal->disc;
        graphics_context_set_fill_color(ctx, color);
        graphics_context_set_stroke_color(ctx, color);

        if (size == 2)
        {
            graphics_draw_line(ctx, GPoint(at.x - 3, at.y), GPoint(at.x + 3, at.y));
            graphics_draw_line(ctx, GPoint(at.x, at.y - 3), GPoint(at.x, at.y + 3));
            graphics_fill_circle(ctx, at, 1);
        }
        else
        {
            graphics_fill_circle(ctx, at, size);
        }
    }
}

/**
 * @brief Send a shooting star across the top of the sky, partway through a burst that brings one.
 *
 * It comes in off the right edge and runs down to the left with a short tail, then the tail burns
 * out after the head has gone. It is drawn over the arc's dashes and under the moon. It is never
 * there when still.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param anim Where the burst has got to.
 */
static void draw_shooting_star(GContext *ctx, const CommonPalette *pal, AnimPhase anim)
{
    // a storm's longer burst never brings one, since its sky is covered, so the frames are an
    // ordinary burst's
    int t = anim.tick;
    if (!anim.extras || t < 5 || t > 13)
    {
        return;
    }

    // high in the sky, under the status bar and well clear of the scene's ground. it starts just
    // off the right edge and stops right of the middle, clear of where the moon tops its arc
    int x0 = PBL_DISPLAY_WIDTH + 4;
    int y0 = PBL_IF_ROUND_ELSE(40, 26);
    int run = PBL_DISPLAY_WIDTH / 3;

    // the head crosses over seven frames, coming in off the edge with its tail behind it, then the
    // tail burns out behind where it went over the next two
    int along = t <= 11 ? ((t - 5) * run) / 6 : run;
    int tail = t <= 11 ? (along < 14 ? along : 14) : 14 - (t - 11) * 6;

    GPoint head = GPoint(x0 - along, y0 + along / 3);
    GPoint back = GPoint(head.x + tail, head.y - tail / 3);

    graphics_context_set_stroke_color(ctx, pal->disc);
    graphics_context_set_stroke_width(ctx, 1);
    if (tail > 0)
    {
        graphics_draw_line(ctx, head, back);
    }

    if (t <= 11)
    {
        graphics_context_set_fill_color(ctx, pal->disc);
        graphics_fill_circle(ctx, head, 1);
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
 * In a burst the ray lengths step round the rays, so the sun shimmers.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param at The disc's centre.
 * @param anim Where the burst has got to.
 */
static void draw_sun(GContext *ctx, const CommonPalette *pal, GPoint at, AnimPhase anim)
{
    // uneven ray lengths, one per eighth turn, so the burst looks scrawled rather than stamped
    static const int16_t ray_len[] = {9, 6, 10, 7, 9, 6, 10, 7};

    // two trips round the four lengths over a burst, which lands back on the still pattern
    unsigned shift = (unsigned)anim_loop(anim, 4, 2);

    graphics_context_set_stroke_color(ctx, pal->disc);
    graphics_context_set_stroke_width(ctx, 3);

    for (unsigned i = 0; i < ARRAY_LENGTH(ray_len); i++)
    {
        int32_t angle = DEG_TO_TRIGANGLE(45 * (int)i);
        int32_t sin_a = sin_lookup(angle);
        int32_t cos_a = cos_lookup(angle);
        int inner = SKETCHBOOK_DISC_R + 4;
        int outer = inner + ray_len[(i + shift) % ARRAY_LENGTH(ray_len)];

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
 * @brief Where the shadow lies across one row of the disc, which is what carves the lit shape.
 *
 * The dark part of a crescent or gibbous is a second circle laid over the disc, and how far it
 * is offset is what decides the phase. The quarters are a straight half instead, since an
 * offset circle can only ever give a curved edge. Either way the shadow crosses a row in one
 * unbroken run.
 *
 * @param phase The glyph index, 0 new through 4 full to 7 waning crescent.
 * @param at The disc's centre.
 * @param reach How far the disc reaches either side of its centre on this row.
 * @param left Receives the first column in shadow.
 * @param right Receives the last column in shadow.
 * @return False when nothing on this row is in shadow.
 */
static bool moon_shadow_span(int phase, GPoint at, int reach, int *left, int *right)
{
    int shadow_x;

    switch (phase)
    {
        case 0:  // new: nothing lit, so only the outline is left behind
            *left = at.x - reach;
            *right = at.x + reach;
            return true;
        case 2:  // first quarter: the left half is dark
            *left = at.x - reach;
            *right = at.x - 1;
            return *left <= *right;
        case 6:  // last quarter: the right half is dark
            *left = at.x + 1;
            *right = at.x + reach;
            return *left <= *right;
        case 1: shadow_x = at.x - (SKETCHBOOK_DISC_R * 6) / 10; break;   // waxing crescent
        case 3: shadow_x = at.x - (SKETCHBOOK_DISC_R * 14) / 10; break;  // waxing gibbous
        case 5: shadow_x = at.x + (SKETCHBOOK_DISC_R * 14) / 10; break;  // waning gibbous
        case 7: shadow_x = at.x + (SKETCHBOOK_DISC_R * 6) / 10; break;   // waning crescent
        default: return false;  // full: the whole disc stays lit
    }

    // the shadow circle is the disc's size, so on any row it reaches as far either side of its own
    // centre as the disc does of its. the shadow is where the two runs overlap
    *left = (shadow_x > at.x ? shadow_x : at.x) - reach;
    *right = (shadow_x < at.x ? shadow_x : at.x) + reach;
    return *left <= *right;
}

/**
 * @brief Draw the moon at tonight's phase.
 *
 * The lit shape is carved rather than drawn: the whole disc goes down first, then the shadow is
 * painted back over it in whatever the sky behind it is.
 *
 * Over the stippled blend between the two sky bands the shadow carries the same stipple rather
 * than one flat colour. The disc sits over the blend for most of its arc, and no single colour
 * matches a dither, so a flat shadow reads as a patch of the wrong blue floating on the moon.
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

    const SceneGeometry *g = sketchbook_scene_geometry();
    int blend_end = g->sky_band_y + g->sky_blend_h;

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, SKETCHBOOK_DISC_R);

    // the furthest column either side of the centre still inside the disc on the row. it widens
    // down to the middle row and narrows after it, so each row moves on from the one above
    int reach = 0;
    for (int dy = -SKETCHBOOK_DISC_R; dy <= SKETCHBOOK_DISC_R; dy++)
    {
        while ((reach + 1) * (reach + 1) + dy * dy <= SKETCHBOOK_DISC_R * SKETCHBOOK_DISC_R)
        {
            reach++;
        }
        while (reach * reach + dy * dy > SKETCHBOOK_DISC_R * SKETCHBOOK_DISC_R)
        {
            reach--;
        }

        int left;
        int right;
        if (!moon_shadow_span(phase, at, reach, &left, &right))
        {
            continue;  // lit all the way across, so leave the disc colour showing
        }

        // the shadow goes down in the band behind it. over the blend that is the upper sky, with
        // the lower sky's runs laid back in on top
        int y = at.y + dy;
        graphics_context_set_fill_color(ctx, y < blend_end ? pal->sky_hi : pal->sky_lo);
        graphics_fill_rect(ctx, GRect(left, y, right - left + 1, 1), 0, GCornerNone);

        if (y >= g->sky_band_y && y < blend_end)
        {
            graphics_context_set_fill_color(ctx, pal->sky_lo);
            fill_blend_row(ctx, g, y, left, right);
        }
    }

    // the shadow leaves a sky colour in the fill. putting the disc colour back means whatever draws
    // after the moon starts from the same fill it gets after the sun
    graphics_context_set_fill_color(ctx, pal->disc);

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
 * @param anim Where the burst has got to.
 */
static void draw_disc(GContext *ctx, const CommonPalette *pal, GPoint at, bool night, AnimPhase anim)
{
    if (night)
    {
        draw_moon(ctx, pal, at);
    }
    else
    {
        draw_sun(ctx, pal, at, anim);
    }
}

void sketchbook_sky_draw(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const SceneGeometry *g = sketchbook_scene_geometry();

    const CommonPalette *pal = frame->palette;

    draw_bands(ctx, bounds, pal);

    if (frame->night)
    {
        draw_stars(ctx, pal, g->stars, g->star_count, frame->anim);
    }

    draw_arc(ctx, pal);

    // only where the sky is open enough to see one, and in front of the arc's dashes so they never
    // break it up. the moon goes on after, so it passes behind
    if (frame->night && frame->sky.celestial != CEL_HIDDEN)
    {
        draw_shooting_star(ctx, pal, frame->anim);
    }

    if (frame->sky.celestial != CEL_HIDDEN)
    {
        draw_disc(ctx, pal, frame->disc, frame->night, frame->anim);
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
