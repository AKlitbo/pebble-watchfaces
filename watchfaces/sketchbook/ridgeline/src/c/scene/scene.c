/**
 * @file scene.c
 * @brief The drawn landscape: sky bands, stars, the sun or moon on its arc, the two ridges,
 * and the weather layered in between.
 *
 * @ingroup watchface-ridgeline
 */
#include "scene.h"

#include "sketchbook/fx/fx.h"
#include "sketchbook/sky/sky.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

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

// a fixed field rather than a random one, so the sky is the same every time you look up at it,
// which is what a sky does. it starts below the status bar and thins out towards the ridges
static const SketchbookStar s_stars[] = {
    {10, 28, 1}, {24, 40, 0}, {36, 25, 0}, {45, 34, 2}, {58, 27, 0}, {66, 44, 1},
    {78, 32, 0}, {88, 24, 1}, {97, 38, 0}, {108, 29, 2}, {119, 45, 0}, {129, 26, 1},
    {140, 36, 0}, {150, 23, 0}, {161, 41, 1}, {172, 30, 0}, {184, 44, 2}, {194, 27, 0},
    {16, 55, 0}, {31, 66, 1}, {50, 58, 0}, {63, 71, 0}, {83, 60, 1}, {94, 70, 0},
    {112, 57, 0}, {127, 68, 1}, {141, 55, 0}, {156, 70, 0}, {176, 59, 1}, {190, 69, 0},
    {6, 36, 0}, {71, 53, 0}, {103, 48, 0}, {167, 52, 0},
};

// snow lies where snow falls, so it caps the peaks on the two snow rows and nothing else
#define RIDGELINE_SNOWCAP_ROWS ((1u << SKETCHBOOK_COND_SNOW) | (1u << SKETCHBOOK_COND_SNSH))

_Static_assert(RIDGELINE_SNOWCAP_ROWS == ((1u << SKETCHBOOK_COND_SNOW) | (1u << SKETCHBOOK_COND_SNSH)),
    "snow caps a peak where snow falls: exactly the two snow rows, no more and no fewer");

/** @brief Whether this condition caps the peaks in snow. */
static bool snowcap(SketchbookCond cond)
{
    return (RIDGELINE_SNOWCAP_ROWS >> cond) & 1u;
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
    bool night = sketchbook_sky_night();
    SketchbookCond cond = sketchbook_cond_parse(weather_store_cond());
    SketchbookSky sky = sketchbook_sky_recipe(cond);
    GPoint disc = sketchbook_sky_arc_point(sketchbook_sky_arc_progress());

    graphics_context_set_antialiased(ctx, true);

    sketchbook_sky_draw_bands(ctx, bounds, pal);

    if (night)
    {
        sketchbook_sky_draw_stars(ctx, pal, s_stars, ARRAY_LENGTH(s_stars));
    }

    sketchbook_sky_draw_arc(ctx, pal);

    if (sky.celestial != CEL_HIDDEN)
    {
        sketchbook_sky_draw_disc(ctx, pal, disc, night);
    }

    // clouds sit in front of the disc but behind the ridges, so a low sun still sets behind
    // the mountains
    sketchbook_fx_draw_clouds(ctx, pal, &sky, disc);

    draw_ridge(ctx, s_far_path, s_far_pts, FAR_CREST, pal->land_far, pal->ink, snowcap(cond), 12);
    draw_ridge(ctx, s_near_path, s_near_pts, NEAR_CREST, pal->land, pal->ink, snowcap(cond), 20);

    // rain falls in front of everything: it is the nearest thing to you
    sketchbook_fx_draw_precip(ctx, pal, &sky, time_store_tm()->tm_min);

    if (sky.wash)
    {
        sketchbook_fx_draw_wash(ctx, pal);
    }

    if (sky.bolt)
    {
        sketchbook_fx_draw_bolt(ctx, pal);
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
