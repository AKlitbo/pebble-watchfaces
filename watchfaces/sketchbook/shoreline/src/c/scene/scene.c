/**
 * @file scene.c
 * @brief The drawn seascape: sky bands, stars, the sun or moon and its glitter on the water,
 * the sea and its waves, the boat, the surf line, and the beach.
 *
 * @ingroup watchface-shoreline
 */
#include "scene.h"

#include "sketchbook/fx/fx.h"
#include "sketchbook/sky/sky.h"
#include "clock/tide.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"

/**
 * @addtogroup watchface-shoreline
 * @{
 */

#define SCREEN_W 200
#define WAVE_ROWS 4
#define SEA_SPLIT_PCT 40  ///< How far down the water the far shade gives way to the near one
#define WET_FADE_H 10     ///< How many rows the wet sand dries out over
#define SURF_H 3          ///< How thick the surf band is
#define BIRD_Y 64         ///< The height the flock crosses at, under the cloud and over the water
#define BIRD_FLOCK_W 64   ///< How wide the flock is, for walking it on and off the screen

/** @brief One gull: where it sits in the flock, and half its wingspan. */
typedef struct
{
    int8_t  dx;
    int8_t  dy;
    uint8_t span;
} Gull;

// a fixed field rather than a random one, so the sky is the same every time you look up at it,
// which is what a sky does. it starts below the status bar and thins out towards the water
static const SketchbookStar s_stars[] = {
    {10, 28, 1}, {24, 38, 0}, {36, 25, 0}, {45, 33, 2}, {58, 27, 0}, {66, 42, 1},
    {78, 31, 0}, {88, 24, 1}, {97, 37, 0}, {108, 29, 2}, {119, 43, 0}, {129, 26, 1},
    {140, 35, 0}, {150, 23, 0}, {161, 40, 1}, {172, 30, 0}, {184, 43, 2}, {194, 27, 0},
    {16, 52, 0}, {31, 62, 1}, {50, 55, 0}, {63, 66, 0}, {83, 57, 1}, {94, 65, 0},
    {112, 54, 0}, {127, 63, 1}, {141, 52, 0}, {156, 66, 0}, {176, 56, 1}, {190, 64, 0},
    {6, 35, 0}, {71, 50, 0}, {103, 46, 0}, {167, 49, 0},
};

// chop goes with wind rather than with cold, so it rides the rain rows and the storm
// rather than the snow: a snowy sea can be dead flat, and a wet one usually is not
#define SHORELINE_CHOPPY_ROWS ((1u << SKETCHBOOK_COND_RAIN) | (1u << SKETCHBOOK_COND_FZRN) | (1u << SKETCHBOOK_COND_SHWR) | (1u << SKETCHBOOK_COND_STRM))

_Static_assert((SHORELINE_CHOPPY_ROWS & ((1u << SKETCHBOOK_COND_SNOW) | (1u << SKETCHBOOK_COND_SNSH))) == 0,
    "chop is a wind flag, not a cold one: it must never ride the snow rows");

/** @brief Whether this condition roughens the water. */
static bool choppy(SketchbookCond cond)
{
    return (SHORELINE_CHOPPY_ROWS >> cond) & 1u;
}

/**
 * @brief A running minute count for the tide.
 *
 * Taken off the clock the face is showing rather than off the system clock, so a dev fixture
 * pins the water where it pins everything else. Leap days are ignored: slipping a day once a
 * year only shifts the phase, and this is a rhythm rather than a tide table.
 */
static int32_t tide_minutes(void)
{
    const struct tm *now = time_store_tm();
    int32_t days = (int32_t)now->tm_year * 365 + now->tm_yday;

    return days * 1440 + now->tm_hour * 60 + now->tm_min;
}

/** @brief The waterline's baseline for a given tide level, before the shore curve bends it. */
static int waterline_base(int level)
{
    return TIDE_LO_Y + ((TIDE_HI_Y - TIDE_LO_Y) * level) / 100;
}

/**
 * @brief How far the water's edge wanders from its baseline at a given column.
 *
 * One long sweep carrying most of the amplitude, with a gentler roll laid over it. The sweep is
 * what the eye actually reads, so it gets the weight: two even waves would come out as a scallop
 * border. Neither lands on a whole number of cycles, so no stretch of the beach repeats another.
 *
 * The two are summed before the divide rather than rounded separately, which at this amplitude
 * is the difference between a ten-pixel sweep and a four-pixel one.
 *
 * @param x The column.
 * @return The offset in rows, within SHORE_SWING either way.
 */
