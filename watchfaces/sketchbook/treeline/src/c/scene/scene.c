/**
 * @file scene.c
 * @brief The drawn forest: sky bands, stars, the sun or moon on its arc, the two treelines,
 * the cabin, and the smoke the wind leans over.
 *
 * @ingroup watchface-treeline
 */
#include "scene.h"

#include "sketchbook/fx/fx.h"
#include "sketchbook/sky/sky.h"
#include "weather/wind_dir.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup watchface-treeline
 * @{
 */

// the platform says how wide it is, so the sea and the shore run to whichever screen they are on
// rather than stopping at 200 and leaving a straight edge partway across a round one
#define SCREEN_W PBL_DISPLAY_WIDTH

// the wind speed at which the plume is bent as far over as it goes. past this it is already
// flat, and a gale and a storm look the same from a chimney
#define WIND_FULL_KMH 40

// the cabin, measured off the treeline it stands in rather than pinned to rows of its own, so
// moving the treeline moves the whole building instead of squashing it
#define CABIN_LEFT 14
#define CABIN_RIGHT 74
#define CABIN_MID ((CABIN_LEFT + CABIN_RIGHT) / 2)
#define CABIN_WALL_TOP (NEAR_BASE - 26)
#define CABIN_RIDGE (CABIN_WALL_TOP - 15)
#define STACK_X 60
#define STACK_TOP (CABIN_RIDGE - 5)
#define STACK_BOTTOM (CABIN_WALL_TOP - 1)

/** @brief One fir: where it stands, how tall it is, and how wide its skirt spreads. */
typedef struct
{
    int16_t x;
    uint8_t height;
    uint8_t half_w;
    int8_t  drop; ///< Rows below its treeline's base, so the row is not stood on a ruled line
} Fir;

// a fixed field rather than a random one, so the sky is the same every time you look up at it,
// which is what a sky does. it starts below the status bar and thins out towards the trees
static const SketchbookStar s_stars[] = {
    {10, 30, 1}, {24, 42, 0}, {36, 27, 0}, {45, 36, 2}, {58, 29, 0}, {66, 46, 1},
    {78, 34, 0}, {88, 26, 1}, {97, 40, 0}, {108, 31, 2}, {119, 47, 0}, {129, 28, 1},
    {140, 38, 0}, {150, 25, 0}, {161, 43, 1}, {172, 32, 0}, {184, 46, 2}, {194, 29, 0},
    {16, 57, 0}, {31, 68, 1}, {50, 60, 0}, {63, 73, 0}, {83, 62, 1}, {94, 72, 0},
    {112, 59, 0}, {127, 70, 1}, {141, 57, 0}, {156, 72, 0}, {176, 61, 1}, {190, 71, 0},
    {6, 38, 0}, {71, 55, 0}, {103, 50, 0}, {167, 54, 0},
};

// the back treeline: small, crowded and even, which is what makes it read as distance
static const Fir s_far[] = {
    {6, 20, 6, 0},  {20, 24, 7, 2},  {33, 18, 6, -1}, {46, 26, 8, 1},  {60, 21, 6, 0},
    {73, 25, 7, 2}, {86, 19, 6, -1}, {99, 27, 8, 1},  {112, 22, 7, 0}, {125, 25, 7, 2},
    {138, 20, 6, -1}, {151, 26, 8, 1}, {164, 21, 6, 0}, {177, 24, 7, 2}, {190, 19, 6, -1},
    {198, 23, 7, 1},
};

// the front treeline: bigger, sparser and staggered, and it leaves the left third clear for the
// cabin to stand in
static const Fir s_near[] = {
    {2, 44, 15, 2},   {62, 46, 16, 1},  {86, 50, 17, -1}, {110, 42, 14, 3},
    {134, 52, 18, -2}, {158, 44, 15, 2}, {180, 50, 16, 0}, {198, 41, 14, 3},
};

// snow lies where snow falls, so it rides the two snow rows and nothing else. sleet is
// left off: it does not settle on a fir
#define TREELINE_SNOWY_ROWS ((1u << SKETCHBOOK_COND_SNOW) | (1u << SKETCHBOOK_COND_SNSH))

_Static_assert(TREELINE_SNOWY_ROWS == ((1u << SKETCHBOOK_COND_SNOW) | (1u << SKETCHBOOK_COND_SNSH)),
    "snow lies where snow falls: exactly the two snow rows, no more and no fewer");

/** @brief Whether this condition lies snow along the firs. */
static bool snowy(SketchbookCond cond)
{
    return (TREELINE_SNOWY_ROWS >> cond) & 1u;
}

/**
 * @brief How wide a fir is, a given number of rows down from its tip.
 *
 * Three skirts, and each one starts back in at half the width the one above it reached. That
 * step is the whole shape: without it a fir is a plain triangle, and drawing the skirts as
 * three overlapping triangles instead leaves six near-parallel edges per tree, which at this
 * size reads as scribble rather than as a wood.
 *
 * @param row Rows below the tip.
 * @param height The tree's full height.
 * @param half_w Half its width at the foot.
 * @return Half its width at that row.
 */
static int fir_half(int row, int height, int half_w)
{
    int span = height / 3;
    if (span < 1)
    {
        span = 1;
    }

    int tier = row / span;
    if (tier > 2)
    {
        tier = 2;
    }

    int reaches = (half_w * (tier + 1)) / 3;      // where this skirt ends
    int starts = tier == 0 ? 0 : (half_w * tier) / 6;  // half of where the last one did
    int into = row - tier * span;

    return starts + ((reaches - starts) * into) / span;
}

/**
 * @brief Draw one fir: a filled silhouette, an inked edge, and a trunk.
 *
 * The edge is walked row to row rather than stamped a pixel at a time, so the horizontal jump
 * where one skirt overhangs the next is drawn too. Left as loose pixels those notches break the
 * outline exactly where the eye is looking for it.
 *
 * @param ctx The graphics context.
 * @param fir The tree to draw.
 * @param base The treeline's base row, before the tree's own drop.
 * @param body The fill colour.
 * @param line The outline colour.
 * @param snow True to lay snow along the skirts.
 */
static void draw_fir(GContext *ctx, const Fir *fir, int base, GColor body, GColor line, bool snow)
{
    int foot = base + fir->drop;
    int top = foot - fir->height;
    int height = fir->height;

    graphics_context_set_stroke_width(ctx, 1);

    graphics_context_set_stroke_color(ctx, body);
    for (int row = 0; row <= height; row++)
    {
        int half = fir_half(row, height, fir->half_w);
        graphics_draw_line(ctx, GPoint(fir->x - half, top + row), GPoint(fir->x + half, top + row));
    }

    graphics_context_set_stroke_color(ctx, line);
    int previous = 0;
    for (int row = 0; row <= height; row++)
    {
        int half = fir_half(row, height, fir->half_w);
        int y = top + row;

        graphics_draw_line(ctx, GPoint(fir->x - previous, y - 1), GPoint(fir->x - half, y));
        graphics_draw_line(ctx, GPoint(fir->x + previous, y - 1), GPoint(fir->x + half, y));
        previous = half;
    }

    // a stub of trunk under the lowest skirt, which is all of it that shows
    graphics_draw_line(ctx, GPoint(fir->x, foot), GPoint(fir->x, foot + 1));

    if (!snow)
    {
        return;
    }

    // snow catches on the top of each skirt, which is the only part of a fir that holds it. laid
    // on one side only, so the whole stand reads as lit from the same quarter
    graphics_context_set_stroke_color(ctx, GColorWhite);
    for (int tier = 1; tier <= 3; tier++)
    {
        int row = (height * tier) / 3 - 1;
        int half = fir_half(row, height, fir->half_w);
        graphics_draw_line(ctx, GPoint(fir->x - half + 1, top + row),
                                GPoint(fir->x - half / 3, top + row - 2));
    }
}

/** @brief Draw a whole treeline, back to front. */
static void draw_treeline(GContext *ctx, const Fir *firs, int count, int base,
    GColor body, GColor line, bool snow)
{
    for (int i = 0; i < count; i++)
    {
        draw_fir(ctx, &firs[i], base, body, line, snow);
    }
}

/**
 * @brief Draw the cabin: log walls, a deep roof, a door and a window.
 *
 * The window is the one thing on the face that answers to nothing but the hour. It is dark all
 * day and lit after sunset, which is what makes the clearing feel occupied rather than drawn.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param night True once the sun is down, which lights the window.
 * @param snow True to lay snow along the roof.
 */
static void draw_cabin(GContext *ctx, const Palette *pal, bool night, bool snow)
{
    const int left = CABIN_LEFT, right = CABIN_RIGHT, base = NEAR_BASE;
    const int wall_top = CABIN_WALL_TOP, ridge = CABIN_RIDGE;

    graphics_context_set_stroke_width(ctx, 1);

    // walls
    graphics_context_set_fill_color(ctx, pal->cabin);
    graphics_fill_rect(ctx, GRect(left, wall_top, right - left, base - wall_top), 0, GCornerNone);

    // the logs, a few lines across the wall rather than a drawn stack of them
    graphics_context_set_stroke_color(ctx, pal->ink);
    for (int y = wall_top + 5; y < base; y += 5)
    {
        graphics_draw_line(ctx, GPoint(left + 1, y), GPoint(right - 2, y));
    }
    graphics_draw_rect(ctx, GRect(left, wall_top, right - left, base - wall_top));

    // the roof, overhanging both walls. filled row by row so the eaves stay square
    graphics_context_set_stroke_color(ctx, pal->roof);
    int span = right - left + 10;
    for (int y = ridge; y <= wall_top; y++)
    {
        int half = (span * (y - ridge)) / (2 * (wall_top - ridge));
        graphics_draw_line(ctx, GPoint(left + (right - left) / 2 - half, y),
                                GPoint(left + (right - left) / 2 + half, y));
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_line(ctx, GPoint(left + (right - left) / 2, ridge), GPoint(left - 5, wall_top));
    graphics_draw_line(ctx, GPoint(left + (right - left) / 2, ridge), GPoint(right + 5, wall_top));
    graphics_draw_line(ctx, GPoint(left - 5, wall_top), GPoint(right + 5, wall_top));

    if (snow)
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, GPoint(left + (right - left) / 2 - 2, ridge + 2), GPoint(left - 3, wall_top - 1));
        graphics_draw_line(ctx, GPoint(left + (right - left) / 2 + 2, ridge + 2), GPoint(right + 3, wall_top - 1));
        graphics_context_set_stroke_width(ctx, 1);
    }

    // the door
    graphics_context_set_fill_color(ctx, pal->roof);
    graphics_fill_rect(ctx, GRect(left + 8, base - 15, 11, 15), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_rect(ctx, GRect(left + 8, base - 15, 11, 15));

    // and the window, which is the whole point of the cabin after dark
    GRect pane = GRect(left + 30, base - 17, 16, 13);
    graphics_context_set_fill_color(ctx, night ? pal->glow : pal->roof);
    graphics_fill_rect(ctx, pane, 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_rect(ctx, pane);
    graphics_draw_line(ctx, GPoint(pane.origin.x + pane.size.w / 2, pane.origin.y),
                            GPoint(pane.origin.x + pane.size.w / 2, pane.origin.y + pane.size.h - 1));
    graphics_draw_line(ctx, GPoint(pane.origin.x, pane.origin.y + pane.size.h / 2),
                            GPoint(pane.origin.x + pane.size.w - 1, pane.origin.y + pane.size.h / 2));
}

/**
 * @brief Draw the chimney and the smoke coming off it.
 *
 * The plume is the one part of the scene the wind reading drives. It leaves the stack upright
 * and bends over as it rises, because that is what the wind gets hold of: the drift grows with
 * the square of how far up a puff has got, so a light air only tips the top of the column while
 * a gale lays the whole thing flat.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param lean Which way and how hard the wind pushes, -100 to 100.
 * @param strength How much of the full bend the wind speed is worth, 0 to 100.
 */
static void draw_smoke(GContext *ctx, const Palette *pal, int lean, int strength)
{
    const int stack_x = STACK_X, stack_top = STACK_TOP, stack_bottom = STACK_BOTTOM;

    // the chimney itself, standing proud of the roof
    graphics_context_set_fill_color(ctx, pal->roof);
    graphics_fill_rect(ctx, GRect(stack_x - 4, stack_top, 8, stack_bottom - stack_top), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_rect(ctx, GRect(stack_x - 4, stack_top, 8, stack_bottom - stack_top));

    graphics_context_set_fill_color(ctx, pal->smoke);

    // a wind blowing straight at you or straight away has no sideways part, so lean alone would
    // draw a gale out of the south exactly like a dead calm. strength gets its own say: it tears
    // the top off the column and shakes what is left, which is what a strong wind looks like
    // from any quarter
    int puffs = 8 - (strength * 3) / 100;

    // spaced closer than their own width so they run together into one plume rather than
    // beading up the sky, and swelling as the column loses its heat
    for (int puff = 0; puff < puffs; puff++)
    {
        int rise = 4 + puff * 5;
        int drift = (lean * strength * rise * rise) / 260000;
        int shake = ((puff % 2) ? strength : -strength) * puff / 220;
        int radius = 3 + (puff * 2) / 3;
        int y = stack_top - rise;

        if (y - radius < 20)
        {
            break;  // gone into the status bar, which would clip it flat
        }

        graphics_fill_circle(ctx, GPoint(stack_x + drift + shake, y), radius);
    }
}

/** @brief Lay down the clearing floor the clock sits on. */
static void draw_ground(GContext *ctx, GRect bounds, const Palette *pal)
{
    graphics_context_set_fill_color(ctx, pal->ground);
    graphics_fill_rect(ctx, GRect(0, GROUND_Y, bounds.size.w, bounds.size.h - GROUND_Y), 0, GCornerNone);
}

void scene_draw(GContext *ctx, GRect bounds, const Palette *pal)
{
    bool night = sketchbook_sky_night();
    SketchbookCond cond = sketchbook_cond_parse(weather_store_cond());
    SketchbookSky sky = sketchbook_sky_recipe(cond);
    GPoint disc = sketchbook_sky_arc_point(sketchbook_sky_arc_progress());
    int minute = time_store_tm()->tm_min;

    // the wind as the plume sees it: which way it pushes, and how much of the full bend the
    // speed is worth. no reading leaves the column standing straight up
    int lean = wind_lean(wind_bearing(weather_store_wind_dir()));
    int kmh = weather_store_wind_kmh();
    int strength = kmh < 0 ? 0 : (kmh >= WIND_FULL_KMH ? 100 : (kmh * 100) / WIND_FULL_KMH);

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

    // clouds sit in front of the disc but behind the trees, so a low sun still goes down into
    // the forest rather than in front of it
    sketchbook_fx_draw_clouds(ctx, pal, &sky, disc);

    // the smoke rises through the sky, so it goes down before the trees it drifts behind
    draw_smoke(ctx, pal, lean, strength);

    draw_treeline(ctx, s_far, ARRAY_LENGTH(s_far), FAR_BASE, pal->tree_far, pal->ink, snowy(cond));
    draw_ground(ctx, bounds, pal);

    draw_cabin(ctx, pal, night, snowy(cond));

    draw_treeline(ctx, s_near, ARRAY_LENGTH(s_near), NEAR_BASE, pal->tree, pal->ink, snowy(cond));

    // rain falls in front of everything: it is the nearest thing to you
    sketchbook_fx_draw_precip(ctx, pal, &sky, minute);

    if (sky.wash)
    {
        sketchbook_fx_draw_wash(ctx, pal);
    }

    if (sky.bolt)
    {
        sketchbook_fx_draw_bolt(ctx, pal);
    }
}

/** @} */
