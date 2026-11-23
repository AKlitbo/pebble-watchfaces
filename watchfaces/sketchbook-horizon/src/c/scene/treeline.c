/**
 * @file treeline.c
 * @brief The drawn forest: the two treelines, the cabin, and the smoke the wind leans over. Its
 * stars and where its sky and weather sit are here too.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/treeline.h"

#include "scene/reading.h"
#include "fx/fx.h"
#include "sky/sky.h"
#include "io/stores/weather_store.h"
#include "draw/line.h"
#include "draw/letter.h"
#include "math/pct.h"
#include "math/scale.h"
#include "weather/wind_dir.h"
#include "theme/treeline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// the wind speed at which the plume is bent as far over as it goes. past this it is already
// flat, and a gale and a storm look the same from a chimney
#define WIND_FULL_KMH 40

// where the treelines stand and where the clearing floor starts. only what this scene draws for
// itself lives here. where the sky, the weather and the arc sit is in the geometry below
//
// the round screen is 30% wider and only 14% taller, so the forest is not a straight scale of the
// rectangle: the treelines spread to the new width and drop far enough that the clearing still
// has the clock's whole band under them
#if defined(PBL_ROUND)
#define FAR_BASE 126    ///< Where the back treeline stands
#define NEAR_BASE 145   ///< Where the front treeline and the cabin stand
#define GROUND_Y 138    ///< Top of the clearing floor. the near trunks stand down into it
#else
#define FAR_BASE 113
#define NEAR_BASE 129
#define GROUND_Y 123
#endif

// the cabin, measured off the treeline it stands in rather than pinned to rows of its own, so
// moving the treeline moves the whole building instead of squashing it
#if defined(PBL_ROUND)
#define CABIN_LEFT 20
#define CABIN_RIGHT 96
#define CABIN_WALL_TOP (NEAR_BASE - 38)
#define CABIN_RIDGE (CABIN_WALL_TOP - 18)
#define STACK_X 78
#define CABIN_DOOR_DX 13
#define CABIN_DOOR_W 14
#define CABIN_DOOR_H 18
#define CABIN_WIN_DX 42
#define CABIN_WIN_W 20
#define CABIN_WIN_H 14
// the board the temperature is lettered on, nailed across the front above the door and window.
// the cabin is this scene's one built thing, so the reading rides it the way shoreline's rides
// the boat rather than standing on a post of its own
#define CABIN_BOARD_INSET 4
#define CABIN_BOARD_H 17
#else
#define CABIN_LEFT 14
#define CABIN_RIGHT 74
#define CABIN_WALL_TOP (NEAR_BASE - 26)
#define CABIN_RIDGE (CABIN_WALL_TOP - 15)
#define STACK_X 60
#define CABIN_DOOR_DX 8
#define CABIN_DOOR_W 11
#define CABIN_DOOR_H 15
#define CABIN_WIN_DX 30
#define CABIN_WIN_W 16
#define CABIN_WIN_H 13
#endif
#define CABIN_MID ((CABIN_LEFT + CABIN_RIGHT) / 2)
#define STACK_TOP (CABIN_RIDGE - 5)
#define STACK_BOTTOM (CABIN_WALL_TOP - 1)

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

// --- Where this scene's sky and weather sit ---
// the baseline sits in the far treeline rather than below it, so the arc's ends tuck into the
// trunks the way shoreline's land on its sea horizon, and it stays a long shallow stroke. the
// overcast band hangs across the sky above the trees, and the fork strikes out of it
#if defined(PBL_ROUND)
static const Puff s_overcast[] = {
    {18, 68, 33, 11}, {75, 64, 30, 12}, {135, 71, 34, 11}, {195, 65, 31, 12}, {238, 72, 28, 10},
};
static const GPoint s_bolt[][2] = {
    {{151, 66}, {127, 92}}, {{127, 92}, {148, 89}}, {{148, 89}, {116, 120}},
};
const SceneGeometry treeline_geometry = {
    .sky_band_y = 56, .sky_blend_h = 28,
    .arc_base_y = 118, .arc_rx = 112, .arc_ry = 55,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 22, .cloud_y_min = 62, .cloud_y_max = 86,
    .drift_lobe_a = 10, .drift_lobe_b = 11,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 40, .precip_h = 105,
    .wash_top = 92, .wash_bottom = 145, .wash_wisp_hi_y = 115, .wash_wisp_lo_y = 128,
    .bolt_stroke = s_bolt, .bolt_count = ARRAY_LENGTH(s_bolt),
};
#else
static const Puff s_overcast[] = {
    {14, 56, 26, 9}, {58, 53, 24, 9}, {104, 59, 27, 9}, {150, 54, 25, 9}, {192, 60, 22, 8},
};
static const GPoint s_bolt[][2] = {
    {{116, 62}, {98, 80}}, {{98, 80}, {114, 78}}, {{114, 78}, {90, 104}},
};
const SceneGeometry treeline_geometry = {
    .sky_band_y = 46, .sky_blend_h = 24,
    .arc_base_y = 108, .arc_rx = 86, .arc_ry = 72,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 18, .cloud_y_min = 54, .cloud_y_max = 66,
    .drift_lobe_a = 8, .drift_lobe_b = 9,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 30, .precip_h = 94,
    .wash_top = 81, .wash_bottom = 129, .wash_wisp_hi_y = 101, .wash_wisp_lo_y = 111,
    .bolt_stroke = s_bolt, .bolt_count = ARRAY_LENGTH(s_bolt),
};
#endif

/** @brief One fir: where it stands, how tall it is, and how wide its skirt spreads. */
typedef struct
{
    int16_t x;
    uint8_t height;
    uint8_t half_w;
    int8_t  drop; ///< Rows below its treeline's base, so the row is not stood on a ruled line
} Fir;

