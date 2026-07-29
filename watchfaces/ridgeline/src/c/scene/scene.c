/**
 * @file scene.c
 * @brief The drawn landscape: sky bands, stars, the sun or moon on its arc, the two ridges,
 * and the weather layered in between.
 *
 * @ingroup watchface-ridgeline
 */
#include "scene.h"

#include "weather_fx.h"
#include "clock/clockstr.h"
#include "clock/moon.h"
#include "clock/solar.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

// the day to fall back on before the phone has sent a sunrise and sunset. a plain twelve-hour
// day is wrong by an hour or two rather than wrong by half a day, so the scene still reads
#define FALLBACK_RISE (6 * 60)
#define FALLBACK_SET (18 * 60)

// --- Ridges ---
// sharp apexes with a steep face one side and a longer run-out the other, which is what makes
// a line read as a mountain rather than a hill. each array ends with two off-screen corners
// that close the polygon for the fill, and the crest count is how many points the outline walks
//
// the far range peaks in the middle and the two flanks are held down, because the arc crosses
// x=40 and x=170 while the sun is still well clear of its horizon. a flank as tall as the
// centre swallows the disc a couple of hours either side of the real sunrise and sunset
#define FAR_CREST 14
static GPoint s_far_pts[] = {
    {-4, 104}, {14, 86}, {26, 98}, {40, 84}, {58, 96}, {72, 82}, {88, 102},
    {104, 70}, {122, 94}, {136, 80}, {154, 100}, {170, 84}, {188, 92}, {204, 86},
    {204, 232}, {-4, 232},
};
static GPathInfo s_far_info = {ARRAY_LENGTH(s_far_pts), s_far_pts};

// the near range is bigger and sparser, built around one dominant central ridgeline
#define NEAR_CREST 11
static GPoint s_near_pts[] = {
    {-4, 126}, {18, 112}, {34, 124}, {54, 100}, {76, 118}, {100, 96},
    {124, 116}, {144, 106}, {162, 122}, {182, 110}, {204, 120},
    {204, 232}, {-4, 232},
};
static GPathInfo s_near_info = {ARRAY_LENGTH(s_near_pts), s_near_pts};

static GPath *s_far_path;
static GPath *s_near_path;

/** @brief One star: where it is and how big it burns. */
typedef struct
{
    int16_t x;
    int16_t y;
    uint8_t size; ///< 0 a faint point, 1 a brighter dot, 2 a twinkling cross
} Star;

// a fixed field rather than a random one, so the sky is the same every time you look up at it,
// which is what a sky does. it starts below the status bar and thins out towards the ridges
static const Star s_stars[] = {
    {10, 28, 1}, {24, 40, 0}, {36, 25, 0}, {45, 34, 2}, {58, 27, 0}, {66, 44, 1},
    {78, 32, 0}, {88, 24, 1}, {97, 38, 0}, {108, 29, 2}, {119, 45, 0}, {129, 26, 1},
    {140, 36, 0}, {150, 23, 0}, {161, 41, 1}, {172, 30, 0}, {184, 44, 2}, {194, 27, 0},
    {16, 55, 0}, {31, 66, 1}, {50, 58, 0}, {63, 71, 0}, {83, 60, 1}, {94, 70, 0},
    {112, 57, 0}, {127, 68, 1}, {141, 55, 0}, {156, 70, 0}, {176, 59, 1}, {190, 69, 0},
    {6, 36, 0}, {71, 53, 0}, {103, 48, 0}, {167, 52, 0},
};

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

bool scene_night(void)
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
static int arc_progress(void)
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
static GPoint arc_point(int progress)
{
    int32_t angle = DEG_TO_TRIGANGLE(180 * progress / 100);

    return GPoint(ARC_CX - (ARC_RX * cos_lookup(angle)) / TRIG_MAX_RATIO,
                  HORIZON_Y - (ARC_RY * sin_lookup(angle)) / TRIG_MAX_RATIO);
}

/**
 * @brief Fill the sky: the upper band, the lower band, and a stippled blend between them.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 */
