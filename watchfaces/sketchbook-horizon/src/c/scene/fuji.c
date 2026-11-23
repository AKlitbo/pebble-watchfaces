/**
 * @file fuji.c
 * @brief The Fuji scene: the cone with its flat crater top and its cap of snow, a band of forest
 * along the far shore, and the lake in front holding the sky.
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
// one either side of the cone and a small one at the far left, all in the open sky beside it. each
// table is placed for its own screen, since the round band's base does not follow LM_Y
#if defined(PBL_ROUND)
static const BoltSpot s_bolt_spots[] = {{198, 48, -1}, {80, 34, 1}, {44, 22, 1}};
#else
static const BoltSpot s_bolt_spots[] = {{46, 40, 1}, {152, 32, -1}, {16, 20, 1}};
#endif
SCENE_BOLTS(fuji_bolts, s_bolt_spots);

// the cone. its slopes curve outwards as they come down, which is what makes it Fuji rather than
// any mountain, so each side is walked in short legs that flatten towards the foot. the top is the
// crater rim, a short run of small notches rather than a point
static const GPoint s_cone[] = {
    {LM_X(-10), LM_Y(108)}, {LM_X(-10), LM_Y(103)}, {LM_X(10), LM_Y(100)}, {LM_X(30), LM_Y(93)},
    {LM_X(48), LM_Y(82)}, {LM_X(62), LM_Y(70)}, {LM_X(74), LM_Y(59)}, {LM_X(84), LM_Y(52)},
    {LM_X(90), LM_Y(50)}, {LM_X(94), LM_Y(52)}, {LM_X(100), LM_Y(49)}, {LM_X(106), LM_Y(52)},
    {LM_X(110), LM_Y(50)}, {LM_X(116), LM_Y(52)}, {LM_X(126), LM_Y(59)}, {LM_X(138), LM_Y(70)},
    {LM_X(152), LM_Y(82)}, {LM_X(170), LM_Y(93)}, {LM_X(190), LM_Y(100)}, {LM_X(210), LM_Y(103)},
    {LM_X(210), LM_Y(108)},
};

// the crater rim runs from the eighth corner of the cone to the fourteenth
#define CONE_RIM_FIRST 7
#define CONE_RIM_LAST 13

// the snow lies down from the rim and stops in a torn line, deeper in the gullies than on the
// ribs between them. the corners keep inside the cone's slopes so the cap never sticks out of it
static const GPoint s_cap[] = {
    {LM_X(84), LM_Y(52)}, {LM_X(90), LM_Y(50)}, {LM_X(94), LM_Y(52)}, {LM_X(100), LM_Y(49)},
    {LM_X(106), LM_Y(52)}, {LM_X(110), LM_Y(50)}, {LM_X(116), LM_Y(52)}, {LM_X(124), LM_Y(58)},
    {LM_X(128), LM_Y(63)}, {LM_X(123), LM_Y(61)}, {LM_X(119), LM_Y(67)}, {LM_X(114), LM_Y(61)},
    {LM_X(109), LM_Y(69)}, {LM_X(104), LM_Y(62)}, {LM_X(100), LM_Y(68)}, {LM_X(96), LM_Y(62)},
    {LM_X(91), LM_Y(69)}, {LM_X(86), LM_Y(61)}, {LM_X(81), LM_Y(66)}, {LM_X(76), LM_Y(61)},
    {LM_X(72), LM_Y(61)},
};

// the gullies running down off the snowline, each one starting at the foot of a tongue of snow.
// short of the base, since a crease all the way down reads as a ruled line
static const GPoint s_gullies[][2] = {
    {{LM_X(81), LM_Y(66)}, {LM_X(56), LM_Y(92)}}, {{LM_X(86), LM_Y(61)}, {LM_X(66), LM_Y(84)}},
    {{LM_X(91), LM_Y(69)}, {LM_X(80), LM_Y(94)}}, {{LM_X(100), LM_Y(68)}, {LM_X(99), LM_Y(96)}},
    {{LM_X(109), LM_Y(69)}, {LM_X(120), LM_Y(94)}}, {{LM_X(114), LM_Y(61)}, {LM_X(134), LM_Y(84)}},
    {{LM_X(119), LM_Y(67)}, {LM_X(144), LM_Y(92)}},
};

// the forest along the far shore, its top edge broken into the crowns of the trees
static const GPoint s_forest[] = {
    {LM_X(-10), LM_Y(108)}, {LM_X(-10), LM_Y(103)}, {LM_X(0), LM_Y(101)}, {LM_X(8), LM_Y(104)},
    {LM_X(16), LM_Y(100)}, {LM_X(24), LM_Y(104)}, {LM_X(34), LM_Y(102)}, {LM_X(44), LM_Y(105)},
    {LM_X(56), LM_Y(103)}, {LM_X(70), LM_Y(106)}, {LM_X(84), LM_Y(104)}, {LM_X(100), LM_Y(106)},
    {LM_X(116), LM_Y(104)}, {LM_X(130), LM_Y(106)}, {LM_X(144), LM_Y(103)}, {LM_X(156), LM_Y(105)},
    {LM_X(166), LM_Y(101)}, {LM_X(176), LM_Y(104)}, {LM_X(186), LM_Y(100)}, {LM_X(196), LM_Y(103)},
    {LM_X(210), LM_Y(101)}, {LM_X(210), LM_Y(108)},
};

// the tall pines standing up out of the forest at either end, where the cone is low enough to
// leave them sky to stand against. each is the tree's tip, and the tree widens from there down to
// the forest
static const GPoint s_pines[] = {
    {LM_X(5), LM_Y(91)}, {LM_X(13), LM_Y(95)}, {LM_X(22), LM_Y(89)}, {LM_X(31), LM_Y(96)},
    {LM_X(170), LM_Y(95)}, {LM_X(179), LM_Y(88)}, {LM_X(188), LM_Y(93)}, {LM_X(196), LM_Y(90)},
};

// the lake runs from the far shore down to the near bank the clock sits on. a round screen has
// the room for a deeper one, deep enough to float the boat that carries the reading
#define LAKE_BOTTOM PBL_IF_ROUND_ELSE(160, 118)

// grass on the near bank, scattered unevenly the way Ridgeline's meadow is. a fixed scatter rather
// than a random one, so the bank is the same every time you look at it. nothing sits below the
// clock's cap line across the middle, or the tufts come up between the digits. out at the sides
// of a round screen the digits never reach, so the grass carries on down the edges there. the
// rectangle's bank is only a narrow strip, so it gets a handful
static const GPoint s_tufts[] = {
#if defined(PBL_ROUND)
    {16, 165}, {34, 163}, {52, 166}, {70, 163}, {92, 165}, {114, 163},
    {140, 166}, {162, 163}, {184, 165}, {204, 163}, {224, 166}, {242, 164},
    {20, 182}, {34, 192}, {230, 180}, {240, 191},
#else
    {10, 124}, {34, 127}, {62, 123}, {142, 126}, {170, 123}, {192, 127},
#endif
};

#if defined(PBL_ROUND)
// --- The boat ---
// a small boat out on the lake under a square sail, with the reading lettered on the sail. the
// whole boat sits on the water, below the far shore, so it reads as a boat rather than a sign
#define BOAT_X 130         ///< The mast's centre line
#define SAIL_TOP 133       ///< Top of the sail, under its yard
#define SAIL_H 16          ///< How tall the sail is
#define SAIL_PAD 6         ///< Air either side of the reading, inside the sail
#define HULL_TOP 151       ///< The gunwale
#define HULL_H 5           ///< How deep the hull sits
#endif

/**
 * @brief Draw one pine: a narrow spike of a tree on a short trunk.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param tip The tree's top.
 */