static int shore_offset(int x)
{
    int32_t sweep = sin_lookup(DEG_TO_TRIGANGLE((x * 260) / SCREEN_W + 30));
    int32_t roll = sin_lookup(DEG_TO_TRIGANGLE((x * 540) / SCREEN_W + 200));

    return (int)((sweep * 7 + roll * 3) / TRIG_MAX_RATIO);
}

/** @brief Where the water's edge sits at a given column. */
static int waterline_at(int x, int base)
{
    return base + shore_offset(x);
}

/**
 * @brief Where the far water gives way to the near water at a given column.
 *
 * Carries the shore's full curve rather than a share of it, so the change of depth runs parallel
 * to the beach the way a bar of shallows does. Taking a fraction of the curve instead leaves it
 * cutting across the shore at a slight angle, which reads as a band laid over the sea.
 */
static int sea_split_at(int x, int base)
{
    return HORIZON_Y + ((base - HORIZON_Y) * SEA_SPLIT_PCT) / 100 + shore_offset(x);
}

/**
 * @brief Whether a pixel is kept at a given dither level, from an ordered 4x4 matrix.
 *
 * A plain `(x + y * 3) % 8` test is cheaper but lays the kept pixels along diagonals, and at
 * these sizes the eye reads those as hatching rather than as a fade.
 *
 * @param x The column.
 * @param y The row.
 * @param level How solid the fill is, 0 for none and 8 for all.
 */
static bool dither_keeps(int x, int y, int level)
{
    static const uint8_t matrix[4][4] = {
        {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5},
    };

    return matrix[(unsigned)y % 4][(unsigned)x % 4] < level * 2;
}

/**
 * @brief Draw the gulls crossing the sky. Day only, the way the stars are night only.
 *
 * Four strokes each, which at this size is the whole bird. The flock slides along on the minute
 * tick and takes a few hours to cross, so it has always moved by the next time you look up and
 * is never caught mid-flap.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param minutes The running minute count, which carries the flock across.
 */
static void draw_birds(GContext *ctx, const Palette *pal, int32_t minutes)
{
    // a loose flock rather than a row, since birds evenly spaced read as a border print
    static const Gull flock[] = {
        {0, 0, 7}, {18, -5, 5}, {33, 4, 8}, {48, -4, 6}, {60, 5, 6},
    };

    // the flock enters off the left and leaves off the right. the cycle is only one flock-width
    // longer than the screen, so there is barely a moment with an empty sky: gulls carrying on
    // out of shot for a stretch of every cycle is realistic and reads as a bug
    int lead = (int)(minutes % (SCREEN_W + BIRD_FLOCK_W)) - BIRD_FLOCK_W;

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);

    for (unsigned i = 0; i < ARRAY_LENGTH(flock); i++)
    {
        int x = lead + flock[i].dx;
        int y = BIRD_Y + flock[i].dy;
        int span = flock[i].span;
        int lift = 2;  // flat: a wing that rises as far as it reaches comes out as a mountain

        // two shallow chevrons meeting at the body, which is all a gull is this far off
        graphics_draw_line(ctx, GPoint(x - span, y), GPoint(x - span / 2, y - lift));
        graphics_draw_line(ctx, GPoint(x - span / 2, y - lift), GPoint(x, y));
        graphics_draw_line(ctx, GPoint(x, y), GPoint(x + span / 2, y - lift));
        graphics_draw_line(ctx, GPoint(x + span / 2, y - lift), GPoint(x + span, y));
    }
}

/**
 * @brief Lay down the water and the beach: the sea, the wet sand behind it, and the dry sand
 * the clock sits on, then rule the horizon over the top.
 *
 * The sea always starts at the horizon and the dry sand always starts at DRY_SAND_Y, so what the
 * tide actually moves is the curve between them: a falling tide reads as the sea narrowing and
 * the wet strip opening up behind it, which is what it looks like from the beach.
 *
 * Drawn a column at a time, one colour per pass rather than one column at a time, so the whole
 * thing costs three colour changes instead of six hundred.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 * @param pal The palette in use.
 * @param base The waterline's baseline, before the shore curve bends it.
 */