static void draw_sky(GContext *ctx, GRect bounds, const Palette *pal)
{
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    int blend_end = SKY_BAND_Y + SKY_BLEND_H;
    graphics_context_set_fill_color(ctx, pal->sky_lo);
    graphics_fill_rect(ctx, GRect(0, blend_end, bounds.size.w, bounds.size.h - blend_end), 0, GCornerNone);

    // the two bands butted straight together read as a stripe ruled across the sky, so the
    // lower one is stippled in over the upper instead, thickening row by row until it takes
    // over. eight levels is all the dither the 64-colour panel can show anyway
    graphics_context_set_stroke_color(ctx, pal->sky_lo);
    for (int y = SKY_BAND_Y; y < blend_end; y++)
    {
        int level = ((y - SKY_BAND_Y) * 8) / SKY_BLEND_H;
        for (int x = 0; x < bounds.size.w; x++)
        {
            // offsetting the pattern per row stops the kept pixels stacking into columns
            if ((x + (y - SKY_BAND_Y) * 3) % 8 < level)
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
static void draw_stars(GContext *ctx, const Palette *pal)
{
    graphics_context_set_stroke_width(ctx, 1);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_stars); i++)
    {
        GPoint at = GPoint(s_stars[i].x, s_stars[i].y);

        // the faint ones sit back in the dim colour and the bright ones come forward in the
        // disc colour, which is what stops the field reading as one flat spray of dots
        GColor color = s_stars[i].size == 0 ? pal->dim : pal->disc;
        graphics_context_set_fill_color(ctx, color);
        graphics_context_set_stroke_color(ctx, color);

        if (s_stars[i].size == 2)
        {
            graphics_draw_line(ctx, GPoint(at.x - 3, at.y), GPoint(at.x + 3, at.y));
            graphics_draw_line(ctx, GPoint(at.x, at.y - 3), GPoint(at.x, at.y + 3));
            graphics_fill_circle(ctx, at, 1);
        }
        else
        {
            graphics_fill_circle(ctx, at, s_stars[i].size);
        }
    }
}

/**
 * @brief Draw the arc the disc travels, as a broken guide line.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_arc(GContext *ctx, const Palette *pal)
{
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);

    // every other span is skipped, which dashes the line without needing a dash routine
    for (int p = 0; p < 100; p += 8)
    {
        graphics_draw_line(ctx, arc_point(p), arc_point(p + 4));
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
        int inner = DISC_R + 4;
        int outer = inner + ray_len[i];

        graphics_draw_line(ctx,
            GPoint(at.x + (inner * sin_a) / TRIG_MAX_RATIO, at.y - (inner * cos_a) / TRIG_MAX_RATIO),
            GPoint(at.x + (outer * sin_a) / TRIG_MAX_RATIO, at.y - (outer * cos_a) / TRIG_MAX_RATIO));
    }

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, DISC_R);

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_circle(ctx, at, DISC_R);
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
    GColor behind = at.y < SKY_BAND_Y ? pal->sky_hi : pal->sky_lo;
    int phase = moon_glyph_index(time(NULL), 8);

    graphics_context_set_fill_color(ctx, pal->disc);
    graphics_fill_circle(ctx, at, DISC_R);

    graphics_context_set_fill_color(ctx, behind);
    switch (phase)
    {
        case 0:  // new: nothing lit, so only the outline is left behind
            graphics_fill_circle(ctx, at, DISC_R);
            break;
        case 1:  // waxing crescent: lit down the right edge
            graphics_fill_circle(ctx, GPoint(at.x - (DISC_R * 6) / 10, at.y), DISC_R);
            break;
        case 2:  // first quarter: the left half is dark
            graphics_fill_rect(ctx, GRect(at.x - DISC_R, at.y - DISC_R, DISC_R, DISC_R * 2 + 1), 0, GCornerNone);
            break;
        case 3:  // waxing gibbous: only a sliver of the left edge is dark
            graphics_fill_circle(ctx, GPoint(at.x - (DISC_R * 14) / 10, at.y), DISC_R);
            break;
        case 5:  // waning gibbous
            graphics_fill_circle(ctx, GPoint(at.x + (DISC_R * 14) / 10, at.y), DISC_R);
            break;
        case 6:  // last quarter: the right half is dark
            graphics_fill_rect(ctx, GRect(at.x + 1, at.y - DISC_R, DISC_R, DISC_R * 2 + 1), 0, GCornerNone);
            break;
        case 7:  // waning crescent: lit down the left edge
            graphics_fill_circle(ctx, GPoint(at.x + (DISC_R * 6) / 10, at.y), DISC_R);
            break;
        default: // full: the whole disc stays lit
            break;
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_circle(ctx, at, DISC_R);
    graphics_context_set_stroke_width(ctx, 1);
}

/**
 * @brief Where the crest sits at a given x, interpolated along the polyline.
 *
 * @param pts The crest points, left to right.
 * @param crest How many points there are.
 * @param x The column to read.
 * @return The crest's y at that column.
 */
static int crest_y_at(const GPoint *pts, int crest, int x)
{
    if (x <= pts[0].x)
    {
        return pts[0].y;
    }

    for (int i = 0; i < crest - 1; i++)
    {
        if (x <= pts[i + 1].x)
        {
            int span = pts[i + 1].x - pts[i].x;
            int rise = pts[i + 1].y - pts[i].y;
            return pts[i].y + (span ? (rise * (x - pts[i].x)) / span : 0);
        }
    }

    return pts[crest - 1].y;
}

/**
 * @brief Draw the spurs running down off each ridgeline.
 *
 * A filled silhouette with one outline on top reads as a paper cut-out. The creases that fall
 * away from an apex are what give it faces, and they are the whole trick behind a line-drawn
 * mountain looking like rock.
 *
 * @param ctx The graphics context.
 * @param pts The crest points.
 * @param crest How many points there are.
 */
static void draw_facets(GContext *ctx, const GPoint *pts, int crest)
{
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 1; i < crest - 1; i++)
    {
        // a ridgeline is a point lower than both its neighbours. the saddles get nothing, so the
        // creases only ever hang off a peak
        if (pts[i].y >= pts[i - 1].y || pts[i].y >= pts[i + 1].y)
        {
            continue;
        }

        int drop = (pts[i - 1].y + pts[i + 1].y) / 2 - pts[i].y;  // how far the ridgeline stands up
        if (drop < 6)
        {
            continue;  // too shallow to be worth creasing
        }

        graphics_draw_line(ctx, pts[i], GPoint(pts[i].x - drop / 2 - 3, pts[i].y + drop + 8));
        graphics_draw_line(ctx, pts[i], GPoint(pts[i].x + drop / 3 + 4, pts[i].y + drop + 2));
    }
}

