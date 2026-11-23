/**
 * @file monumentvalley.c
 * @brief The Monument Valley scene: the two Mittens and Merrick Butte standing on their slopes of
 * scree over the desert, the highway running in towards them, and on a round screen a roadside sign
 * carrying the temperature.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/landmarks.h"

#include "draw/ground.h"
#include "draw/line.h"
#include "draw/shape.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief One butte: its outline, sweeping up the scree to the column and over its top, and its cracks. */
typedef struct
{
    GPoint outline[10];  ///< up the left slope, up the column, across the top, and down the other side
    int16_t crack_x[2];  ///< where the two cracks run down the column
    int16_t crack_top;   ///< where the cracks start, running down to the column's foot
} Butte;

// the West Mitten, the East Mitten, and Merrick Butte, the last half off the right edge
static const Butte s_buttes[] = {
    {
        .outline = {{LM_X(16), LM_Y(108)}, {LM_X(24), LM_Y(103)}, {LM_X(30), LM_Y(97)}, {LM_X(35), LM_Y(90)},
            {LM_X(39), LM_Y(46)}, {LM_X(62), LM_Y(48)}, {LM_X(64), LM_Y(90)}, {LM_X(69), LM_Y(97)},
            {LM_X(75), LM_Y(103)}, {LM_X(84), LM_Y(108)}},
        .crack_x = {LM_X(56), LM_X(59)}, .crack_top = LM_Y(50),
    },
    {
        .outline = {{LM_X(108), LM_Y(108)}, {LM_X(116), LM_Y(103)}, {LM_X(122), LM_Y(97)}, {LM_X(127), LM_Y(90)},
            {LM_X(130), LM_Y(50)}, {LM_X(153), LM_Y(48)}, {LM_X(156), LM_Y(90)}, {LM_X(161), LM_Y(97)},
            {LM_X(167), LM_Y(103)}, {LM_X(174), LM_Y(108)}},
        .crack_x = {LM_X(147), LM_X(150)}, .crack_top = LM_Y(52),
    },
    {
        .outline = {{LM_X(168), LM_Y(108)}, {LM_X(176), LM_Y(104)}, {LM_X(181), LM_Y(100)}, {LM_X(185), LM_Y(96)},
            {LM_X(186), LM_Y(78)}, {LM_X(200), LM_Y(78)}, {LM_X(202), LM_Y(96)}, {LM_X(206), LM_Y(100)},
            {LM_X(211), LM_Y(104)}, {LM_X(218), LM_Y(108)}},
        .crack_x = {LM_X(196), LM_X(199)}, .crack_top = LM_Y(80),
    },
};

// the thumbs that give the Mittens their name, a thin spire beside each column. their feet run
// down into the scree, which is drawn over them
static const GPoint s_thumbs[][4] = {
    {{LM_X(66), LM_Y(104)}, {LM_X(73), LM_Y(104)}, {LM_X(72), LM_Y(60)}, {LM_X(68), LM_Y(58)}},
    {{LM_X(118), LM_Y(104)}, {LM_X(125), LM_Y(104)}, {LM_X(124), LM_Y(62)}, {LM_X(120), LM_Y(60)}},
};

// streaks of fallen rock down the Mittens' slopes, each a top and a foot
static const GPoint s_scree[][2] = {
    {{LM_X(40), LM_Y(94)}, {LM_X(36), LM_Y(102)}},
    {{LM_X(60), LM_Y(94)}, {LM_X(64), LM_Y(102)}},
    {{LM_X(130), LM_Y(94)}, {LM_X(126), LM_Y(102)}},
    {{LM_X(152), LM_Y(94)}, {LM_X(156), LM_Y(102)}},
};

// the highway's two edges, running in from the near desert to a point on the horizon
static const GPoint s_road[][2] = {
    {{LM_X(98), LM_Y(108)}, {LM_X(72), LM_Y(120)}},
    {{LM_X(102), LM_Y(108)}, {LM_X(128), LM_Y(120)}},
};

// the dashes down the middle of the highway, each a top and a foot
static const GPoint s_dashes[][2] = {
    {{LM_X(100), LM_Y(110)}, {LM_X(100), LM_Y(112)}},
    {{LM_X(100), LM_Y(115)}, {LM_X(100), LM_Y(117)}},
};

// scrub dotted across the desert floor, clear of the round screen's sign. nothing sits below the
// clock's cap line across the middle
static const GPoint s_scrub[] = {
    {LM_X(8), LM_Y(117)}, {LM_X(20), LM_Y(114)}, {LM_X(44), LM_Y(112)}, {LM_X(184), LM_Y(112)}, {LM_X(180), LM_Y(118)},
};

#if defined(PBL_ROUND)
// --- The roadside sign ---
// a highway sign on two posts beside the road, with the reading on its board
#define SIGN_X 196         ///< The board's centre line, clear of the road's end for a wide reading
#define SIGN_PAD 8         ///< Air either side of the reading, inside the board
#endif

/**
 * @brief Stand one butte on its scree, with the cracks down its column.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param butte The butte to draw.
 */
static void draw_butte(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground,
    const Butte *butte)
{
    shape_fill_outlined(ctx, butte->outline, ARRAY_LENGTH(butte->outline), ground->land_far, pal->ink);

    // the column's foot is the fourth point of the outline, where the left slope meets it
    int foot = butte->outline[3].y;
    for (unsigned i = 0; i < ARRAY_LENGTH(butte->crack_x); i++)
    {
        graphics_draw_line(ctx, GPoint(butte->crack_x[i], butte->crack_top), GPoint(butte->crack_x[i], foot));
    }
}

void monument_valley_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const RidgelineGround *ground = frame->ground;
    int horizon = LM_Y(LM_HORIZON);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_thumbs); i++)
    {
        shape_fill_outlined(ctx, s_thumbs[i], ARRAY_LENGTH(s_thumbs[i]), ground->land_far, pal->ink);
    }
    for (unsigned i = 0; i < ARRAY_LENGTH(s_buttes); i++)
    {
        draw_butte(ctx, pal, ground, &s_buttes[i]);
    }
    line_segments(ctx, s_scree, ARRAY_LENGTH(s_scree));

    // the desert runs from the foot of the buttes down under the clock
    ground_fill(ctx, bounds, horizon, ground->land, pal->ink);

    // the road and the scrub are marks on the ground, so they take the palette's dim
    // a palette can make the desert the same colour as the ink
    graphics_context_set_stroke_color(ctx, pal->dim);
    line_segments(ctx, s_road, ARRAY_LENGTH(s_road));
    line_segments(ctx, s_dashes, ARRAY_LENGTH(s_dashes));
    graphics_context_set_fill_color(ctx, pal->dim);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_scrub); i++)
    {
        graphics_fill_circle(ctx, s_scrub[i], 1);
    }
}

#if defined(PBL_ROUND)
void monument_valley_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;

    // the board is filled with the sky and lettered in ink, the one pairing every palette
    // guarantees. drawn after the weather, because the reading is the one thing on the face that
    // has to stay readable in every condition. the posts stand in the desert, so they take the dim
    // the road and the scrub do
    landmarks_draw_sign(ctx, SIGN_X, SIGN_PAD, 2, pal->dim, pal->sky_hi, pal->ink);
}
#endif

/** @} */