static void draw_pine(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground, GPoint tip)
{
    int base = LM_Y(105);
    int half = (base - tip.y) / 4 + 2;
    GPoint tree[] = {{tip.x - half, base}, tip, {tip.x + half, base}};

    shape_fill_outlined(ctx, tree, ARRAY_LENGTH(tree), ground->land, pal->ink);
}

/**
 * @brief Lay the lake down and give it the mountain back, upside down and broken up.
 *
 * The lake is the colour of the sky low down, since still water holds the sky, and the cone comes
 * back in its own colour as a few wavering bands under the peak.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_lake(GContext *ctx, GRect bounds, const CommonPalette *pal, const RidgelineGround *ground)
{
    int top = LM_Y(LM_HORIZON);

    graphics_context_set_fill_color(ctx, pal->sky_lo);
    graphics_fill_rect(ctx, GRect(0, top, bounds.size.w, LAKE_BOTTOM - top), 0, GCornerNone);

    // the mountain under the water, a little narrower on every row down
    graphics_context_set_stroke_color(ctx, ground->land_far);
    graphics_context_set_stroke_width(ctx, 1);
    for (int y = top + 2, half = LM_W(40); y < LAKE_BOTTOM - 1 && half > 2; y += 2, half -= LM_W(7))
    {
        graphics_draw_line(ctx, GPoint(LM_X(100) - half, y), GPoint(LM_X(100) + half, y));
    }

    // a few broken marks across the open water so it reads as a lake and not as more sky
    graphics_context_set_stroke_color(ctx, pal->ink);
    for (int y = top + 3; y < LAKE_BOTTOM - 1; y += 5)
    {
        for (int x = (y * 11) % 17; x < bounds.size.w; x += 23 + y % 9)
        {
            graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 3 + y % 4, y));
        }
    }

    // the near bank, which is the ground the clock stands on
    ground_fill(ctx, bounds, LAKE_BOTTOM, ground->land, pal->ink);
}

#if defined(PBL_ROUND)
/**
 * @brief Sail a boat across the lake with the temperature on its sail.
 *
 * Drawn after the weather, because the reading is the one thing on the face that has to stay
 * readable in every condition. The sail is filled with the sky and lettered in ink, the one
 * pairing every palette guarantees.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_boat(GContext *ctx, const CommonPalette *pal, const RidgelineGround *ground)
{
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;  // no reading, no boat, rather than a sail lettered "--"
    }

    GRect sail = GRect(BOAT_X - (width + SAIL_PAD * 2) / 2, SAIL_TOP, width + SAIL_PAD * 2, SAIL_H);
    int hull_half = sail.size.w / 2 + 6;

    // the hull first, a little wider than the sail and narrowing to the keel
    GPoint hull[] = {
        {BOAT_X - hull_half, HULL_TOP}, {BOAT_X + hull_half, HULL_TOP},
        {BOAT_X + hull_half - 5, HULL_TOP + HULL_H}, {BOAT_X - hull_half + 5, HULL_TOP + HULL_H},
    };
    shape_fill_outlined(ctx, hull, ARRAY_LENGTH(hull), ground->land_far, pal->ink);

    // the mast, then the sail over it, then the yard the sail hangs from
    graphics_draw_line(ctx, GPoint(BOAT_X, SAIL_TOP - 4), GPoint(BOAT_X, HULL_TOP));

    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, sail, 0, GCornerNone);
    graphics_draw_rect(ctx, sail);

    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, GPoint(sail.origin.x - 2, SAIL_TOP), GPoint(sail.origin.x + sail.size.w + 1, SAIL_TOP));
    graphics_context_set_stroke_width(ctx, 1);

    letter_board(ctx, temp, sail, pal->ink);

    // a wake off either end of the hull, so it sits in the lake rather than on top of it
    int water = HULL_TOP + HULL_H - 1;
    graphics_draw_line(ctx, GPoint(BOAT_X - hull_half - 8, water), GPoint(BOAT_X - hull_half - 2, water));
    graphics_draw_line(ctx, GPoint(BOAT_X + hull_half + 2, water), GPoint(BOAT_X + hull_half + 8, water));
}
#endif

void fuji_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const RidgelineGround *ground = frame->ground;

    shape_fill_outlined(ctx, s_cone, ARRAY_LENGTH(s_cone), ground->land_far, pal->ink);

    // the gullies go on before the snow, so the snow closes over their heads
    line_segments(ctx, s_gullies, ARRAY_LENGTH(s_gullies));

    // Fuji keeps its snow all year, so the cap is always on. white whatever the palette, the
    // way the snow on Ridgeline's peaks is
    shape_fill_outlined(ctx, s_cap, ARRAY_LENGTH(s_cap), GColorWhite, pal->ink);

    // the rim goes over the cap's top edge in a heavier line, so the summit holds its shape
    // against a white sky
    graphics_context_set_stroke_width(ctx, 2);
    line_polyline(ctx, &s_cone[CONE_RIM_FIRST], CONE_RIM_LAST - CONE_RIM_FIRST + 1);
    graphics_context_set_stroke_width(ctx, 1);

    draw_lake(ctx, bounds, pal, ground);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_pines); i++)
    {
        draw_pine(ctx, pal, ground, s_pines[i]);
    }
    shape_fill_outlined(ctx, s_forest, ARRAY_LENGTH(s_forest), ground->land, pal->ink);

    // the tufts are marks on the ground, so they take the palette's dim, the colour the date and
    // readouts use down there
    marks_tufts(ctx, s_tufts, ARRAY_LENGTH(s_tufts), pal->dim);
}

#if defined(PBL_ROUND)
void fuji_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_boat(ctx, frame->palette, frame->ground);
}
#endif

/** @} */
