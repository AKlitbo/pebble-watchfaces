/**
 * @file scene.c
 * @brief The drawn crossing: the city standing back, the bridge over the river, the lamps that
 * come on at dusk, and what the water makes of them.
 *
 * @ingroup watchface-skyline
 */
#include "scene.h"

#include "sketchbook/fx/fx.h"
#include "sketchbook/sky/sky.h"
#include "clock/solar.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "ui/readouts.h"
#include "ui/fonts.h"
#include "sketchbook/draw/fonts.h"

/**
 * @addtogroup watchface-skyline
 * @{
 */

// the platform says how wide it is, so the city, the deck and the water run to whichever
// screen they are on rather than stopping at 200 partway across a round one
#define SCREEN_W PBL_DISPLAY_WIDTH

// the two bridge towers, and how far the deck runs past them to the bank each side
#if defined(PBL_ROUND)
#define TOWER_L 70
#define TOWER_R 190
#define TOWER_HALF 5
#else
#define TOWER_L 54
#define TOWER_R 146
#define TOWER_HALF 4
#endif

// lamps stand along the deck at this spacing, starting half a step in from the left bank
#define LAMP_STEP 22
#define LAMP_FIRST 11

// the deck is 3 rows thick, so this clears it and leaves a little open water before the mirror
// the reflections start below the waterline, not below the deck. keyed to the deck they sat above
// the water once the bank dropped, which put the crossing's mirror image on dry land
#define MIRROR_TOP (WATER_TOP + 4)

/** @brief One block in the city standing back behind the bridge. */
typedef struct
{
    int16_t x;
    uint8_t width;
    uint8_t height;
} Block;

// the far bank: a low, even run of blocks, deliberately plainer than the bridge in front of it.
// the crossing is the subject and the city is the backdrop, so nothing here competes for a look
#if defined(PBL_ROUND)
// the same skyline spread across the wider bank. the heights are deliberately uneven and a couple
// of them run well above the crossing: a row of blocks all one height reads as a wall, and a real
// skyline behind a bridge is mostly the two or three towers that clear it
static const Block s_city[] = {
    {3, 18, 24}, {23, 13, 42}, {39, 21, 18}, {62, 16, 58}, {81, 12, 28}, {95, 20, 36},
    {117, 14, 22}, {134, 17, 70}, {153, 13, 32}, {169, 21, 26}, {192, 16, 50}, {211, 12, 20},
    {225, 18, 40}, {246, 14, 16},
};
#else
static const Block s_city[] = {
    {2, 14, 20}, {18, 10, 34}, {30, 16, 15}, {48, 12, 46}, {62, 9, 23}, {73, 15, 30},
    {90, 11, 18}, {103, 13, 56}, {118, 10, 26}, {130, 16, 21}, {148, 12, 40}, {162, 9, 17},
    {173, 14, 32}, {189, 11, 14},
};
#endif

// a fixed field rather than a random one, so the sky is the same every time you look up at it
#if defined(PBL_ROUND)
// the same field spread to the wider sky. a circle cuts the corners hardest where this field is
// thickest, so the few stars that scaling put outside the glass were moved inwards to where
// there is sky rather than left to clip away
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
    {10, 30, 1}, {24, 42, 0}, {36, 27, 0}, {45, 36, 2}, {58, 29, 0}, {66, 46, 1},
    {78, 34, 0}, {88, 26, 1}, {97, 40, 0}, {108, 31, 2}, {119, 43, 0}, {129, 26, 1},
    {140, 38, 0}, {150, 23, 0}, {161, 43, 1}, {172, 30, 0}, {184, 46, 2}, {194, 27, 0},
    {16, 52, 0}, {31, 62, 1}, {50, 55, 0}, {63, 66, 0}, {83, 57, 1}, {94, 65, 0},
    {112, 54, 0}, {127, 63, 1}, {141, 52, 0}, {156, 66, 0}, {176, 56, 1}, {190, 64, 0},
    {6, 35, 0}, {71, 50, 0}, {103, 46, 0}, {167, 49, 0},
};
#endif

