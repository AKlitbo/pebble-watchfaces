/**
 * @file giza.c
 * @brief The Giza scene: the three pyramids over the desert, a pair of palms on the left, and on a
 * round screen a stele carrying the temperature in a cartouche.
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
// one up each side of the big pyramid and one past the second, all in the open sky between the
// peaks. each table is placed for its own screen, since the round band's base does not follow LM_Y
#if defined(PBL_ROUND)
static const BoltSpot s_bolt_spots[] = {{156, 36, 1}, {80, 30, 1}, {210, 24, -1}};
#else
static const BoltSpot s_bolt_spots[] = {{52, 40, 1}, {98, 26, -1}, {160, 20, -1}};
#endif
SCENE_BOLTS(giza_bolts, s_bolt_spots);

/**
 * @brief One pyramid seen from a corner: its two base corners, its apex, and where the ridge
 * between its two faces meets the ground.
 */
typedef struct
{
    GPoint left;   ///< The near-left base corner
    GPoint apex;   ///< The top
    GPoint foot;   ///< The foot of the ridge between the two faces
    GPoint right;  ///< The far-right base corner
} Pyramid;

// back to front: Menkaure small and far off on the right, Khafre behind and the tallest of the
// three, and Khufu in front. seen from a corner, so each shows a broad face and a narrow one
static const Pyramid s_pyramids[] = {
    {{LM_X(170), LM_Y(108)}, {LM_X(188), LM_Y(87)}, {LM_X(192), LM_Y(108)}, {LM_X(210), LM_Y(108)}},
    {{LM_X(88), LM_Y(108)}, {LM_X(140), LM_Y(54)}, {LM_X(150), LM_Y(108)}, {LM_X(192), LM_Y(108)}},
    {{LM_X(20), LM_Y(108)}, {LM_X(76), LM_Y(58)}, {LM_X(88), LM_Y(108)}, {LM_X(132), LM_Y(108)}},
};

// two long dunes across the sand in front, low enough to stay clear of the clock
static const GPoint s_dune_near[] = {
    {LM_X(-10), LM_Y(124)}, {LM_X(20), LM_Y(121)}, {LM_X(50), LM_Y(124)}, {LM_X(80), LM_Y(121)},
    {LM_X(110), LM_Y(124)}, {LM_X(140), LM_Y(121)}, {LM_X(170), LM_Y(124)}, {LM_X(210), LM_Y(121)},
};
static const GPoint s_dune_far[] = {
    {LM_X(-10), LM_Y(114)}, {LM_X(30), LM_Y(111)}, {LM_X(70), LM_Y(114)}, {LM_X(110), LM_Y(111)},
    {LM_X(150), LM_Y(114)}, {LM_X(190), LM_Y(111)}, {LM_X(210), LM_Y(112)},
};

#if defined(PBL_ROUND)
// --- The stele ---
// a flat slab standing in the sand in front of the pyramids, under a cornice, with the sun disc
// carved at the top and the reading cut into a cartouche below it. its foot stays clear of the
// AM/PM marker over the clock
#define STELE_X 130        ///< The slab's centre line
#define CORNICE_TOP 128    ///< Top of the cornice that caps the slab
#define STELE_TOP 131      ///< Top of the slab itself, under the cornice
#define STELE_BOTTOM 162   ///< Where it goes into the sand
#define CARTOUCHE_TOP 139  ///< Top of the cartouche
#define CARTOUCHE_H 18     ///< How tall the cartouche is, room for the hand font's low-hanging digits
#define CARTOUCHE_PAD 8    ///< Air either side of the reading, clear of the rounded ends
#endif

/**
 * @brief Where a line between two points crosses a row.
 *
 * @param from One end of the line.
 * @param to The other end.
 * @param y The row.
 * @return The column it crosses at.
 */
static int x_at(GPoint from, GPoint to, int y)
{
    int span = to.y - from.y;
    return from.x + (span ? ((to.x - from.x) * (y - from.y)) / span : 0);
}

/**
 * @brief Draw one pyramid: both faces in stone, the shaded one ruled across in courses.
 *
 * The broad face is left plain and the narrow one is ruled, so the light comes from one side
 * whatever the palette. Shading it with a second colour would rest on that colour holding up
 * against the sky in every theme, and ruling it in ink is how the rest of the face shades.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param p The pyramid.
 */
static void draw_pyramid(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground,
    const Pyramid *p)
{
    GPoint body[] = {p->left, p->apex, p->right};
    shape_fill_outlined(ctx, body, ARRAY_LENGTH(body), ground->land_far, pal->ink);

    // the courses on the shaded face, one every few rows from just under the apex to the sand
    for (int y = p->apex.y + 4; y < p->foot.y; y += 4)
    {
        graphics_draw_line(ctx, GPoint(x_at(p->apex, p->foot, y), y), GPoint(x_at(p->apex, p->right, y), y));
    }

    graphics_draw_line(ctx, p->apex, p->foot);
}

