/**
 * @file shoreline.c
 * @brief The drawn seascape: the gulls, the sun or moon's glitter on the water, the sea and its
 * waves, the boat, the surf line, and the beach. Its stars and where its sky and weather sit are
 * here too.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/shoreline.h"

#include "fx/fx.h"
#include "sky/sky.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "ui/readouts.h"
#include "ui/fonts.h"
#include "draw/fonts.h"
#include "draw/marks.h"
#include "draw/measure.h"
#include "clock/tide.h"
#include "theme/shoreline.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- Scene geometry ---
#if defined(PBL_ROUND)
#define HORIZON_Y 93    ///< Where the sea meets the sky, and the arc's baseline
#else
#define HORIZON_Y 76    ///< Where the sea meets the sky, and the arc's baseline
#endif

// the round boat, which is big enough to fly the temperature on a pennant
#define BOAT_MAST_H 25     ///< Mast height above the hull
#define BOAT_FLAG_W 39     ///< Four characters with margin: "120F" and "-12C" both sit clear of the edge
#define BOAT_FLAG_H 15
// how far in from each edge the boat's run stops. it has to clear its own pennant, which flies
// off one side, so this is wider than the hull alone would need
#define BOAT_MARGIN PBL_IF_ROUND_ELSE(56, 20)

// the water's edge is a curve, not a rule. the tide slides the whole curve up and down between
// these two, and every column of water is measured off wherever the curve has got to at that x.
// the swing is worth more than the tide range is: a shore that sweeps across the beach reads as
// a coastline, and one that runs straight reads as a ruled line whatever the water is doing
#if defined(PBL_ROUND)
#define TIDE_LO_Y 131      ///< The curve's baseline at dead low, with the sea at its narrowest
#define TIDE_HI_Y 150      ///< The curve's baseline at high water
#define DRY_SAND_MARGIN 3  ///< Rows of wet sand still showing below the curve at the top of a full tide
#else
#define TIDE_LO_Y 107      ///< The curve's baseline at dead low, with the sea at its narrowest
#define TIDE_HI_Y 123      ///< The curve's baseline at high water
#define DRY_SAND_MARGIN 0  ///< Rows of wet sand still showing below the curve at the top of a full tide
#endif

// the curve is one long sweep with a gentler roll laid over it, and the two together are how far
// it wanders either side of its baseline
#define SHORE_SWEEP 7
#define SHORE_ROLL 3
#define SHORE_SWING (SHORE_SWEEP + SHORE_ROLL)

// the dry sand starts here whatever the sea is doing, which is what gives the clock one home
// while the water comes and goes. it has to clear the curve at its lowest reach on a full tide
#define DRY_SAND_Y (TIDE_HI_Y + SHORE_SWING + DRY_SAND_MARGIN)

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

// --- Where this scene's sky and weather sit ---
// the arc's baseline is the horizon, so the sun and moon rise out of the sea and set back into it.
// the overcast band hangs low in the sky and the storm's bolts strike out of it, and the sea haze
// wash sits down over the water
// the bolts strike the open sea, clear of the boat and short of the sand
#if defined(PBL_ROUND)
static const Puff s_overcast[] = {
    {18, 68, 34, 11}, {70, 65, 32, 11}, {124, 72, 36, 11}, {178, 66, 33, 11}, {212, 73, 30, 10},
};
static const BoltSpot s_bolt_spots[] = {{72, 34, -1}, {168, 30, -1}, {130, 18, 1}};
const SceneGeometry shoreline_geometry = {
    .sky_band_y = 49, .sky_blend_h = 27,
    .arc_base_y = 93, .arc_rx = 114, .arc_ry = 51,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 22, .cloud_y_min = 66, .cloud_y_max = 81,
    .drift_lobe_a = 8, .drift_lobe_b = 9,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 37, .precip_h = 115,
    .wash_top = 78, .wash_bottom = 148, .wash_wisp_hi_y = 125, .wash_wisp_lo_y = 138,
};
#else
static const Puff s_overcast[] = {
    {14, 56, 26, 9}, {58, 53, 24, 9}, {104, 59, 27, 9}, {150, 54, 25, 9}, {192, 60, 22, 8},
};
static const BoltSpot s_bolt_spots[] = {{60, 30, 1}, {148, 24, 1}, {108, 18, 1}};
const SceneGeometry shoreline_geometry = {
    .sky_band_y = 40, .sky_blend_h = 22,
    .arc_base_y = 76, .arc_rx = 88, .arc_ry = 42,
    .stars = s_stars, .star_count = ARRAY_LENGTH(s_stars),
    .cloud_drop = 18, .cloud_y_min = 54, .cloud_y_max = 66,
    .drift_lobe_a = 8, .drift_lobe_b = 9,
    .overcast = s_overcast, .overcast_count = ARRAY_LENGTH(s_overcast),
    .precip_top = 30, .precip_h = 94,
    .wash_top = 64, .wash_bottom = 121, .wash_wisp_hi_y = 102, .wash_wisp_lo_y = 113,
};
#endif
SCENE_BOLTS(shoreline_bolts, s_bolt_spots);

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

// chop goes with wind rather than with cold, so it rides the rain rows and the storm
// rather than the snow: a snowy sea can be dead flat, and a wet one usually is not
#define SHORELINE_CHOPPY_ROWS ((1u << WX_COND_RAIN) | (1u << WX_COND_FZRN) | (1u << WX_COND_SHWR) | (1u << WX_COND_STRM))

_Static_assert((SHORELINE_CHOPPY_ROWS & ((1u << WX_COND_SNOW) | (1u << WX_COND_SNSH))) == 0,
    "chop is a wind flag, not a cold one: it must never ride the snow rows");

/** @brief Whether this condition roughens the water. */
static bool choppy(WxCond cond)
{
    return cond < WX_COND_COUNT && ((SHORELINE_CHOPPY_ROWS >> cond) & 1u);
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

/**
 * @brief The waterline's baseline for a given tide level, before the shore curve bends it. It rounds
 * to the nearest pixel, so the water holds its high mark about as long as its low one rather than
 * reaching it only at a level of exactly 100.
 */
static int waterline_base(int level)
{
    return TIDE_LO_Y + ((TIDE_HI_Y - TIDE_LO_Y) * level + 50) / 100;
}

/** @brief Where the tide stands on the clock the face is showing. */
typedef struct
{
    int32_t minutes;  ///< The running minute count, which the gulls ride on too
    int level;        ///< How far in the tide has come, 0 to 100
    int base;         ///< The waterline's baseline, before the shore curve bends it
    bool rising;      ///< True while the tide is flooding
} TideNow;

/**
 * @brief Read the tide off the clock.
 *
 * The ground and the front both read it here, so the hull and the mast it carries always ride the
 * same water.
 *
 * @return Where the tide stands.
 */
static TideNow tide_now(void)
{
    int32_t minutes = tide_minutes();
    int level = tide_level(minutes);

    return (TideNow){
        .minutes = minutes, .level = level, .base = waterline_base(level), .rising = tide_rising(minutes),
    };
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

    return (int)((sweep * SHORE_SWEEP + roll * SHORE_ROLL) / TRIG_MAX_RATIO);
}

/**
 * @brief Work the shore curve out for every column of the screen, once a frame.
 *
 * Each column's offset costs two sine lookups and a handful of divides, and the passes over the
 * water need it about fifteen times a column between them, so each column is worked out once and
 * read back from here. The glitter and the boat each want a single column, so they call
 * shore_offset for it.
 *
 * @param shore Receives each column's offset from the waterline's baseline.
 */
static void shore_curve(int8_t shore[SCREEN_W])
{
    for (int x = 0; x < SCREEN_W; x++)
    {
        shore[x] = (int8_t)shore_offset(x);
    }
}

/** @brief Where the water's edge sits at a given column of the screen. */
static int waterline_at(const int8_t *shore, int x, int base)
{
    return base + shore[x];
}

/**
 * @brief Where the far water gives way to the near water at a given column.
 *
 * Carries the shore's full curve rather than a share of it, so the change of depth runs parallel
 * to the beach the way a bar of shallows does. Taking a fraction of the curve instead leaves it
 * cutting across the shore at a slight angle, which reads as a band laid over the sea.
 */
static int sea_split_at(const int8_t *shore, int x, int base)
{
    return HORIZON_Y + ((base - HORIZON_Y) * SEA_SPLIT_PCT) / 100 + shore[x];
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
 * tick and takes a few hours to cross, so it has always moved by the next time you look up. In a
 * burst the gulls beat their wings, each out of step with the next.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param minutes The running minute count, which carries the flock across.
 * @param anim Where the burst has got to.
 */
static void draw_birds(GContext *ctx, const CommonPalette *pal, int32_t minutes, AnimPhase anim)
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

        // a downstroke drops the wingtips under the body for a frame or two
        if (anim.tick && ((anim.tick + (int)i) / 2) % 2)
        {
            lift = -2;
        }

        marks_bird(ctx, GPoint(x, y), span, lift);
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
 * @param ground The scene's ground row.
 * @param shore The shore curve, one offset per column.
 * @param base The waterline's baseline, before the shore curve bends it.
 */
static void draw_water(GContext *ctx, GRect bounds, const CommonPalette *pal,
    const ShorelineGround *ground, const int8_t *shore, int base)
{
    int width = bounds.size.w;
    // the curve only runs as wide as the screen, and the bounds are the screen
    int columns = width < SCREEN_W ? width : SCREEN_W;

    graphics_context_set_fill_color(ctx, ground->sand);
    graphics_fill_rect(ctx, GRect(0, DRY_SAND_Y, width, bounds.size.h - DRY_SAND_Y), 0, GCornerNone);

    graphics_context_set_stroke_width(ctx, 1);

    // the far water, from the horizon down to wherever this column's depth changes
    graphics_context_set_stroke_color(ctx, ground->sea_far);
    for (int x = 0; x < columns; x++)
    {
        graphics_draw_line(ctx, GPoint(x, HORIZON_Y), GPoint(x, sea_split_at(shore, x, base)));
    }

    // then the near water on down to the edge. the join needs no dither: it follows the same
    // curve as the shore, so it already reads as water rather than as a rule across the sea
    graphics_context_set_stroke_color(ctx, ground->sea);
    for (int x = 0; x < columns; x++)
    {
        graphics_draw_line(ctx, GPoint(x, sea_split_at(shore, x, base)),
                                GPoint(x, waterline_at(shore, x, base)));
    }

    // and the sand the sea has just come off, between the edge and the high-water mark. it closes
    // up to nothing where a full tide reaches DRY_SAND_Y and is widest at dead low
    graphics_context_set_stroke_color(ctx, ground->wet);
    for (int x = 0; x < columns; x++)
    {
        graphics_draw_line(ctx, GPoint(x, waterline_at(shore, x, base) + 1), GPoint(x, DRY_SAND_Y - 1));
    }

    // the wet strip is dried out from the bottom up rather than cut off, because a hard edge
    // along its foot is a line ruled across the beach and reads as exactly that
    graphics_context_set_stroke_color(ctx, ground->sand);
    for (int y = DRY_SAND_Y - WET_FADE_H; y < DRY_SAND_Y; y++)
    {
        int level = 8 - ((DRY_SAND_Y - y) * 8) / WET_FADE_H;
        for (int x = 0; x < columns; x++)
        {
            if (y > waterline_at(shore, x, base) && dither_keeps(x, y, level))
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
 * @param anim Where the burst has got to. The wobble walks down the glitter twice, so it shimmers.
 */
static void draw_glitter(GContext *ctx, const CommonPalette *pal, GPoint disc, int base, AnimPhase anim)
{
    static const int8_t wobble[] = {0, 2, -1, 1, -2, 0, 1, -1, 2, -2, 1, 0};
    int walk = anim_loop(anim, ARRAY_LENGTH(wobble), 2);

    int waterline = base + shore_offset(disc.x);
    int band = waterline - HORIZON_Y;

    graphics_context_set_stroke_color(ctx, pal->disc);
    graphics_context_set_stroke_width(ctx, 1);

    for (int y = HORIZON_Y + 1; y < waterline; y += 2)
    {
        int down = y - HORIZON_Y;
        int half = 1 + (down * 7) / band;
        int shift = wobble[(unsigned)(down / 2 + walk) % ARRAY_LENGTH(wobble)];

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
 * @param ground The scene's ground row.
 * @param shore The shore curve, one offset per column.
 * @param base The waterline's baseline.
 * @param minute The clock minute, which slides the rows along.
 * @param choppy True to break the ranks up into rougher water.
 * @param anim Where the burst has got to. Every row runs one gap along.
 */
static void draw_waves(GContext *ctx, const ShorelineGround *ground, const int8_t *shore, int base, int minute,
    bool choppy, AnimPhase anim)
{
    graphics_context_set_stroke_color(ctx, ground->foam);
    graphics_context_set_stroke_width(ctx, 1);

    for (int row = 0; row < WAVE_ROWS; row++)
    {
        int dash = 2 + row;        // crests widen as they come in
        int gap = 15 + row * 6;    // and thin out, so the far water reads as further off
        int slide = (minute * (row + 2) + anim_loop(anim, gap, 1)) % gap;

        for (int x = slide - gap; x < SCREEN_W; x += gap)
        {
            // each crest takes its height from the water at its own column, so the whole row
            // bends with the shore instead of cutting across it
            int column = x < 0 ? 0 : (x >= SCREEN_W ? SCREEN_W - 1 : x);
            int y = HORIZON_Y + ((waterline_at(shore, column, base) - HORIZON_Y) * (row + 1)) / (WAVE_ROWS + 1);

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
 * @brief Where the boat sits: its drift across the water and its bob on the swell.
 *
 * The hull and the pennant are drawn at different points in the frame, so they work their
 * position out from here rather than each carrying its own copy of the sum.
 *
 * @param base The tide base.
 * @param level How far in the tide has come, 0 to 100.
 * @param minute The current minute, which is what the bob rides on.
 * @param anim Where the burst has got to. The boat rides the swell up and down twice.
 * @return The boat's anchor point.
 */
static GPoint boat_anchor(int base, int level, int minute, AnimPhase anim)
{
    // the drift spans the screen it is on, so a wider watch sails the whole width rather than
    // stopping where a 200px one used to
    int x = BOAT_MARGIN + (level * (SCREEN_W - BOAT_MARGIN * 2)) / 100;
    int band = base + shore_offset(x) - HORIZON_Y;

    // sat in the near water rather than on the line where the far water gives way to it, so the
    // hull has one shade behind it instead of straddling two
    return GPoint(x, HORIZON_Y + (band * 55) / 100 + ((minute % 3) - 1) + anim_sway(anim, 1, 2, 0));
}

/**
 * @brief Draw the boat out on the water.
 *
 * It rides the tide: in with the flood, back out with the ebb, and it comes about at each turn
 * so it always faces the way it is going. Since the tide eases at the top and bottom of its
 * cycle, the boat slows and turns there too, without any of that being written twice.
 *
 * This is the hull and, on the rectangle, its sail. On the round screen the mast and pennant are
 * drawn separately in draw_pennant, after the weather, so the reading stays legible in the rain.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param base The waterline's baseline.
 * @param level How far in the tide is, 0 to 100.
 * @param rising True while the tide is flooding, which leans the rectangle's sail.
 * @param minute The clock minute, which bobs it a pixel.
 * @param anim Where the burst has got to, which rocks it on the swell.
 */
static void draw_boat(GContext *ctx, const CommonPalette *pal, const ShorelineGround *ground,
    int base, int level, bool rising, int minute, AnimPhase anim)
{
    GPoint at = boat_anchor(base, level, minute, anim);
    int x = at.x, y = at.y;

#if defined(PBL_ROUND)
    (void)rising;  // only the rectangle's sail leans with the tide

    // and a solid hull under it. the profile is held nearly straight down from the deck and only
    // drawn in near the keel: tapering evenly from the widest row makes a triangle, and a triangle
    // floating on water reads as a diamond rather than as a boat
    static const int8_t hull[] = {16, 16, 15, 14, 12, 9, 5};

    graphics_context_set_stroke_color(ctx, pal->ink);
    for (unsigned row = 0; row < ARRAY_LENGTH(hull); row++)
    {
        graphics_draw_line(ctx, GPoint(x - hull[row], y - 4 + (int)row),
                                GPoint(x + hull[row], y - 4 + (int)row));
    }

#else
    // the sail first, filled row by row so it needs no path and no allocation. a right triangle
    // hung off the mast, leaning whichever way the boat is pointed
    int lean = rising ? 1 : -1;
    graphics_context_set_stroke_color(ctx, ground->foam);
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
#endif
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
 * @param ground The scene's ground row.
 * @param shore The shore curve, one offset per column.
 * @param base The waterline's baseline.
 * @param anim Where the burst has got to. The spray walks once along the edge as the surf breaks.
 */
static void draw_surf(GContext *ctx, const ShorelineGround *ground, const int8_t *shore, int base, AnimPhase anim)
{
    // mostly nothing, so the flecks land in clumps rather than at a steady interval
    static const int8_t spray[] = {0, 0, 3, 0, 0, 0, 5, 0, 2, 0, 0, 4, 0, 0, 0, 3, 0, 0, 6, 0, 0, 2, 0};
    int walk = anim_loop(anim, ARRAY_LENGTH(spray), 1);

    graphics_context_set_stroke_color(ctx, ground->foam);
    graphics_context_set_stroke_width(ctx, 1);

    for (int x = 0; x < SCREEN_W; x++)
    {
        int waterline = waterline_at(shore, x, base);
        graphics_draw_line(ctx, GPoint(x, waterline - SURF_H + 1), GPoint(x, waterline));

        int lift = spray[(unsigned)(x + walk) % ARRAY_LENGTH(spray)];
        if (lift)
        {
            graphics_draw_pixel(ctx, GPoint(x, waterline - SURF_H - lift));
        }
    }
}

#if defined(PBL_ROUND)
/**
 * @brief Fly the temperature from the boat's masthead, or a sail when there is no reading.
 *
 * Drawn after the weather rather than with the hull, because the reading is the one thing on the
 * face that has to stay readable in every condition. Painted with the boat it sits behind the
 * rain, and a heavy fall or a haze bank crosses straight over the number.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param ground The scene's ground row.
 * @param base The tide base.
 * @param level How far in the tide has come, 0 to 100.
 * @param rising True while the tide is flooding, which is the way the sail leans.
 * @param minute The current minute.
 * @param anim Where the burst has got to, so the mast rides with the hull.
 */
static void draw_pennant(GContext *ctx, const CommonPalette *pal, const ShorelineGround *ground,
    int base, int level, bool rising, int minute, AnimPhase anim)
{
    GPoint at = boat_anchor(base, level, minute, anim);
    int x = at.x, y = at.y;

    // there is room out here for a proper boat, so it gets one: a taller mast flying a pennant
    // with the temperature on it. that is the reading found somewhere it belongs rather than
    // stacked under the date as a third line of text
    char temp[8];
    readout_weather_temp(temp, sizeof(temp));

    int mast_top = y - BOAT_MAST_H;

    graphics_context_set_stroke_color(ctx, pal->ink);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(x, mast_top), GPoint(x, y - 4));

    // no location and no reading means no pennant, but a mast on its own is a stick rather than
    // a boat, so it carries a sail instead. the rectangle's boat sails the same way
    if (weather_store_temp() == WEATHER_NO_TEMP)
    {
        int lean = rising ? 1 : -1;
        int rows = BOAT_MAST_H - 7;

        graphics_context_set_stroke_color(ctx, ground->foam);
        for (int row = 1; row <= rows; row++)
        {
            int width = (row * 13) / rows;
            graphics_draw_line(ctx, GPoint(x + lean, mast_top + row),
                                    GPoint(x + lean * (1 + width), mast_top + row));
        }

        // the luff back over the fill, so the sail has an edge against the water behind it
        graphics_context_set_stroke_color(ctx, pal->ink);
        graphics_draw_line(ctx, GPoint(x + lean * 14, mast_top + rows), GPoint(x, mast_top));
        return;
    }

    // the pennant sits square over the hull rather than hanging off one side of the mast, which
    // read as a sign held out sideways. drawn after the mast, so the mast disappears behind it
    // and shows only in the gap down to the deck
    // filled with the upper sky, which is the one colour ink is guaranteed to read against: every
    // line in this scene is drawn in ink over that sky, so the pennant inherits the same contrast
    // in every theme. filling with the sea instead leaves the reading nearly invisible wherever a
    // theme pairs dark water with dark linework, which several of them do
    GRect flag = GRect(x - BOAT_FLAG_W / 2, mast_top, BOAT_FLAG_W, BOAT_FLAG_H);
    graphics_context_set_fill_color(ctx, pal->sky_hi);
    graphics_fill_rect(ctx, flag, 0, GCornerNone);
    graphics_draw_rect(ctx, flag);

    // the reading sits centred on the pennant. the hand font's line box is taller than the
    // letters in it, so the height is measured and the difference split rather than guessed
    // the reading only moves when the weather does, so its measurement is kept between repaints
    static MeasureMemo s_flag_memo;
    GFont flag_font = fonts_get(FONT_HAND_16);
    GSize reading = measure_size(&s_flag_memo, temp, flag_font, GRect(0, 0, BOAT_FLAG_W, 40),
        GTextAlignmentCenter);

    graphics_context_set_text_color(ctx, pal->ink);
    graphics_draw_text(ctx, temp, flag_font,
        GRect(flag.origin.x, flag.origin.y + (BOAT_FLAG_H - reading.h) / 2 - 3, flag.size.w, reading.h + 4),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

}
#endif

void shoreline_draw_ground(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;
    const ShorelineGround *ground = frame->ground;

    TideNow tide = tide_now();

    // kept off the stack, since this sits at the bottom of the deepest draw path and the round
    // screen's curve is 260 bytes. it is filled afresh each frame
    static int8_t s_shore[SCREEN_W];
    shore_curve(s_shore);

    // gulls fly nearer than the cloud they pass under, and they keep the daytime sky occupied
    // the way the stars do at night
    if (!frame->night)
    {
        draw_birds(ctx, pal, tide.minutes, frame->anim);
    }

    draw_water(ctx, bounds, pal, ground, s_shore, tide.base);

    // the reflection only exists while there is something up there to cast it
    if (frame->sky.celestial != CEL_HIDDEN && frame->disc.y < HORIZON_Y)
    {
        draw_glitter(ctx, pal, frame->disc, tide.base, frame->anim);
    }

    draw_waves(ctx, ground, s_shore, tide.base, frame->minute, choppy(frame->cond), frame->anim);

    draw_boat(ctx, pal, ground, tide.base, tide.level, tide.rising, frame->minute, frame->anim);

    draw_surf(ctx, ground, s_shore, tide.base, frame->anim);
}

#if defined(PBL_ROUND)
void shoreline_draw_front(GContext *ctx, GRect bounds, const SceneFrame *frame)
{
    TideNow tide = tide_now();

    // the pennant goes in front of the weather. the reading is the one thing on the face that has
    // to stay readable in every condition, and painted with the hull it sits behind the rain
    draw_pennant(ctx, frame->palette, frame->ground, tide.base, tide.level, tide.rising, frame->minute,
        frame->anim);
}
#endif

/** @} */