/**
 * @brief Cap each ridgeline in snow, with a ragged lower edge.
 *
 * Drawn column by column: the cap is deepest at the apex and tapers out to nothing down the
 * slopes, and a repeating offset roughens the bottom so the snowline is torn rather than
 * ruled.
 *
 * @param ctx The graphics context.
 * @param pts The crest points.
 * @param crest How many points there are.
 * @param span How far either side of an apex the snow reaches.
 */
static void draw_snowcaps(GContext *ctx, const GPoint *pts, int crest, int span)
{
    static const int8_t ragged[] = {0, 3, -2, 4, 1, -1, 3, -3, 2, 0, 4, -2};

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 1; i < crest - 1; i++)
    {
        if (pts[i].y >= pts[i - 1].y || pts[i].y >= pts[i + 1].y)
        {
            continue;
        }

        for (int x = pts[i].x - span; x <= pts[i].x + span; x++)
        {
            int from_apex = x > pts[i].x ? x - pts[i].x : pts[i].x - x;
            int depth = ((span - from_apex) * 9) / span + ragged[(unsigned)(x + i) % ARRAY_LENGTH(ragged)];
            if (depth < 1)
            {
                continue;
            }

            int top = crest_y_at(pts, crest, x);
            graphics_draw_line(ctx, GPoint(x, top), GPoint(x, top + depth));
        }
    }
}

/**
 * @brief Fill one ridge, crease it, cap it, and stroke its crest.
 *
 * @param ctx The graphics context.
 * @param path The closed polygon for the body.
 * @param pts The crest points.
 * @param crest How many of the points make up the crest.
 * @param body The body fill colour.
 * @param line The outline colour.
 * @param snow True to cap the summits in snow.
 * @param snow_span How far either side of an apex the snow reaches.
 */
static void draw_ridge(GContext *ctx, GPath *path, const GPoint *pts, int crest,
    GColor body, GColor line, bool snow, int snow_span)
{
    if (!path)
    {
        return;
    }

    graphics_context_set_fill_color(ctx, body);
    gpath_draw_filled(ctx, path);

    graphics_context_set_stroke_color(ctx, line);
    draw_facets(ctx, pts, crest);

    if (snow)
    {
        draw_snowcaps(ctx, pts, crest, snow_span);
    }

    // the crest goes on last so it sits cleanly over both the creases and the snow
    graphics_context_set_stroke_color(ctx, line);
    graphics_context_set_stroke_width(ctx, 3);
    for (int i = 0; i < crest - 1; i++)
    {
        graphics_draw_line(ctx, pts[i], pts[i + 1]);
    }

    graphics_context_set_stroke_width(ctx, 1);
}

void scene_init(void)
{
    s_far_path = gpath_create(&s_far_info);
    s_near_path = gpath_create(&s_near_info);
}

void scene_draw(GContext *ctx, GRect bounds, const Palette *pal)
{
    bool night = scene_night();
    SkyRecipe recipe = fx_recipe(weather_store_cond());
    GPoint disc = arc_point(arc_progress());

    graphics_context_set_antialiased(ctx, true);

    draw_sky(ctx, bounds, pal);

    if (night)
    {
        draw_stars(ctx, pal);
    }

    draw_arc(ctx, pal);

    if (recipe.celestial != CEL_HIDDEN)
    {
        if (night)
        {
            draw_moon(ctx, pal, disc);
        }
        else
        {
            draw_sun(ctx, pal, disc);
        }
    }

    // clouds sit in front of the disc but behind the ridges, so a low sun still sets behind
    // the mountains
    fx_draw_clouds(ctx, pal, &recipe, disc);

    draw_ridge(ctx, s_far_path, s_far_pts, FAR_CREST, pal->land_far, pal->ink, recipe.snowcap, 12);
    draw_ridge(ctx, s_near_path, s_near_pts, NEAR_CREST, pal->land, pal->ink, recipe.snowcap, 20);

    // rain falls in front of everything: it is the nearest thing to you
    fx_draw_precip(ctx, pal, &recipe, time_store_tm()->tm_min);

    if (recipe.fog)
    {
        fx_draw_fog(ctx, pal);
    }

    if (recipe.bolt)
    {
        fx_draw_bolt(ctx, pal);
    }
}

void scene_deinit(void)
{
    gpath_destroy(s_far_path);
    s_far_path = NULL;
    gpath_destroy(s_near_path);
    s_near_path = NULL;
}

/** @} */
