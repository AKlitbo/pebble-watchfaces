/**
 * @file ridgeline.c
 * @brief The drawn landscape: the two ridges and their snow, and on a round screen the meadow,
 * the trail and its sign. Its stars and where its sky and weather sit are here too.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/ridgeline.h"

#include "scene/reading.h"
#include "fx/fx.h"
#include "sky/sky.h"
#include "draw/line.h"
#include "draw/marks.h"
#include "draw/letter.h"
#include "draw/shape.h"
#include "theme/ridgeline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- Trail sign ---
// a round screen has no room to stack a date and a reading under the clock, so the temperature
// comes off that stack and onto a signpost planted in the saddle right of the main peak. the
// board stands against the sky where it always has something to read against, and the post runs
// down into the slope below the crest
#if defined(PBL_ROUND)
#define SIGN_X 178         ///< The post's centre line
#define SIGN_BOARD_Y 108   ///< Top of the board
#define SIGN_BOARD_H 17    ///< How tall the board is
#define SIGN_BASE_Y 146    ///< Where the post meets the slope
#define SIGN_PAD 6         ///< Air either side of the reading, inside the board
#endif

// a fixed field rather than a random one, so the sky is the same every time you look up at it,
// which is what a sky does. it starts below the status bar and thins out towards the ridges
#if defined(PBL_ROUND)
// the same field spread to the wider sky. a circle cuts the corners hardest where this field is
// thickest, so the few stars that scaling put outside the glass were moved down and inwards to
// where there is sky rather than left to clip away
static const SketchbookStar s_stars[] = {
    {30, 62, 1}, {31, 54, 0}, {47, 34, 0}, {59, 46, 2}, {75, 36, 0}, {86, 59, 1},
    {101, 43, 0}, {114, 32, 1}, {126, 51, 0}, {140, 39, 2}, {155, 61, 0}, {168, 35, 1},
    {182, 49, 0}, {195, 31, 0}, {209, 55, 1}, {224, 41, 0}, {239, 59, 2}, {236, 66, 0},
    {21, 74, 0}, {40, 89, 1}, {65, 78, 0}, {82, 96, 0}, {108, 81, 1}, {122, 95, 0},
    {146, 77, 0}, {165, 92, 1}, {183, 74, 0}, {203, 95, 0}, {229, 80, 1}, {247, 93, 0},
    {24, 57, 0}, {92, 72, 0}, {134, 65, 0}, {217, 70, 0},
};
#else
static const SketchbookStar s_stars[] = {
    {10, 28, 1}, {24, 40, 0}, {36, 25, 0}, {45, 34, 2}, {58, 27, 0}, {66, 44, 1},
    {78, 32, 0}, {88, 24, 1}, {97, 38, 0}, {108, 29, 2}, {119, 45, 0}, {129, 26, 1},
    {140, 36, 0}, {150, 23, 0}, {161, 41, 1}, {172, 30, 0}, {184, 44, 2}, {194, 27, 0},
    {16, 55, 0}, {31, 66, 1}, {50, 58, 0}, {63, 71, 0}, {83, 60, 1}, {94, 70, 0},
    {112, 57, 0}, {127, 68, 1}, {141, 55, 0}, {156, 70, 0}, {176, 59, 1}, {190, 69, 0},
    {6, 36, 0}, {71, 53, 0}, {103, 48, 0}, {167, 52, 0},
};
#endif

// --- Where this scene's sky and weather sit ---
// the baseline sits on the far crest rather than down behind the ridges, so the arc's ends tuck
// into the flanks. the overcast band hangs low enough to keep its tops clear of the status bar
// and high enough to keep the whole band off the ridges, and the fork strikes out of it
#if defined(PBL_ROUND)
static const Puff s_overcast[] = {
    {18, 68, 33, 11}, {75, 64, 30, 12}, {135, 71, 34, 11}, {195, 65, 31, 12}, {238, 72, 28, 10},
};
static const GPoint s_bolt[][2] = {
    {{151, 64}, {127, 96}}, {{127, 96}, {148, 93}}, {{148, 93}, {116, 124}},
};
const SceneGeometry ridgeline_geometry = {
    .sky_band_y = 56, .sky_blend_h = 28,
    .arc_base_y = 107, .arc_rx = 112, .arc_ry = 55,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 22, .cloud_y_min = 62, .cloud_y_max = 86,
    .drift_lobe_a = 10, .drift_lobe_b = 11,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 40, .precip_h = 102,
    .wash_top = 88, .wash_bottom = 142, .wash_wisp_hi_y = 122, .wash_wisp_lo_y = 132,
    .bolt_stroke = s_bolt, .bolt_count = ARRAY_LENGTH(s_bolt),
};
#else
static const Puff s_overcast[] = {
    {14, 60, 26, 10}, {58, 56, 24, 11}, {104, 63, 27, 10}, {150, 57, 25, 11}, {192, 64, 22, 9},
};
static const GPoint s_bolt[][2] = {
    {{116, 58}, {98, 86}}, {{98, 86}, {114, 84}}, {{114, 84}, {88, 120}},
};
const SceneGeometry ridgeline_geometry = {
    .sky_band_y = 48, .sky_blend_h = 24,
    .arc_base_y = 124, .arc_rx = 86, .arc_ry = 78,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 20, .cloud_y_min = 57, .cloud_y_max = 82,
    .drift_lobe_a = 9, .drift_lobe_b = 10,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 52, .precip_h = 70,
    .wash_top = 82, .wash_bottom = 126, .wash_wisp_hi_y = 112, .wash_wisp_lo_y = 118,
    .bolt_stroke = s_bolt, .bolt_count = ARRAY_LENGTH(s_bolt),
};
#endif

// --- Ridges ---
// sharp apexes with a steep face one side and a longer run-out the other, which is what makes
// a line read as a mountain rather than a hill. each array ends with two off-screen corners
// that close the polygon for the fill, and the crest count is how many points the outline walks
//
// the far range peaks in the middle and the two flanks are held down, because the arc crosses
// x=40 and x=170 while the sun is still well clear of its horizon. a flank as tall as the
// centre swallows the disc a couple of hours either side of the real sunrise and sunset
// the round arrays are the same composition spread to the wider screen: the peak spacing keeps
// its rhythm, and every crest drops 12px so the range clears the deeper status strip and still
// leaves the clock solid ground to sit on
#define FAR_CREST 14
#if defined(PBL_ROUND)
static const GPoint s_far_pts[] = {
    {-4, 122}, {18, 104}, {34, 116}, {52, 102}, {75, 114}, {94, 100}, {114, 120},
    {135, 88}, {158, 112}, {177, 98}, {200, 118}, {221, 102}, {244, 110}, {264, 104},
    {264, 264}, {-4, 264},
};
#else
static const GPoint s_far_pts[] = {
    {-4, 104}, {14, 86}, {26, 98}, {40, 84}, {58, 96}, {72, 82}, {88, 102},
    {104, 70}, {122, 94}, {136, 80}, {154, 100}, {170, 84}, {188, 92}, {204, 86},
    {204, 232}, {-4, 232},
};
#endif

// the near range is bigger and sparser, built around one dominant central ridgeline
#define NEAR_CREST 11
#if defined(PBL_ROUND)
static const GPoint s_near_pts[] = {
    {-4, 140}, {23, 126}, {44, 138}, {70, 114}, {99, 132}, {130, 110},
    {161, 130}, {187, 120}, {211, 136}, {237, 124}, {264, 134},
    {264, 264}, {-4, 264},
};
#else
static const GPoint s_near_pts[] = {
    {-4, 126}, {18, 112}, {34, 124}, {54, 100}, {76, 118}, {100, 96},
    {124, 116}, {144, 106}, {162, 122}, {182, 110}, {204, 120},
    {204, 232}, {-4, 232},
};
#endif

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
 * @param pts The ridge's points: the crest left to right, then the two corners that close it.
 * @param count How many points there are in all.
 * @param crest How many of the points make up the crest.
 * @param body The body fill colour.
 * @param line The outline colour.
 * @param snow True to cap the summits in snow.
 * @param snow_span How far either side of an apex the snow reaches.
 */
