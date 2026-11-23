/**
 * @file tablemountain.c
 * @brief The Table Mountain scene: the flat top between Devil's Peak and Lion's Head, the
 * tablecloth of cloud pouring over its edge when there is cloud about, the city along the bay,
 * the cable up the front with a car on it, which on a round screen carries the temperature.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/landmarks.h"

#include "scene/reading.h"
#include "draw/letter.h"
#include "draw/shape.h"
#include "sky/sky.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- The storm's bolts ---
// short ones in what sky the mountain leaves: over the saddle to Lion's Head, past Devil's Peak,
// and above the tablecloth. each table is placed for its own screen, since the round band's base
// does not follow LM_Y
#if defined(PBL_ROUND)
static const BoltSpot s_bolt_spots[] = {{204, 36, -1}, {84, 24, -1}, {46, 18, 1}};
#else
static const BoltSpot s_bolt_spots[] = {{160, 26, -1}, {16, 20, 1}, {64, 15, -1}};
#endif
SCENE_BOLTS(table_mountain_bolts, s_bolt_spots);

// the mountain: Devil's Peak on the left, the long flat table, then the saddle down to Lion's
// Head and Signal Hill on the right. its foot is hidden behind the city
static const GPoint s_mountain[] = {
    {LM_X(-10), LM_Y(108)}, {LM_X(-10), LM_Y(92)}, {LM_X(6), LM_Y(89)}, {LM_X(20), LM_Y(68)},
    {LM_X(30), LM_Y(78)}, {LM_X(40), LM_Y(73)}, {LM_X(48), LM_Y(60)}, {LM_X(150), LM_Y(58)},
    {LM_X(158), LM_Y(73)}, {LM_X(166), LM_Y(81)}, {LM_X(172), LM_Y(82)}, {LM_X(178), LM_Y(76)},
    {LM_X(184), LM_Y(71)}, {LM_X(190), LM_Y(72)}, {LM_X(196), LM_Y(79)}, {LM_X(204), LM_Y(87)},
    {LM_X(210), LM_Y(92)}, {LM_X(210), LM_Y(108)},
};

// the tablecloth: cloud lying along the top and spilling over the front cliff in tongues
static const GPoint s_tablecloth[] = {
    {LM_X(44), LM_Y(60)}, {LM_X(52), LM_Y(52)}, {LM_X(62), LM_Y(50)}, {LM_X(72), LM_Y(53)},
    {LM_X(80), LM_Y(51)}, {LM_X(92), LM_Y(48)}, {LM_X(104), LM_Y(50)}, {LM_X(116), LM_Y(49)},
    {LM_X(128), LM_Y(46)}, {LM_X(140), LM_Y(49)}, {LM_X(150), LM_Y(56)}, {LM_X(152), LM_Y(60)},
    {LM_X(150), LM_Y(64)}, {LM_X(146), LM_Y(70)}, {LM_X(140), LM_Y(66)}, {LM_X(134), LM_Y(72)},
    {LM_X(126), LM_Y(67)}, {LM_X(118), LM_Y(73)}, {LM_X(110), LM_Y(67)}, {LM_X(102), LM_Y(72)},
    {LM_X(94), LM_Y(67)}, {LM_X(86), LM_Y(71)}, {LM_X(78), LM_Y(66)}, {LM_X(70), LM_Y(69)},
    {LM_X(62), LM_Y(65)}, {LM_X(54), LM_Y(67)}, {LM_X(48), LM_Y(63)},
};

/** @brief One block of the city along the bay, standing on the shore. */
typedef struct
{
    int16_t x;
    uint8_t width;
    uint8_t height;
} Block;

// a low run of buildings along the shore, under the mountain
static const Block s_city[] = {
    {LM_X(10), LM_W(10), LM_H(5)}, {LM_X(24), LM_W(8), LM_H(6)},
    {LM_X(36), LM_W(12), LM_H(4)}, {LM_X(52), LM_W(8), LM_H(6)},
    {LM_X(64), LM_W(10), LM_H(5)}, {LM_X(80), LM_W(6), LM_H(7)},
    {LM_X(90), LM_W(12), LM_H(5)}, {LM_X(108), LM_W(8), LM_H(6)},
    {LM_X(120), LM_W(10), LM_H(4)}, {LM_X(134), LM_W(8), LM_H(6)},
    {LM_X(146), LM_W(12), LM_H(5)}, {LM_X(162), LM_W(8), LM_H(6)},
    {LM_X(176), LM_W(10), LM_H(4)}, {LM_X(190), LM_W(8), LM_H(6)},
};

// the cable up the front face, from its foot above the city to the upper station on the table's edge
#define CABLE_FOOT GPoint(LM_X(138), LM_Y(97))
#define CABLE_TOP GPoint(LM_X(126), LM_Y(58))

// --- The cable car ---
// a car hanging off the cable partway up by a grip and an arm, with a row of windows round its
// top. on a round screen the reading goes on its side under the windows, and the car is sized to
// it. the rectangle shows the reading in its stats row, so there the car is small and plain
#if defined(PBL_ROUND)
#define CAR_ALONG 60       ///< How far up the cable the car hangs, out of 100, clear of the city
#define CAR_DROP 6         ///< How far the car hangs below the cable
#define CAR_WINDOWS 6      ///< How tall the band of windows is
#define CAR_TEXT_H 16      ///< How tall the band the reading goes in is
#define CAR_PAD 6          ///< Air either side of the reading, inside the car
#else
#define CAR_ALONG 40
#define CAR_DROP 4
#define CAR_W 14           ///< How wide the small car is
#define CAR_H 9            ///< How tall it is
#endif