/**
 * @brief Draw a palm leaning out of the sand: a bent trunk and four fronds hanging off its top.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param foot Where the trunk meets the ground.
 * @param height How tall the trunk stands.
 * @param lean Which way it leans, 1 to the right or -1 to the left.
 */
static void draw_palm(GContext *ctx, const CommonPalette *pal, GPoint foot, int height, int lean)
{
    // each frond goes out and droops, so it is two short legs rather than one straight one
    static const int8_t fronds[][4] = {{-8, 1, -13, 6}, {8, 0, 13, 5}, {-4, -6, -9, -7}, {4, -6, 9, -7}};
    GPoint top = GPoint(foot.x + lean * LM_W(3), foot.y - height);

    marks_tree(ctx, foot, top, fronds, ARRAY_LENGTH(fronds), pal->ink);
}

#if defined(PBL_ROUND)
/**
 * @brief Stand a stele in the sand with the temperature cut into a cartouche on its face.
 *
 * Drawn after the weather, because the reading is the one thing on the face that has to stay
 * readable in every condition. The cartouche is filled with the sky and lettered in ink, the one
 * pairing every palette guarantees.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_stele(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground)
{
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;  // no reading, no stele, rather than a cartouche reading "--"
    }

    GRect cartouche = GRect(STELE_X - (width + CARTOUCHE_PAD * 2) / 2, CARTOUCHE_TOP,
        width + CARTOUCHE_PAD * 2, CARTOUCHE_H);
    int half = cartouche.size.w / 2 + 5;

    // the slab, then the cornice across its top, a little wider than the slab the way an Egyptian
    // cornice overhangs the wall under it
    GRect slab = GRect(STELE_X - half, STELE_TOP, half * 2, STELE_BOTTOM - STELE_TOP);
    GRect cornice = GRect(STELE_X - half - 2, CORNICE_TOP, half * 2 + 4, STELE_TOP - CORNICE_TOP + 1);
    graphics_context_set_fill_color(ctx, ground->land_far);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_fill_rect(ctx, slab, 0, GCornerNone);
    graphics_draw_rect(ctx, slab);
    graphics_fill_rect(ctx, cornice, 0, GCornerNone);
    graphics_draw_rect(ctx, cornice);

    // the winged sun carved between the cornice and the cartouche
    int sun_y = STELE_TOP + 4;
    graphics_context_set_fill_color(ctx, pal->ink);
    graphics_fill_circle(ctx, GPoint(STELE_X, sun_y), 2);
    graphics_draw_line(ctx, GPoint(STELE_X - 9, sun_y), GPoint(STELE_X - 4, sun_y));
    graphics_draw_line(ctx, GPoint(STELE_X + 4, sun_y), GPoint(STELE_X + 9, sun_y));

    // the cartouche, a long oval with the bar across its foot that makes it one
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, cartouche, 5, GCornersAll);
    graphics_draw_round_rect(ctx, cartouche, 5);
    graphics_draw_line(ctx, GPoint(cartouche.origin.x + 2, cartouche.origin.y + CARTOUCHE_H + 1),
        GPoint(cartouche.origin.x + cartouche.size.w - 3, cartouche.origin.y + CARTOUCHE_H + 1));

    letter_board(ctx, temp, cartouche, pal->ink);
}
#endif

void giza_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const RidgelineGround *ground = frame->ground;
    int horizon = LM_Y(LM_HORIZON);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_pyramids); i++)
    {
        draw_pyramid(ctx, pal, ground, &s_pyramids[i]);
    }

    // the sand runs from the foot of the pyramids down under the clock
    ground_fill(ctx, bounds, horizon, ground->land, pal->ink);

    // the dunes are ground marks rather than linework against the sky, so they take the palette's
    // dim, the colour the date and readouts use down there. they are drawn two wide, since a
    // hairline gets lost in the sand
    graphics_context_set_stroke_color(ctx, pal->dim);
    graphics_context_set_stroke_width(ctx, 2);
    line_polyline(ctx, s_dune_far, ARRAY_LENGTH(s_dune_far));
    line_polyline(ctx, s_dune_near, ARRAY_LENGTH(s_dune_near));

    // a pair of palms on the left, one taller than the other and leaning apart, so Menkaure on
    // the right stands clear of them
    draw_palm(ctx, pal, GPoint(LM_X(10), horizon + 1), LM_H(24), -1);
    draw_palm(ctx, pal, GPoint(LM_X(20), horizon + 1), LM_H(17), 1);
}

#if defined(PBL_ROUND)
void giza_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_stele(ctx, frame->palette, frame->ground);
}
#endif

/** @} */