// the back treeline: small, crowded and even, which is what makes it read as distance
#if defined(PBL_ROUND)
static const Fir s_far[] = {
    {8, 22, 7, 0},   {26, 26, 8, 2},  {43, 20, 7, -1}, {60, 28, 9, 1},  {78, 23, 7, 0},
    {95, 27, 8, 2},  {112, 21, 7, -1}, {129, 29, 9, 1}, {146, 24, 8, 0}, {163, 27, 8, 2},
    {180, 22, 7, -1}, {197, 28, 9, 1}, {213, 23, 7, 0}, {230, 26, 8, 2}, {246, 21, 7, -1},
    {258, 24, 8, 1},
};
#else
static const Fir s_far[] = {
    {6, 20, 6, 0},  {20, 24, 7, 2},  {33, 18, 6, -1}, {46, 26, 8, 1},  {60, 21, 6, 0},
    {73, 25, 7, 2}, {86, 19, 6, -1}, {99, 27, 8, 1},  {112, 22, 7, 0}, {125, 25, 7, 2},
    {138, 20, 6, -1}, {151, 26, 8, 1}, {164, 21, 6, 0}, {177, 24, 7, 2}, {190, 19, 6, -1},
    {198, 23, 7, 1},
};
#endif

// the front treeline: bigger, sparser and staggered, and it leaves the left third clear for the
// cabin to stand in
#if defined(PBL_ROUND)
static const Fir s_near[] = {
    {4, 48, 16, 2},    {104, 50, 17, 1}, {130, 54, 18, -1}, {156, 46, 15, 3},
    {182, 56, 19, -2}, {208, 48, 16, 2}, {232, 54, 17, 0},  {256, 45, 15, 3},
};
#else
static const Fir s_near[] = {
    {2, 44, 15, 2},   {62, 46, 16, 1},  {86, 50, 17, -1}, {110, 42, 14, 3},
    {134, 52, 18, -2}, {158, 44, 15, 2}, {180, 50, 16, 0}, {198, 41, 14, 3},
};
#endif

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
static void draw_cabin(GContext *ctx, const TreelinePalette *pal, bool night, bool snow)
{
    graphics_context_set_stroke_width(ctx, 1);

    // walls
    GRect walls = GRect(CABIN_LEFT, CABIN_WALL_TOP, CABIN_RIGHT - CABIN_LEFT, NEAR_BASE - CABIN_WALL_TOP);
    graphics_context_set_fill_color(ctx, pal->cabin);
    graphics_fill_rect(ctx, walls, 0, GCornerNone);

    // the logs, a few lines across the wall rather than a drawn stack of them
    graphics_context_set_stroke_color(ctx, pal->ink);
    for (int y = CABIN_WALL_TOP + 5; y < NEAR_BASE; y += 5)
    {
        graphics_draw_line(ctx, GPoint(CABIN_LEFT + 1, y), GPoint(CABIN_RIGHT - 2, y));
    }
    graphics_draw_rect(ctx, walls);

    // the roof, overhanging both walls. filled row by row so the eaves stay square
    graphics_context_set_stroke_color(ctx, pal->roof);
    int span = CABIN_RIGHT - CABIN_LEFT + 10;
    for (int y = CABIN_RIDGE; y <= CABIN_WALL_TOP; y++)
    {
        int half = (span * (y - CABIN_RIDGE)) / (2 * (CABIN_WALL_TOP - CABIN_RIDGE));
        graphics_draw_line(ctx, GPoint(CABIN_MID - half, y), GPoint(CABIN_MID + half, y));
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_line(ctx, GPoint(CABIN_MID, CABIN_RIDGE), GPoint(CABIN_LEFT - 5, CABIN_WALL_TOP));
    graphics_draw_line(ctx, GPoint(CABIN_MID, CABIN_RIDGE), GPoint(CABIN_RIGHT + 5, CABIN_WALL_TOP));
    graphics_draw_line(ctx, GPoint(CABIN_LEFT - 5, CABIN_WALL_TOP), GPoint(CABIN_RIGHT + 5, CABIN_WALL_TOP));

    if (snow)
    {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 3);
        graphics_draw_line(ctx, GPoint(CABIN_MID - 2, CABIN_RIDGE + 2), GPoint(CABIN_LEFT - 3, CABIN_WALL_TOP - 1));
        graphics_draw_line(ctx, GPoint(CABIN_MID + 2, CABIN_RIDGE + 2), GPoint(CABIN_RIGHT + 3, CABIN_WALL_TOP - 1));
        graphics_context_set_stroke_width(ctx, 1);
    }

    // the door
    graphics_context_set_fill_color(ctx, pal->roof);
    GRect door = GRect(CABIN_LEFT + CABIN_DOOR_DX, NEAR_BASE - CABIN_DOOR_H, CABIN_DOOR_W, CABIN_DOOR_H);
    graphics_fill_rect(ctx, door, 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_rect(ctx, door);

    // and the window, which is the whole point of the cabin after dark
    GRect pane = GRect(CABIN_LEFT + CABIN_WIN_DX, NEAR_BASE - CABIN_WIN_H - 4, CABIN_WIN_W, CABIN_WIN_H);
    graphics_context_set_fill_color(ctx, night ? pal->glow : pal->roof);
    graphics_fill_rect(ctx, pane, 0, GCornerNone);
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
static void draw_smoke(GContext *ctx, const TreelinePalette *pal, int lean, int strength)
{
    // the chimney itself, standing proud of the roof
    graphics_context_set_fill_color(ctx, pal->roof);
    graphics_fill_rect(ctx, GRect(STACK_X - 4, STACK_TOP, 8, STACK_BOTTOM - STACK_TOP), 0, GCornerNone);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_rect(ctx, GRect(STACK_X - 4, STACK_TOP, 8, STACK_BOTTOM - STACK_TOP));

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
        int y = STACK_TOP - rise;

        if (y - radius < 20)
        {
            break;  // gone into the status bar, which would clip it flat
        }

        graphics_fill_circle(ctx, GPoint(STACK_X + drift + shake, y), radius);
    }
}

#if defined(PBL_ROUND)
/**
 * @brief Letter the temperature on the board nailed across the cabin front.
 *
 * Drawn after the weather rather than with the cabin, because the reading is the one thing on the
 * face that has to stay readable in every condition. Painted with the building it sits behind the
 * rain, and a heavy fall or a fog bank crosses straight over the number.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_temp_board(GContext *ctx, const TreelinePalette *pal)
{
    // no location and no reading means no board at all, rather than a plaque lettered "--".
    // a blank cabin front is a cabin, an empty sign is a fault
    char temp[16];
    int reading;
    if (!reading_temp(temp, sizeof(temp), &reading))
    {
        return;
    }

    // sized to the reading rather than the wall, so it reads as a plaque over the door and leaves
    // the logs either side of it showing. a floor keeps a short reading from looking like a chip,
    // and a ceiling keeps "120F" from running into the eaves
    int board_w = clamp_int(reading + 14, 44, CABIN_RIGHT - CABIN_LEFT - CABIN_BOARD_INSET * 2);

    GRect board = GRect(CABIN_MID - board_w / 2, CABIN_WALL_TOP + 2, board_w, CABIN_BOARD_H);

    // filled with the sky and lettered in ink, because ink over sky is the one pairing the palette
    // guarantees. filling it with the wall instead would rest on cabin-against-ink holding up in
    // every theme, which is the sort of assumption that leaves a reading invisible on half of them
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_fill_rect(ctx, board, 0, GCornerNone);
    graphics_draw_rect(ctx, board);
    letter_board(ctx, temp, board, pal->ink);
}
#endif

/** @brief Lay down the clearing floor the clock sits on. */
static void draw_ground(GContext *ctx, GRect bounds, const TreelinePalette *pal)
{
    graphics_context_set_fill_color(ctx, pal->ground);
    graphics_fill_rect(ctx, GRect(0, GROUND_Y, bounds.size.w, bounds.size.h - GROUND_Y), 0, GCornerNone);
}

#if defined(PBL_ROUND)
// fallen needles on the clearing floor, lying flat rather than standing up, which is what keeps
// them reading as forest litter instead of grass. a fixed scatter, so the floor is the same every
// time you look at it
//
// nothing sits below the clock's cap line across the middle, or the litter comes up between the
// digits. out at the sides the digits never reach, so the floor carries on down the edges there
static const GPoint s_litter[] = {
    {10, 152}, {100, 151}, {120, 147}, {140, 151},
    {160, 148}, {180, 151}, {200, 146}, {220, 150}, {240, 148},
    {22, 164}, {40, 172}, {16, 179},
    {228, 163}, {244, 171}, {220, 178},
};

// the track worn out of the doorway into the clearing. it starts at the foot of the door and
// stops short of the clock's cap line, so it never runs in among the digits
static const GPoint s_path[] = {
    {40, 147}, {50, 150}, {62, 152}, {74, 153},
};

/**
 * @brief Scatter fallen needles across the clearing.
 *
 * Drawn in the colour the palette guarantees over the ground rather than the ink the trees take,
 * because this lies on the floor rather than against the sky.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_litter(GContext *ctx, const TreelinePalette *pal)
{
    graphics_context_set_stroke_color(ctx, pal->dim);
    graphics_context_set_stroke_width(ctx, 1);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_litter); i++)
    {
        GPoint at = s_litter[i];

        // alternating lie, so the scatter does not comb one way
        int tilt = (i & 1) ? 2 : -2;
        graphics_draw_line(ctx, GPoint(at.x - 3, at.y - tilt), GPoint(at.x + 3, at.y + tilt));
        graphics_draw_line(ctx, GPoint(at.x + 1, at.y + 3), GPoint(at.x + 5, at.y + 1));
    }

    // the path, dashed 3 on and 3 off so it reads as trodden ground rather than a drawn line
    line_dashed(ctx, s_path, ARRAY_LENGTH(s_path), 3, 6);
}
#endif

void treeline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const TreelinePalette *pal = frame->palette;

    // the wind as the plume sees it: which way it pushes, and how much of the full bend the
    // speed is worth. no reading leaves the column standing straight up
    int lean = wind_lean(wind_bearing(weather_store_wind_dir()));
    int strength = pct_of(weather_store_wind_kmh(), WIND_FULL_KMH);

    // the smoke rises through the sky, so it goes down before the trees it drifts behind
    draw_smoke(ctx, pal, lean, strength);

    // snow lies where snow falls, so it rides the firs and the roof only when it is snowing
    bool snow = sketchbook_cond_snows(frame->cond);

    draw_treeline(ctx, s_far, ARRAY_LENGTH(s_far), FAR_BASE, pal->tree_far, pal->ink, snow);
    draw_ground(ctx, bounds, pal);

    draw_cabin(ctx, pal, frame->night, snow);

    draw_treeline(ctx, s_near, ARRAY_LENGTH(s_near), NEAR_BASE, pal->tree, pal->ink, snow);

#if defined(PBL_ROUND)
    draw_litter(ctx, pal);
#endif
}

#if defined(PBL_ROUND)
void treeline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_temp_board(ctx, frame->palette);
}
#endif

/** @} */