/**
 * @brief How lit the crossing is, from 0 by day to 100 in the small hours.
 *
 * The lamps do not simply follow darkness. They come on quickly at dusk, hold through the
 * evening, and the city behind them thins out as the night wears on, which is what makes the
 * scene read as a place with people in it rather than a light sensor.
 *
 * Built on solar_night_progress, which already handles the awkward parts: the night wrapping
 * past midnight, a missing reading, and how short a summer night is. All that is added here is
 * the shape over it.
 *
 * @return 0 to 100.
 */
static int lamps_lit(void)
{
    const struct tm *now = time_store_tm();
    int minutes = now->tm_hour * 60 + now->tm_min;

    int rise = weather_store_sunrise();
    int set = weather_store_sunset();
    if (rise < 0 || set < 0)
    {
        rise = 6 * 60;
        set = 18 * 60;
    }

    int night = solar_night_progress(rise, set, minutes);
    if (night < 0)
    {
        return 0;  // broad daylight, so only the handful that never go off
    }

    // up fast over the first fifth of the night, then a slow decline as the city turns in
    if (night < 20)
    {
        return (night * 100) / 20;
    }

    return 100 - ((night - 20) * 45) / 80;
}

/**
 * @brief A stable pseudo-random rank for one light, 0 to 99.
 *
 * Every lamp and window keeps its own place in the order things switch on, so the city fills up
 * the same way each evening instead of flickering at random from minute to minute. Cheap integer
 * hash: the multiply-and-shift scatters neighbours that would otherwise light in a row.
 *
 * @param a First coordinate of the light, e.g. its block.
 * @param b Second, e.g. which window of that block.
 */
static int light_rank(int a, int b)
{
    unsigned mixed = (unsigned)(a * 73856093) ^ (unsigned)(b * 19349663);
    return (int)((mixed >> 13) % 100);
}

/**
 * @brief Draw the city standing back behind the bridge, with whatever of it is awake.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param lit How lit the city is, 0 to 100.
 */
static void draw_city(GContext *ctx, const Palette *pal, int lit)
{
    graphics_context_set_fill_color(ctx, pal->city);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_city); i++)
    {
        const Block *b = &s_city[i];
        graphics_fill_rect(ctx, GRect(b->x, CITY_BASE - b->height, b->width, b->height), 0, GCornerNone);
    }

    // windows on a two by five grid inside each block, only where the block is tall enough to
    // hold them. a window is on once the city is lit past that window's own place in the queue
    graphics_context_set_fill_color(ctx, pal->lamp);

    for (unsigned i = 0; i < ARRAY_LENGTH(s_city); i++)
    {
        const Block *b = &s_city[i];
        int top = CITY_BASE - b->height;

        for (int row = 0; row < 5; row++)
        {
            int y = top + 3 + row * 5;
            if (y > CITY_BASE - 4)
            {
                break;
            }

            for (int col = 0; col < 2; col++)
            {
                int x = b->x + 3 + col * (b->width - 7);
                if (x + 2 > b->x + b->width - 2)
                {
                    continue;
                }

                if (light_rank((int)i, row * 2 + col) < lit)
                {
                    graphics_fill_rect(ctx, GRect(x, y, 2, 2), 0, GCornerNone);
                }
            }
        }
    }
}

/**
 * @brief Where the main cable hangs at a given column.
 *
 * A catenary is close enough to a parabola at this size, so each span is drawn as one: anchored
 * high at the bank, over each tower top, and sagging to just above the deck at mid-river. The
 * suspenders hang straight down off it, which is what makes the shape read as a suspension
 * bridge rather than an arch.
 *
 * @param x The column.
 * @return The cable's row.
 */