static void draw_ridge(GContext *ctx, const GPoint *pts, int count, int crest,
    GColor body, GColor line, bool snow, int snow_span)
{
    shape_fill(ctx, pts, count, body);

    graphics_context_set_stroke_color(ctx, line);
    draw_facets(ctx, pts, crest);

    if (snow)
    {
        draw_snowcaps(ctx, pts, crest, snow_span);
    }

    // the crest goes on last so it sits cleanly over both the creases and the snow
    graphics_context_set_stroke_color(ctx, line);
    graphics_context_set_stroke_width(ctx, 3);
    line_polyline(ctx, pts, crest);
    graphics_context_set_stroke_width(ctx, 1);
}

#if defined(PBL_ROUND)
// the trail the sign stands beside, wandering up the lower slope from the left. it fills the
// band the round screen leaves between the ridges and the clock, and it gives the sign a reason
// to be there
// it runs past the foot of the post rather than into it, so the sign stands beside the path the
// way a sign does, and the path carries on out of frame
static const GPoint s_trail[] = {
    {66, 152}, {98, 146}, {126, 150}, {152, 145}, {176, 150}, {200, 145}, {218, 148},
};

// grass on the flat ground in front of the range. a fixed scatter rather than a random one, so
// the field is the same every time you look at it
//
// nothing sits below the clock's cap line across the middle, or the tufts come up between the
// digits. out at the sides the digits never reach, so the field carries on down the edges there
static const GPoint s_tufts[] = {
    {14, 152}, {30, 148}, {46, 153}, {58, 146}, {72, 151}, {88, 154}, {104, 147},
    {118, 152}, {132, 150}, {148, 153}, {162, 146}, {176, 152}, {190, 149},
    {204, 153}, {218, 147}, {232, 152}, {246, 149},
    {18, 166}, {38, 172}, {14, 179},
    {224, 164}, {244, 170}, {218, 177},
};