static void draw_water(GContext *ctx, GRect bounds, const Palette *pal, int base)
{
    int width = bounds.size.w;

    graphics_context_set_fill_color(ctx, pal->sand);
    graphics_fill_rect(ctx, GRect(0, DRY_SAND_Y, width, bounds.size.h - DRY_SAND_Y), 0, GCornerNone);

    graphics_context_set_stroke_width(ctx, 1);

    // the far water, from the horizon down to wherever this column's depth changes
    graphics_context_set_stroke_color(ctx, pal->sea_far);
    for (int x = 0; x < width; x++)
    {
        graphics_draw_line(ctx, GPoint(x, HORIZON_Y), GPoint(x, sea_split_at(x, base)));
    }

    // then the near water on down to the edge. the join needs no dither: it follows the same
    // curve as the shore, so it already reads as water rather than as a rule across the sea
    graphics_context_set_stroke_color(ctx, pal->sea);
    for (int x = 0; x < width; x++)
    {
        graphics_draw_line(ctx, GPoint(x, sea_split_at(x, base)), GPoint(x, waterline_at(x, base)));
    }

    // and the sand the sea has just come off, between the edge and the high-water mark. it closes
    // up to nothing where a full tide reaches DRY_SAND_Y and is widest at dead low
    graphics_context_set_stroke_color(ctx, pal->wet);
    for (int x = 0; x < width; x++)
    {
        graphics_draw_line(ctx, GPoint(x, waterline_at(x, base) + 1), GPoint(x, DRY_SAND_Y - 1));
    }

    // the wet strip is dried out from the bottom up rather than cut off, because a hard edge
    // along its foot is a line ruled across the beach and reads as exactly that
    graphics_context_set_stroke_color(ctx, pal->sand);
    for (int y = DRY_SAND_Y - WET_FADE_H; y < DRY_SAND_Y; y++)
    {
        int level = 8 - ((DRY_SAND_Y - y) * 8) / WET_FADE_H;
        for (int x = 0; x < width; x++)
        {
            if (y > waterline_at(x, base) && dither_keeps(x, y, level))
            {
                graphics_draw_pixel(ctx, GPoint(x, y));
            }
        }
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(0, HORIZON_Y), GPoint(width - 1, HORIZON_Y));
    graphics_context_set_stroke_width(ctx, 1);
}

/**
 * @brief Draw the disc's reflection running down the water towards you.
 *
 * Broken into separate dashes rather than drawn as a bar, and widening as it comes in, which is
 * what makes it read as light on a moving surface. It is the one thing that ties the sun or the
 * moon to the sea instead of leaving it pasted on the sky.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param disc Where the sun or moon is.
 * @param base The waterline's baseline.
 */
static void draw_glitter(GContext *ctx, const Palette *pal, GPoint disc, int base)
{
    static const int8_t wobble[] = {0, 2, -1, 1, -2, 0, 1, -1, 2, -2, 1, 0};

    int waterline = waterline_at(disc.x, base);
    int band = waterline - HORIZON_Y;

    graphics_context_set_stroke_color(ctx, pal->disc);
    graphics_context_set_stroke_width(ctx, 1);

    for (int y = HORIZON_Y + 1; y < waterline; y += 2)
    {
        int down = y - HORIZON_Y;
        int half = 1 + (down * 7) / band;
        int shift = wobble[(unsigned)(down / 2) % ARRAY_LENGTH(wobble)];

        graphics_draw_line(ctx, GPoint(disc.x - half + shift, y), GPoint(disc.x + half + shift, y));
    }
}

/**
 * @brief Draw the wave crests, in rows running away from you.
 *
 * The rows sit at fixed fractions of the water rather than at fixed rows, so they spread apart
 * as the tide comes in instead of leaving a gap at the bottom. Crest positions come from the
 * minute rather than a random number, so the sea creeps along on the tick that is already
 * happening and never costs a timer.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param base The waterline's baseline.
 * @param minute The clock minute, which slides the rows along.
 * @param choppy True to break the ranks up into rougher water.
 */
static void draw_waves(GContext *ctx, const Palette *pal, int base, int minute, bool choppy)
{
    graphics_context_set_stroke_color(ctx, pal->foam);
    graphics_context_set_stroke_width(ctx, 1);

    for (int row = 0; row < WAVE_ROWS; row++)
    {
        int dash = 2 + row;        // crests widen as they come in
        int gap = 15 + row * 6;    // and thin out, so the far water reads as further off
        int slide = (minute * (row + 2)) % gap;

        for (int x = slide - gap; x < SCREEN_W; x += gap)
        {
            // each crest takes its height from the water at its own column, so the whole row
            // bends with the shore instead of cutting across it
            int column = x < 0 ? 0 : (x >= SCREEN_W ? SCREEN_W - 1 : x);
            int y = HORIZON_Y + ((waterline_at(column, base) - HORIZON_Y) * (row + 1)) / (WAVE_ROWS + 1);

            graphics_draw_line(ctx, GPoint(x, y), GPoint(x + dash, y));

            if (choppy)
            {
                // a second mark half a gap along and a row up, which roughens the water without
                // needing a row of its own
                graphics_draw_line(ctx, GPoint(x + gap / 2, y - 1), GPoint(x + gap / 2 + dash - 1, y - 1));
            }
        }
    }
}