static int cable_y(int x)
{
    // held at the same fractions of the tower-to-deck drop the rectangle uses, so the curve keeps
    // its shape when the whole crossing moves down for a round screen
    const int anchor_y = PBL_IF_ROUND_ELSE(92, 76);    // where the cable meets each bank
    const int sag_y = PBL_IF_ROUND_ELSE(102, 84);      // the lowest it gets, mid-river

    if (x < TOWER_L)
    {
        // the short side span, rising from the bank to the tower top
        int along = (x * 100) / TOWER_L;
        return anchor_y - ((anchor_y - TOWER_TOP) * along) / 100;
    }

    if (x > TOWER_R)
    {
        int along = ((x - TOWER_R) * 100) / (SCREEN_W - TOWER_R);
        return TOWER_TOP + ((anchor_y - TOWER_TOP) * along) / 100;
    }

    // the main span: a parabola with its low point halfway across
    int half = (TOWER_R - TOWER_L) / 2;
    int from_mid = x - (TOWER_L + half);
    return sag_y - ((sag_y - TOWER_TOP) * from_mid * from_mid) / (half * half);
}

/**
 * @brief Draw the bridge: towers, deck, main cable and the suspenders off it.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_bridge(GContext *ctx, const Palette *pal)
{
    graphics_context_set_stroke_width(ctx, 1);

    // cables and outlines go in the linework colour rather than the bridge's own. by night the
    // bridge is near enough black, and black wire over black water is no bridge at all
    graphics_context_set_stroke_color(ctx, pal->ink);

    // the suspenders first, so the cable and the deck close over their ends. they run the whole
    // crossing rather than the main span alone, which is what a suspension bridge actually does:
    // the side spans hang off the same cable on its way down to the anchorages
    for (int x = 10; x < SCREEN_W; x += 10)
    {
        // nothing hangs off the towers themselves
        if ((x > TOWER_L - TOWER_HALF - 3 && x < TOWER_L + TOWER_HALF + 3) ||
            (x > TOWER_R - TOWER_HALF - 3 && x < TOWER_R + TOWER_HALF + 3))
        {
            continue;
        }

        graphics_draw_line(ctx, GPoint(x, cable_y(x)), GPoint(x, DECK_Y - 1));
    }

    // the main cable, walked column to column so the sag stays smooth
    int previous = cable_y(0);
    for (int x = 1; x < SCREEN_W; x++)
    {
        int y = cable_y(x);
        graphics_draw_line(ctx, GPoint(x - 1, previous), GPoint(x, y));
        previous = y;
    }

    // the two towers, carrying on below the deck into the river rather than stopping on it. cut
    // off at the roadway they read as a bridge floating on the water instead of standing in it,
    // and a suspension tower is a pier that happens to rise above the deck
    graphics_context_set_fill_color(ctx, pal->bridge);
    for (int t = 0; t < 2; t++)
    {
        int x = t == 0 ? TOWER_L : TOWER_R;
        GRect tower = GRect(x - TOWER_HALF, TOWER_TOP - 4, TOWER_HALF * 2,
                            DECK_Y - TOWER_TOP + 4 + PIER_DROP);

        graphics_fill_rect(ctx, tower, 0, GCornerNone);
        graphics_draw_rect(ctx, tower);
    }

    // and the deck, thick enough to read as a road but not so thick it becomes the horizon. the
    // roadway line on top of it is what the lamps stand on
    graphics_fill_rect(ctx, GRect(0, DECK_Y, SCREEN_W, 3), 0, GCornerNone);
    graphics_draw_line(ctx, GPoint(0, DECK_Y), GPoint(SCREEN_W - 1, DECK_Y));
}

/**
 * @brief Draw the lamps along the deck, lit or not.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param lit How lit the crossing is, 0 to 100.
 */
static void draw_lamps(GContext *ctx, const Palette *pal, int lit)
{
    for (int x = LAMP_FIRST, i = 0; x < SCREEN_W; x += LAMP_STEP, i++)
    {
        bool on = light_rank(i, 7) < lit;

        // a short post off the deck with the lamp on top of it. the post takes the linework
        // colour for the same reason the cables do: it has to hold against a dark sky
        graphics_context_set_stroke_color(ctx, pal->ink);
        graphics_draw_line(ctx, GPoint(x, DECK_Y - 1), GPoint(x, DECK_Y - 6));

        graphics_context_set_fill_color(ctx, on ? pal->lamp : pal->bridge);
        graphics_fill_circle(ctx, GPoint(x, DECK_Y - 7), 2);

        // an unlit lamp is the same shape, just not glowing, so it needs its outline back
        if (!on)
        {
            graphics_draw_circle(ctx, GPoint(x, DECK_Y - 7), 2);
        }
    }
}

