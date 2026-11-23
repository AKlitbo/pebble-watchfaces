/**
 * @file halfdome.c
 * @brief The Half Dome scene: the granite dome sheared off on one side between the valley walls,
 * pines along the meadow and the river through it, climbers' headlamps on the face after dark,
 * and on a round screen a park sign carrying the temperature.
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

// the dome: a long rounded back rising from the left, and the sheer face dropping straight down
// on the right, with a short slope of rubble at its foot
static const GPoint s_dome[] = {
    {LM_X(40), LM_Y(108)}, {LM_X(46), LM_Y(100)}, {LM_X(52), LM_Y(92)}, {LM_X(59), LM_Y(85)},
    {LM_X(66), LM_Y(78)}, {LM_X(73), LM_Y(71)}, {LM_X(80), LM_Y(65)}, {LM_X(89), LM_Y(59)},
    {LM_X(98), LM_Y(55)}, {LM_X(108), LM_Y(52)}, {LM_X(118), LM_Y(51)}, {LM_X(128), LM_Y(54)},
    {LM_X(134), LM_Y(57)}, {LM_X(136), LM_Y(60)}, {LM_X(136), LM_Y(95)}, {LM_X(150), LM_Y(108)},
};

// the curved sheets the granite peels off in, running up the dome's back towards the face
static const GPoint s_sheets[][5] = {
    {{LM_X(60), LM_Y(89)}, {LM_X(75), LM_Y(78)}, {LM_X(92), LM_Y(70)}, {LM_X(108), LM_Y(64)}, {LM_X(126), LM_Y(60)}},
    {{LM_X(72), LM_Y(97)}, {LM_X(86), LM_Y(88)}, {LM_X(100), LM_Y(81)}, {LM_X(116), LM_Y(75)}, {LM_X(132), LM_Y(71)}},
    {{LM_X(88), LM_Y(105)}, {LM_X(98), LM_Y(98)}, {LM_X(110), LM_Y(92)}, {LM_X(121), LM_Y(87)}, {LM_X(134), LM_Y(84)}},
};

// the valley walls either side, lower and further off than the dome
static const GPoint s_wall_left[] = {
    {LM_X(-10), LM_Y(108)}, {LM_X(-10), LM_Y(86)}, {LM_X(10), LM_Y(81)}, {LM_X(30), LM_Y(86)},
    {LM_X(44), LM_Y(92)}, {LM_X(56), LM_Y(97)}, {LM_X(70), LM_Y(108)},
};
static const GPoint s_wall_right[] = {
    {LM_X(150), LM_Y(108)}, {LM_X(156), LM_Y(89)}, {LM_X(170), LM_Y(82)}, {LM_X(186), LM_Y(86)},
    {LM_X(200), LM_Y(81)}, {LM_X(210), LM_Y(84)}, {LM_X(210), LM_Y(108)},
};

// the pines along the meadow's far edge, each by its tip
static const GPoint s_pines[] = {
    {LM_X(4), LM_Y(96)}, {LM_X(12), LM_Y(99)}, {LM_X(21), LM_Y(94)}, {LM_X(30), LM_Y(101)},
    {LM_X(162), LM_Y(99)}, {LM_X(171), LM_Y(93)}, {LM_X(180), LM_Y(99)}, {LM_X(189), LM_Y(94)},
    {LM_X(199), LM_Y(98)},
};

// the river winding through the meadow, both banks
static const GPoint s_river[][5] = {
    {{LM_X(68), LM_Y(118)}, {LM_X(78), LM_Y(113)}, {LM_X(90), LM_Y(114)}, {LM_X(100), LM_Y(115)}, {LM_X(112), LM_Y(108)}},
    {{LM_X(76), LM_Y(118)}, {LM_X(84), LM_Y(115)}, {LM_X(94), LM_Y(116)}, {LM_X(106), LM_Y(116)}, {LM_X(116), LM_Y(108)}},
};

// two climbers bivouacked on the face, whose headlamps show once it is dark. they sit just inside
// the face's outline, so they read as lights on the rock rather than gaps in the line
static const GPoint s_headlamps[] = {{LM_X(133), LM_Y(72)}, {LM_X(133), LM_Y(84)}};

#if defined(PBL_ROUND)
// --- The park sign ---
// a routed wooden sign on two posts at the meadow's edge, the way a park marks its trails, with
// the reading on its board
#define SIGN_X 50          ///< The board's centre line, left of where the river comes in
#define SIGN_PAD 10        ///< Air either side of the reading, inside the board
#endif

/**
 * @brief Draw one pine: a narrow spike of a tree standing on the meadow.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param tip The tree's top.
 */
static void draw_pine(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground, GPoint tip)
{
    int base = LM_Y(109);
    int half = (base - tip.y) / 3 + 1;
    GPoint tree[] = {{tip.x - half, base}, tip, {tip.x + half, base}};

    shape_fill_outlined(ctx, tree, ARRAY_LENGTH(tree), ground->land, pal->ink);
}

void half_dome_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const RidgelineGround *ground = frame->ground;
    int horizon = LM_Y(LM_HORIZON);

    // the walls are the far side of the valley, so they take the sky's low colour and read as
    // distance, outlined in ink like everything else against the sky
    shape_fill_outlined(ctx, s_wall_left, ARRAY_LENGTH(s_wall_left), pal->sky_lo, pal->ink);
    shape_fill_outlined(ctx, s_wall_right, ARRAY_LENGTH(s_wall_right), pal->sky_lo, pal->ink);

    shape_fill_outlined(ctx, s_dome, ARRAY_LENGTH(s_dome), ground->land_far, pal->ink);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_sheets); i++)
    {
        line_polyline(ctx, s_sheets[i], ARRAY_LENGTH(s_sheets[i]));
    }

    if (frame->night)
    {
        graphics_context_set_fill_color(ctx, pal->disc);
        for (unsigned i = 0; i < ARRAY_LENGTH(s_headlamps); i++)
        {
            graphics_fill_circle(ctx, s_headlamps[i], 1);
        }
    }

    // the meadow runs from the foot of the valley down under the clock
    ground_fill(ctx, bounds, horizon, ground->land, pal->ink);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_pines); i++)
    {
        draw_pine(ctx, pal, ground, s_pines[i]);
    }

    // the river is a mark on the ground, so it takes the palette's dim, the colour the date and
    // readouts use down there
    graphics_context_set_stroke_color(ctx, pal->dim);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_river); i++)
    {
        line_polyline(ctx, s_river[i], ARRAY_LENGTH(s_river[i]));
    }
}

#if defined(PBL_ROUND)
void half_dome_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;

    // a routed park sign: dark ink lettered in the sky's colour, the guaranteed pairing turned the
    // other way round. drawn after the weather, because the reading is the one thing on the face
    // that has to stay readable in every condition. the posts stand in the meadow, so they take
    // the dim every other mark on the ground does
    landmarks_draw_sign(ctx, SIGN_X, SIGN_PAD, 1, pal->dim, pal->ink, pal->sky_hi);
}
#endif

/** @} */