/**
 * @brief Draw the trail up the lower slope.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_trail(GContext *ctx, const RidgelinePalette *pal)
{
    // a path is bare ground, so it clears a margin through the speckle and the dashes are laid in
    // that. drawn straight onto the meadow a hairline disappears into it, and anything bolder
    // stops being a path through a field and becomes a road
    graphics_context_set_stroke_color(ctx, pal->land);
    graphics_context_set_stroke_width(ctx, 5);
    line_polyline(ctx, s_trail, ARRAY_LENGTH(s_trail));

    // the dashes are on the ground, not against the sky, so they take a colour the palette
    // guarantees there rather than the ink the rest of the linework uses. 3 on and 6 off, sparse
    // enough that it reads as a path rather than another crease
    graphics_context_set_stroke_color(ctx, pal->dim);
    graphics_context_set_stroke_width(ctx, 1);
    line_dashed(ctx, s_trail, ARRAY_LENGTH(s_trail), 3, 9);
}

/**
 * @brief Plant a trail sign in the saddle and letter the temperature on its board.
 *
 * Drawn after the weather rather than with the ridges, because the reading is the one thing on
 * the face that has to stay readable in every condition.
 *
 * The board is filled with the sky and lettered in ink, because ink over sky is the one pairing
 * the palette guarantees: every line in this scene is drawn that way. Filling with the land
 * instead leaves the reading nearly invisible wherever a theme pairs dark ground with dark
 * linework, which several of them do by day. The post is ground rather than sky, so it takes the
 * clock's colour, which is the pairing guaranteed over the ground.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_trail_sign(GContext *ctx, const RidgelinePalette *pal)
{
    // no location and no reading means no sign at all, post included, rather than a board
    // lettered "--". a trail with nothing to say carries no sign
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;
    }

    GRect board = GRect(SIGN_X - (width + SIGN_PAD * 2) / 2, SIGN_BOARD_Y, width + SIGN_PAD * 2, SIGN_BOARD_H);

    // the post first, so the board's own outline closes over the top of it
    graphics_context_set_stroke_color(ctx, pal->text);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(SIGN_X, board.origin.y + board.size.h), GPoint(SIGN_X, SIGN_BASE_Y));
    graphics_context_set_stroke_width(ctx, 1);

    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_fill_rect(ctx, board, 0, GCornerNone);
    graphics_draw_rect(ctx, board);
    letter_board(ctx, temp, board, pal->ink);
}
#endif

void ridgeline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const RidgelinePalette *pal = frame->palette;

    // snow lies where snow falls, so it caps the peaks only when it is snowing
    bool snow = sketchbook_cond_snows(frame->cond);

    draw_ridge(ctx, s_far_pts, ARRAY_LENGTH(s_far_pts), FAR_CREST, pal->land_far, pal->ink, snow, 12);
    draw_ridge(ctx, s_near_pts, ARRAY_LENGTH(s_near_pts), NEAR_CREST, pal->land, pal->ink, snow, 20);

#if defined(PBL_ROUND)
    // grass on the ground the trail crosses, in the colour the palette guarantees down there
    marks_tufts(ctx, s_tufts, ARRAY_LENGTH(s_tufts), pal->dim);
    draw_trail(ctx, pal);
#endif
}

#if defined(PBL_ROUND)
void ridgeline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    // the sign goes in front of the weather. the reading is the one thing on the face that has to
    // stay readable in every condition, and painted with the ridges it sits behind the rain
    draw_trail_sign(ctx, frame->palette);
}
#endif

/** @} */