/**
 * @brief Lay the river down and put the crossing back on it upside down.
 *
 * The water is a mirror rather than a shore: no swell, no edge, nothing moving. What gives it
 * away is entirely what it is reflecting, which is why every mark here is broken rather than
 * solid. The deck and the towers come back in the linework so the river reads as water in broad
 * daylight too, and the lamps add their streaks on top once they are on.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 * @param lit How lit the crossing is, 0 to 100.
 * @param minute The clock minute, which shifts the break pattern.
 */
static void draw_water(GContext *ctx, GRect bounds, const Palette *pal, int lit, int minute)
{
    static const int8_t wobble[] = {0, 1, -1, 0, 1, 0, -1, 1, 0, -1, 0, 1};

    graphics_context_set_fill_color(ctx, pal->water);
    graphics_fill_rect(ctx, GRect(0, WATER_TOP, bounds.size.w, bounds.size.h - WATER_TOP), 0, GCornerNone);

    // the far bank, which is the only hard edge the river has
    graphics_context_set_stroke_width(ctx, 1);
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_line(ctx, GPoint(0, WATER_TOP), GPoint(bounds.size.w - 1, WATER_TOP));

    // the deck coming back up at you. the dashes and the gaps between them both vary, because an
    // even one would read as a ruled line rather than as something floating on water
    static const uint8_t dash[] = {3, 6, 2, 5, 3, 7, 2, 4, 6, 3, 5, 2};

    graphics_context_set_stroke_color(ctx, pal->dim);
    for (int x = 0, i = 0; x < SCREEN_W; i++)
    {
        int len = dash[(unsigned)i % ARRAY_LENGTH(dash)];
        int y = MIRROR_TOP + wobble[(unsigned)i % ARRAY_LENGTH(wobble)];

        graphics_draw_line(ctx, GPoint(x, y), GPoint(x + len - 1, y));
        x += len + 3 + i % 3;
    }

    // and the open river past it, ticking over now and then. sparse on purpose: this is the half
    // the clock sits on, so it wants enough to read as water and no more
    for (int y = MIRROR_TOP + 8; y < SCENE_BOTTOM; y += 7)
    {
        for (int x = (y * 7) % 13; x < SCREEN_W; x += 31 + y % 7)
        {
            graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 2 + y % 3, y));
        }
    }

    // and the towers under it, breaking up and narrowing the further down they go, which is
    // what stops a stack of even bars reading as a barcode
    graphics_context_set_stroke_color(ctx, pal->bridge);
    for (int t = 0; t < 2; t++)
    {
        int x = t == 0 ? TOWER_L : TOWER_R;

        for (int y = MIRROR_TOP + 2, step = 0; y < MIRROR_TOP + 22; y += 2, step++)
        {
            if ((y + t * 3) % 5 == 0)
            {
                continue;  // a row the ripple has taken out
            }

            // each tick is its own length and its own offset, so what comes back is a column
            // wavering on the water rather than a stack of even bars
            int half = TOWER_HALF - step / 3 - (step + t) % 2;
            int shift = wobble[(unsigned)(y + t) % ARRAY_LENGTH(wobble)];
            graphics_draw_line(ctx, GPoint(x - half + shift, y), GPoint(x + half + shift, y));
        }
    }

    if (lit <= 0)
    {
        return;
    }

    graphics_context_set_stroke_color(ctx, pal->lamp);

    for (int x = LAMP_FIRST, i = 0; x < SCREEN_W; x += LAMP_STEP, i++)
    {
        if (light_rank(i, 7) >= lit)
        {
            continue;
        }

        // the brighter the night, the further down the water carries it
        int reach = MIRROR_TOP + 4 + (lit * (SCENE_BOTTOM - MIRROR_TOP - 4)) / 100;

        for (int y = DECK_Y + 4; y < reach; y++)
        {
            // a broken streak: some rows of the reflection are simply missing, and which ones
            // shift with the minute so the water is never quite still between glances
            if ((y + i * 3 + minute) % 5 == 0)
            {
                continue;
            }

            int shift = wobble[(unsigned)(y + i) % ARRAY_LENGTH(wobble)];
            graphics_draw_pixel(ctx, GPoint(x + shift, y));
        }
    }
}

