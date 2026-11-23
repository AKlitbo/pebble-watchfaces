/**
 * @file uluru.c
 * @brief The Uluru scene: the long rounded rock seen from afar across flat desert, spinifex on the
 * sand, a desert oak to the right, and a water tank on its stand to the left, which on a round
 * screen carries the temperature.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/landmarks.h"

#include "scene/reading.h"
#include "draw/ground.h"
#include "draw/line.h"
#include "draw/marks.h"
#include "draw/letter.h"
#include "draw/shape.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- The storm's bolts ---
// one at either end of the rock and a small one over its middle, clear of its top. each table is
// placed for its own screen, since the round band's base does not follow LM_Y
#if defined(PBL_ROUND)
static const BoltSpot s_bolt_spots[] = {{204, 50, -1}, {80, 40, 1}, {136, 26, -1}};
#else
static const BoltSpot s_bolt_spots[] = {{44, 42, 1}, {154, 34, -1}, {104, 22, 1}};
#endif
SCENE_BOLTS(uluru_bolts, s_bolt_spots);

// the rock, low and long, rising steeply at both ends to a gently rolling top
static const GPoint s_rock[] = {
    {LM_X(28), LM_Y(108)}, {LM_X(27), LM_Y(99)}, {LM_X(30), LM_Y(93)}, {LM_X(36), LM_Y(89)},
    {LM_X(46), LM_Y(85)}, {LM_X(58), LM_Y(82)}, {LM_X(70), LM_Y(81)}, {LM_X(90), LM_Y(79)},
    {LM_X(110), LM_Y(80)}, {LM_X(130), LM_Y(79)}, {LM_X(150), LM_Y(81)}, {LM_X(160), LM_Y(83)},
    {LM_X(168), LM_Y(87)}, {LM_X(172), LM_Y(93)}, {LM_X(175), LM_Y(100)}, {LM_X(176), LM_Y(108)},
};

// the gullies the rain has cut down its face, each a top, a middle, and a foot
static const GPoint s_gullies[][3] = {
    {{LM_X(40), LM_Y(87)}, {LM_X(41), LM_Y(98)}, {LM_X(40), LM_Y(108)}},
    {{LM_X(60), LM_Y(82)}, {LM_X(59), LM_Y(96)}, {LM_X(60), LM_Y(108)}},
    {{LM_X(84), LM_Y(80)}, {LM_X(85), LM_Y(94)}, {LM_X(84), LM_Y(108)}},
    {{LM_X(108), LM_Y(80)}, {LM_X(107), LM_Y(94)}, {LM_X(108), LM_Y(108)}},
    {{LM_X(132), LM_Y(79)}, {LM_X(133), LM_Y(94)}, {LM_X(132), LM_Y(108)}},
    {{LM_X(156), LM_Y(82)}, {LM_X(155), LM_Y(96)}, {LM_X(156), LM_Y(108)}},
};

// clumps of spinifex on the sand, each drawn as a low mound. nothing sits below the clock's cap
// line across the middle
static const GPoint s_spinifex[] = {
    {LM_X(6), LM_Y(118)}, {LM_X(40), LM_Y(115)}, {LM_X(76), LM_Y(120)},
    {LM_X(120), LM_Y(116)}, {LM_X(150), LM_Y(121)}, {LM_X(170), LM_Y(114)},
};

// the desert oak: a trunk, and its wispy branches hanging off the top, each a bend and a tip
#define OAK_FOOT GPoint(LM_X(190), LM_Y(116))
#define OAK_TOP GPoint(LM_X(189), LM_Y(96))
static const int8_t s_oak[][4] = {
    {-6, -1, -10, 5}, {6, -1, 10, 5}, {-4, -5, -8, -4}, {4, -5, 8, -4}, {-5, 3, -7, 8}, {5, 3, 7, 8},
};

// --- The water tank ---
// a corrugated tank on a stand, the way a station keeps its water out here. on a round screen the
// reading is painted on a panel on its side, and the tank is sized to it. the rectangle shows the
// reading in its stats row, so there the tank stands as scenery at a fixed size
#if defined(PBL_ROUND)
#define TANK_X 38          ///< The tank's centre line
#define TANK_ROOF 123      ///< The point of its roof
#define TANK_TOP 130       ///< Where the roof meets the side
#define TANK_H 22          ///< How tall the side is
#define TANK_LEGS 160      ///< Where the stand's legs reach the ground
#define PANEL_H 16         ///< How tall the painted panel is, room for the hand font's low-hanging digits
#define PANEL_PAD 6        ///< Air either side of the reading, inside the panel
#else
#define TANK_X 16          ///< The tank's centre line
#define TANK_ROOF 92       ///< The point of its roof
#define TANK_TOP 97        ///< Where the roof meets the side
#define TANK_H 12          ///< How tall the side is
#define TANK_LEGS 117      ///< Where the stand's legs reach the ground, deep enough for the brace to cross
#define TANK_W 20          ///< How wide the tank is
#endif

/**
 * @brief Stand the water tank on its stand to the left of the rock.
 *
 * On a round screen it carries the temperature on a painted panel, filled with the sky and
 * lettered in ink, the one pairing every palette guarantees. It is drawn after the weather there,
 * because the reading is the one thing on the face that has to stay readable in every condition.
 * With no reading there is no tank, rather than a panel reading "--".
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_tank(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground)
{
#if defined(PBL_ROUND)
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;
    }

    int panel_w = width + PANEL_PAD * 2;
    int half = panel_w / 2 + 3;
#else
    int half = TANK_W / 2;
#endif
    GRect body = GRect(TANK_X - half, TANK_TOP, half * 2, TANK_H);

    // the stand's legs and their cross brace first, so the tank sits on top of them
    // they stand on the sand, so they take the dim the spinifex uses
    // a palette can make the sand the same colour as the ink
    graphics_context_set_stroke_color(ctx, pal->dim);
    graphics_context_set_stroke_width(ctx, 1);
    int left = body.origin.x + 4;
    int right = body.origin.x + body.size.w - 5;
    int foot = TANK_TOP + TANK_H;
    graphics_draw_line(ctx, GPoint(left, foot), GPoint(left, TANK_LEGS));
    graphics_draw_line(ctx, GPoint(right, foot), GPoint(right, TANK_LEGS));
    graphics_draw_line(ctx, GPoint(left, foot + 3), GPoint(right, TANK_LEGS));
    graphics_draw_line(ctx, GPoint(right, foot + 3), GPoint(left, TANK_LEGS));

    // the tank, its shallow roof, and the corrugations top and bottom
    GPoint roof[] = {{body.origin.x - 2, TANK_TOP}, {TANK_X, TANK_ROOF}, {body.origin.x + body.size.w + 1, TANK_TOP}};
    shape_fill_outlined(ctx, roof, ARRAY_LENGTH(roof), ground->land_far, pal->ink);

    graphics_context_set_fill_color(ctx, ground->land_far);
    graphics_fill_rect(ctx, body, 0, GCornerNone);
    graphics_draw_rect(ctx, body);
    graphics_draw_line(ctx, GPoint(body.origin.x + 1, TANK_TOP + 2), GPoint(body.origin.x + body.size.w - 2, TANK_TOP + 2));
    graphics_draw_line(ctx, GPoint(body.origin.x + 1, foot - 3), GPoint(body.origin.x + body.size.w - 2, foot - 3));

#if defined(PBL_ROUND)
    GRect panel = GRect(TANK_X - panel_w / 2, TANK_TOP + (TANK_H - PANEL_H) / 2, panel_w, PANEL_H);
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, panel, 0, GCornerNone);
    graphics_draw_rect(ctx, panel);
    letter_board(ctx, temp, panel, pal->ink);
#else
    // a corrugation through the middle, where the round screen has its panel
    graphics_draw_line(ctx, GPoint(body.origin.x + 1, TANK_TOP + TANK_H / 2), GPoint(body.origin.x + body.size.w - 2, TANK_TOP + TANK_H / 2));
#endif
}

void uluru_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const RidgelineGround *ground = frame->ground;
    int horizon = LM_Y(LM_HORIZON);

    shape_fill_outlined(ctx, s_rock, ARRAY_LENGTH(s_rock), ground->land_far, pal->ink);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_gullies); i++)
    {
        line_polyline(ctx, s_gullies[i], ARRAY_LENGTH(s_gullies[i]));
    }

    // the sand runs from the foot of the rock down under the clock
    ground_fill(ctx, bounds, horizon, ground->land, pal->ink);

    // spinifex is a mark on the ground, so it takes the palette's dim, the colour the date and
    // readouts use down there
    graphics_context_set_stroke_color(ctx, pal->dim);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_spinifex); i++)
    {
        GPoint at = s_spinifex[i];
        graphics_draw_line(ctx, GPoint(at.x - 5, at.y), GPoint(at.x - 2, at.y - 3));
        graphics_draw_line(ctx, GPoint(at.x - 2, at.y - 3), GPoint(at.x + 2, at.y - 3));
        graphics_draw_line(ctx, GPoint(at.x + 2, at.y - 3), GPoint(at.x + 5, at.y));
    }

    marks_tree(ctx, OAK_FOOT, OAK_TOP, s_oak, ARRAY_LENGTH(s_oak), pal->ink);

    // the rectangle's tank is scenery, so it stands behind the weather with the rest of the scene
#if !defined(PBL_ROUND)
    draw_tank(ctx, pal, ground);
#endif
}

#if defined(PBL_ROUND)
void uluru_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_tank(ctx, frame->palette, frame->ground);
}
#endif

/** @} */
