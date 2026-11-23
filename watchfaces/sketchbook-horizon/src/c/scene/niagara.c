/**
 * @file niagara.c
 * @brief The Niagara Falls scene: the Horseshoe Falls curving between the cliffs, the mist rising
 * off their foot with a rainbow in it while the sun is out, the river below, and on a round screen
 * a barrel bobbing in the river carrying the temperature.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/landmarks.h"

#include "scene/reading.h"
#include "draw/letter.h"
#include "draw/shape.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- The storm's bolts ---
// short ones in the strip of sky over the falls, so they stop above the crest and the cliffs. each
// table is placed for its own screen, since the round band's base does not follow LM_Y
#if defined(PBL_ROUND)
static const BoltSpot s_bolt_spots[] = {{80, 30, 1}, {194, 26, -1}, {136, 20, 1}};
#else
static const BoltSpot s_bolt_spots[] = {{60, 23, 1}, {156, 19, -1}, {108, 15, 1}};
#endif
SCENE_BOLTS(niagara_bolts, s_bolt_spots);

// the river above the falls, a band curving back from the brink
static const GPoint s_upper_river[] = {
    {LM_X(24), LM_Y(70)}, {LM_X(62), LM_Y(67)}, {LM_X(100), LM_Y(65)}, {LM_X(138), LM_Y(67)},
    {LM_X(176), LM_Y(70)}, {LM_X(170), LM_Y(73)}, {LM_X(135), LM_Y(70)}, {LM_X(100), LM_Y(69)},
    {LM_X(65), LM_Y(70)}, {LM_X(30), LM_Y(73)},
};

// the falls themselves: the curved brink along the top and the curtain of water dropping into the
// mist
static const GPoint s_falls[] = {
    {LM_X(30), LM_Y(73)}, {LM_X(65), LM_Y(70)}, {LM_X(100), LM_Y(69)}, {LM_X(135), LM_Y(70)},
    {LM_X(170), LM_Y(73)}, {LM_X(170), LM_Y(104)}, {LM_X(30), LM_Y(104)},
};

// the cliffs either side, Table Rock on the left and Goat Island on the right
static const GPoint s_cliff_left[] = {
    {LM_X(-10), LM_Y(108)}, {LM_X(-10), LM_Y(69)}, {LM_X(24), LM_Y(69)}, {LM_X(30), LM_Y(73)},
    {LM_X(32), LM_Y(108)},
};
static const GPoint s_cliff_right[] = {
    {LM_X(168), LM_Y(108)}, {LM_X(170), LM_Y(73)}, {LM_X(176), LM_Y(69)}, {LM_X(210), LM_Y(69)},
    {LM_X(210), LM_Y(108)},
};

// the trees along the cliff tops, by their centres
static const GPoint s_trees[] = {
    {LM_X(2), LM_Y(67)}, {LM_X(12), LM_Y(66)}, {LM_X(184), LM_Y(67)}, {LM_X(192), LM_Y(65)},
    {LM_X(201), LM_Y(67)},
};

// the mist boiling up off the foot of the falls, as soft puffs with no outline, each a centre
// and a radius. the curtain's streaks stop where the mist swallows them
static const struct
{
    int16_t x;
    int16_t y;
    uint8_t r;
} s_mist[] = {
    {36, 102, 6}, {48, 99, 7}, {62, 101, 6}, {76, 98, 8}, {92, 100, 7},
    {108, 98, 8}, {124, 100, 7}, {140, 99, 7}, {154, 101, 6}, {166, 102, 5},
};

#if defined(PBL_ROUND)
// --- The barrel ---
// a barrel bobbing in the river below the falls, with the reading on a label round its middle
#define BARREL_X 180       ///< The barrel's centre line
#define BARREL_TOP 138     ///< Top of the barrel
#define BARREL_H 20        ///< How tall the barrel lies
#define LABEL_H 16         ///< How tall the label is, room for the hand font's low-hanging digits
#define LABEL_PAD 4        ///< Air either side of the reading, inside the label
#endif

/**
 * @brief Draw the rainbow standing in the mist, while the sun is out.
 *
 * Two bands in the sun's colour and the rain's, so it takes its colours from the palette like
 * everything else rather than painting a rainbow onto a black and white page.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_rainbow(GContext *ctx, const CommonPalette *pal)
{
    GColor bands[] = {pal->disc, pal->precip};

    graphics_context_set_stroke_width(ctx, 1);
    for (unsigned b = 0; b < ARRAY_LENGTH(bands); b++)
    {
        graphics_context_set_stroke_color(ctx, bands[b]);

        // a half ellipse over the mist, walked in short legs, each band one stroke inside the last
        int rx = LM_W(30) - (int)b * 2;
        int ry = LM_H(24) - (int)b * 2;
        int cx = LM_X(100);
        int base = LM_Y(100);
        GPoint previous = GPoint(cx - rx, base);
        for (int step = 1; step <= 16; step++)
        {
            int32_t angle = TRIG_MAX_ANGLE / 2 - (TRIG_MAX_ANGLE / 2) * step / 16;
            GPoint next = GPoint(cx + (rx * cos_lookup(angle)) / TRIG_MAX_RATIO,
                base - (ry * sin_lookup(angle)) / TRIG_MAX_RATIO);
            graphics_draw_line(ctx, previous, next);
            previous = next;
        }
    }
}

#if defined(PBL_ROUND)
/**
 * @brief Float a barrel in the river with the temperature on its label.
 *
 * Drawn after the weather, because the reading is the one thing on the face that has to stay
 * readable in every condition. The label is filled with the sky and lettered in ink, the one
 * pairing every palette guarantees.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_barrel(GContext *ctx, const CommonPalette *pal, const SkylineGround *ground)
{
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;  // no reading, no barrel, rather than a label reading "--"
    }

    int label_w = width + LABEL_PAD * 2;
    GRect barrel = GRect(BARREL_X - label_w / 2 - 7, BARREL_TOP, label_w + 14, BARREL_H);
    GRect label = GRect(BARREL_X - label_w / 2, BARREL_TOP + (BARREL_H - LABEL_H) / 2, label_w, LABEL_H);

    graphics_context_set_fill_color(ctx, ground->bridge);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_fill_rect(ctx, barrel, 7, GCornersAll);
    graphics_draw_round_rect(ctx, barrel, 7);

    // a hoop either end, between the label and the barrel's rounded ends
    graphics_draw_line(ctx, GPoint(barrel.origin.x + 3, barrel.origin.y + 2), GPoint(barrel.origin.x + 3, barrel.origin.y + BARREL_H - 3));
    graphics_draw_line(ctx, GPoint(barrel.origin.x + barrel.size.w - 4, barrel.origin.y + 2),
        GPoint(barrel.origin.x + barrel.size.w - 4, barrel.origin.y + BARREL_H - 3));

    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, label, 0, GCornerNone);
    graphics_draw_rect(ctx, label);
    letter_board(ctx, temp, label, pal->ink);

    // a wake off either end, so it sits in the river rather than on top of it. it is a mark on the
    // water, so it takes the dim the ripples do
    int water = BARREL_TOP + BARREL_H - 1;
    graphics_context_set_stroke_color(ctx, pal->dim);
    graphics_draw_line(ctx, GPoint(barrel.origin.x - 8, water), GPoint(barrel.origin.x - 2, water));
    graphics_draw_line(ctx, GPoint(barrel.origin.x + barrel.size.w + 1, water), GPoint(barrel.origin.x + barrel.size.w + 9, water));
}
#endif

void niagara_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const SkylineGround *ground = frame->ground;

    shape_fill_outlined(ctx, s_upper_river, ARRAY_LENGTH(s_upper_river), ground->water, pal->ink);
    shape_fill_outlined(ctx, s_falls, ARRAY_LENGTH(s_falls), pal->cloud, pal->ink);

    // the water pouring over the brink, in close streaks down the curtain, some running further
    // than others so it reads as falling water rather than as railings
    graphics_context_set_stroke_color(ctx, pal->dim);
    for (int x = 34, i = 0; x < 168; x += 4, i++)
    {
        int from_mid = x > 100 ? x - 100 : 100 - x;
        int brink = 69 + (from_mid * from_mid * 4) / (70 * 70);
        int reach = 98 - (i % 3) * 4;
        graphics_draw_line(ctx, GPoint(LM_X(x), LM_Y(brink + 2)), GPoint(LM_X(x), LM_Y(reach)));
    }

    if (!frame->night && frame->sky.celestial != CEL_HIDDEN)
    {
        draw_rainbow(ctx, pal);
    }

    graphics_context_set_fill_color(ctx, pal->cloud);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_mist); i++)
    {
        graphics_fill_circle(ctx, GPoint(LM_X(s_mist[i].x), LM_Y(s_mist[i].y)), LM_W(s_mist[i].r));
    }
    graphics_fill_rect(ctx, GRect(LM_X(30), LM_Y(102), LM_X(170) - LM_X(30), LM_Y(108) - LM_Y(102)), 0, GCornerNone);

    shape_fill_outlined(ctx, s_cliff_left, ARRAY_LENGTH(s_cliff_left), ground->city, pal->ink);
    shape_fill_outlined(ctx, s_cliff_right, ARRAY_LENGTH(s_cliff_right), ground->city, pal->ink);
    graphics_context_set_fill_color(ctx, ground->bridge);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_trees); i++)
    {
        int r = (i % 2) ? LM_W(4) : LM_W(3);
        graphics_fill_circle(ctx, s_trees[i], r);
        graphics_draw_circle(ctx, s_trees[i], r);
    }

    landmarks_draw_water(ctx, bounds, ground->water, pal->ink, pal->dim);
}

#if defined(PBL_ROUND)
void niagara_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_barrel(ctx, frame->palette, frame->ground);
}
#endif

/** @} */