/**
 * @brief Draw the city along the shore, with whatever of it is awake.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param lit How lit the city is, 0 to 100.
 */
static void draw_city(GContext *ctx, const CommonPalette *pal, const SkylineGround *ground, int lit)
{
    int shore = LM_Y(LM_HORIZON);

    graphics_context_set_stroke_color(ctx, pal->ink);
    for (unsigned i = 0; i < ARRAY_LENGTH(s_city); i++)
    {
        const Block *b = &s_city[i];
        GRect block = GRect(b->x, shore - b->height, b->width, b->height);

        graphics_context_set_fill_color(ctx, ground->bridge);
        graphics_fill_rect(ctx, block, 0, GCornerNone);
        graphics_draw_rect(ctx, block);

        // one window a block, lit once the evening passes its place
        if (sketchbook_sky_light_rank((int)i, 3) < lit)
        {
            graphics_context_set_fill_color(ctx, ground->lamp);
            graphics_fill_rect(ctx, GRect(block.origin.x + block.size.w / 2 - 1, block.origin.y + 2, 2, 2), 0, GCornerNone);
        }
    }
}

/**
 * @brief Hang a cable car off the cable, by its grip and arm, with a row of windows round its top.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param width How wide the car is.
 * @param height How tall the car is.
 * @return The car's body, so a round screen can letter the reading on it.
 */
static GRect draw_car(GContext *ctx, const CommonPalette *pal, const SkylineGround *ground,
    int width, int height)
{
    GPoint foot = CABLE_FOOT;
    GPoint top = CABLE_TOP;
    GPoint hook = GPoint(foot.x + ((top.x - foot.x) * CAR_ALONG) / 100, foot.y + ((top.y - foot.y) * CAR_ALONG) / 100);
    GRect car = GRect(hook.x - width / 2, hook.y + CAR_DROP, width, height);

    // the grip clamped on the cable, and the arm hanging the car off it
    graphics_context_set_fill_color(ctx, ground->bridge);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    GRect grip = GRect(hook.x - 3, hook.y - 1, 6, 3);
    graphics_fill_rect(ctx, grip, 0, GCornerNone);
    graphics_draw_rect(ctx, grip);
    graphics_draw_line(ctx, GPoint(hook.x, hook.y + 2), GPoint(hook.x, car.origin.y - 1));

    // a roof across the top, then the car with its windows
    graphics_draw_line(ctx, GPoint(car.origin.x + 2, car.origin.y - 1), GPoint(car.origin.x + car.size.w - 3, car.origin.y - 1));
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, car, 3, GCornersAll);
    graphics_draw_round_rect(ctx, car, 3);

    graphics_context_set_fill_color(ctx, pal->ink);
    for (int x = car.origin.x + 3; x + 3 <= car.origin.x + car.size.w - 3; x += 5)
    {
        graphics_fill_rect(ctx, GRect(x, car.origin.y + 2, 3, 3), 0, GCornerNone);
    }

    return car;
}

#if defined(PBL_ROUND)
/**
 * @brief Hang the cable car with the temperature on its side, under its windows.
 *
 * Drawn after the weather, because the reading is the one thing on the face that has to stay
 * readable in every condition. The car is filled with the sky and lettered in ink, the one
 * pairing every palette guarantees. With no reading there is no car, rather than one reading "--".
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 */
static void draw_car_with_reading(GContext *ctx, const CommonPalette *pal, const SkylineGround *ground)
{
    char temp[16];
    int width;
    if (!reading_temp(temp, sizeof(temp), &width))
    {
        return;
    }

    GRect car = draw_car(ctx, pal, ground, width + CAR_PAD * 2, CAR_WINDOWS + CAR_TEXT_H);
    // the band starts a pixel inside the windows, which sits the digits in the middle of the side
    letter_board(ctx, temp, GRect(car.origin.x, car.origin.y + CAR_WINDOWS - 1, car.size.w, CAR_TEXT_H), pal->ink);
}
#endif

void table_mountain_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const SkylineGround *ground = frame->ground;
    int lit = sketchbook_sky_lit();

    shape_fill_outlined(ctx, s_mountain, ARRAY_LENGTH(s_mountain), ground->city, pal->ink);

    // the face of the cliff under the table, ruled down in short strokes
    for (int x = 58, i = 0; x <= 146; x += 8, i++)
    {
        int top = LM_Y(61) - (x - 58) / 40;
        graphics_draw_line(ctx, GPoint(LM_X(x), top), GPoint(LM_X(x), top + LM_H(8 + (i % 3) * 2)));
    }

    // the cable and its upper station, on the table's edge
    graphics_draw_line(ctx, CABLE_FOOT, CABLE_TOP);
    GRect station = GRect(CABLE_TOP.x - 4, CABLE_TOP.y - 3, 8, 4);
    graphics_context_set_fill_color(ctx, ground->bridge);
    graphics_fill_rect(ctx, station, 0, GCornerNone);
    graphics_draw_rect(ctx, station);

    // the tablecloth only forms when there is cloud about, which is when it really does
    if (frame->sky.clouds != CLOUDS_NONE)
    {
        shape_fill_outlined(ctx, s_tablecloth, ARRAY_LENGTH(s_tablecloth), pal->cloud, pal->ink);
    }

    draw_city(ctx, pal, ground, lit);
    landmarks_draw_water(ctx, bounds, ground->water, pal->ink, pal->dim);

    // the rectangle's car is scenery, so it rides behind the weather with the rest of the scene
#if !defined(PBL_ROUND)
    draw_car(ctx, pal, ground, CAR_W, CAR_H);
#endif
}

#if defined(PBL_ROUND)
void table_mountain_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    draw_car_with_reading(ctx, frame->palette, frame->ground);
}
#endif

/** @} */