/**
 * @brief Draw the boat out on the water.
 *
 * It rides the tide: in with the flood, back out with the ebb, and it comes about at each turn
 * so it always faces the way it is going. Since the tide eases at the top and bottom of its
 * cycle, the boat slows and turns there too, without any of that being written twice.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param base The waterline's baseline.
 * @param level How far in the tide is, 0 to 100.
 * @param rising True while the tide is flooding.
 * @param minute The clock minute, which bobs it a pixel.
 */
static void draw_boat(GContext *ctx, const Palette *pal, int base, int level, bool rising, int minute)
{
    int x = 20 + (level * 160) / 100;
    int band = waterline_at(x, base) - HORIZON_Y;

    // sat in the near water rather than on the line where the far water gives way to it, so the
    // hull has one shade behind it instead of straddling two
    int y = HORIZON_Y + (band * 55) / 100 + ((minute % 3) - 1);
    int lean = rising ? 1 : -1;

    // the sail first, filled row by row so it needs no path and no allocation. a right triangle
    // hung off the mast, leaning whichever way the boat is pointed
    graphics_context_set_stroke_color(ctx, pal->foam);
    for (int row = 1; row <= 10; row++)
    {
        int width = (row * 7) / 10;
        graphics_draw_line(ctx, GPoint(x + lean, y - 14 + row), GPoint(x + lean * (1 + width), y - 14 + row));
    }

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_draw_line(ctx, GPoint(x, y - 14), GPoint(x, y - 3));

    // and the hull under it, three rows tapering to a keel
    for (int row = 0; row < 3; row++)
    {
        int half = 6 - row * 2;
        graphics_draw_line(ctx, GPoint(x - half, y - 2 + row), GPoint(x + half, y - 2 + row));
    }
}

/**
 * @brief Draw the surf along the water's edge.
 *
 * An even band hugging the edge, taking its whole shape from the shore curve underneath. Giving
 * it a wandering depth of its own was the thing that made it read as grass rather than water: at
 * three pixels thick, any variation is most of the band, so it comes out as teeth. The spray
 * above it is what breaks the line up instead.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param base The waterline's baseline.
 */
static void draw_surf(GContext *ctx, const Palette *pal, int base)
{
    // mostly nothing, so the flecks land in clumps rather than at a steady interval
    static const int8_t spray[] = {0, 0, 3, 0, 0, 0, 5, 0, 2, 0, 0, 4, 0, 0, 0, 3, 0, 0, 6, 0, 0, 2, 0};

    graphics_context_set_stroke_color(ctx, pal->foam);
    graphics_context_set_stroke_width(ctx, 1);

    for (int x = 0; x < SCREEN_W; x++)
    {
        int waterline = waterline_at(x, base);
        graphics_draw_line(ctx, GPoint(x, waterline - SURF_H + 1), GPoint(x, waterline));

        int lift = spray[(unsigned)x % ARRAY_LENGTH(spray)];
        if (lift)
        {
            graphics_draw_pixel(ctx, GPoint(x, waterline - SURF_H - lift));
        }
    }
}

void scene_draw(GContext *ctx, GRect bounds, const Palette *pal)
{
    bool night = sketchbook_sky_night();
    SketchbookCond cond = sketchbook_cond_parse(weather_store_cond());
    SketchbookSky sky = sketchbook_sky_recipe(cond);
    GPoint disc = sketchbook_sky_arc_point(sketchbook_sky_arc_progress());
    int32_t minutes = tide_minutes();
    int level = tide_level(minutes);
    int base = waterline_base(level);
    int minute = time_store_tm()->tm_min;

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

    // clouds sit in front of the disc but behind the water, so a low sun still goes down into
    // the sea rather than in front of it
    sketchbook_fx_draw_clouds(ctx, pal, &sky, disc);

    // gulls fly nearer than the cloud they pass under, and they keep the daytime sky occupied
    // the way the stars do at night
    if (!night)
    {
        draw_birds(ctx, pal, minutes);
    }

    draw_water(ctx, bounds, pal, base);

    // the reflection only exists while there is something up there to cast it
    if (sky.celestial != CEL_HIDDEN && disc.y < HORIZON_Y)
    {
        draw_glitter(ctx, pal, disc, base);
    }

    draw_waves(ctx, pal, base, minute, choppy(cond));

    draw_boat(ctx, pal, base, level, tide_rising(minutes), minute);

    draw_surf(ctx, pal, base);

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
}

/** @} */