#if defined(PBL_ROUND)
/**
 * @brief Float the temperature on a channel buoy, mid-river.
 *
 * A round screen has no room to stack a date and a reading under the clock, so the reading comes
 * off that stack and onto the water. A buoy carries its number painted on its side, which is a
 * real thing a river has rather than a label dropped onto the picture.
 *
 * Drawn after the weather, because the reading is the one thing on the face that has to stay
 * readable in every condition.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_buoy(GContext *ctx, const Palette *pal)
{
    // no location and no reading means no buoy, rather than one lettered "--"
    if (weather_store_temp() == WEATHER_NO_TEMP)
    {
        return;
    }

    char temp[16];
    readout_weather_temp(temp, sizeof(temp));

    GFont font = fonts_get(FONT_HAND_16);
    GSize reading = graphics_text_layout_get_content_size(temp, font,
        GRect(0, 0, 120, 26), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);

    int mid = SCREEN_W / 2;
    int width = reading.w + 12;
    GRect body = GRect(mid - width / 2, BUOY_TOP, width, BUOY_H);

    // the mast and its topmark first, so the body's outline closes over the foot of the mast
    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(mid, BUOY_TOP - BUOY_MAST), GPoint(mid, BUOY_TOP));
    graphics_context_set_fill_color(ctx, pal->ink);
    graphics_fill_circle(ctx, GPoint(mid, BUOY_TOP - BUOY_MAST), 2);

    // filled with the sky and lettered in ink, the one pairing the palette guarantees. filling it
    // with the water instead would rest on water-against-ink holding up in every theme
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, body, 2, GCornersAll);
    graphics_draw_round_rect(ctx, body, 2);

    graphics_context_set_text_color(ctx, pal->ink);
    graphics_draw_text(ctx, temp, font,
        GRect(body.origin.x, body.origin.y - 2, body.size.w, BUOY_H + 8),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

    // a wake either side of the waterline, so it sits in the river rather than on top of it
    graphics_context_set_stroke_color(ctx, pal->ink);
    int water = BUOY_TOP + BUOY_H - 3;
    graphics_draw_line(ctx, GPoint(body.origin.x - 7, water), GPoint(body.origin.x - 2, water));
    graphics_draw_line(ctx, GPoint(body.origin.x + width + 1, water), GPoint(body.origin.x + width + 6, water));
}
#endif

void scene_draw(GContext *ctx, GRect bounds, const Palette *pal)
{
    bool night = sketchbook_sky_night();
    SketchbookCond cond = sketchbook_cond_parse(weather_store_cond());
    SketchbookSky sky = sketchbook_sky_recipe(cond);
    GPoint disc = sketchbook_sky_arc_point(sketchbook_sky_arc_progress());
    int minute = time_store_tm()->tm_min;
    int lit = lamps_lit();

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

    // clouds sit in front of the disc but behind the city, so a low sun still goes down behind
    // the far bank rather than in front of it
    sketchbook_fx_draw_clouds(ctx, pal, &sky, disc);

    // the river goes down before the crossing, not after: the water starts at the far bank the
    // city stands on, so the bridge has to be painted on top of it to be over it
    draw_city(ctx, pal, lit);
    draw_water(ctx, bounds, pal, lit, minute);
    draw_bridge(ctx, pal);
    draw_lamps(ctx, pal, lit);

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

#if defined(PBL_ROUND)
    draw_buoy(ctx, pal);
#endif
}

/** @} */
