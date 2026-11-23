/**
 * @file fx.c
 * @brief The cloud, precipitation, wash and lightning draws.
 *
 * @ingroup watchface-sketchbook
 */
#include "fx/fx.h"

#include "scene/geometry.h"
#include "theme/palette.h"
#include "math/scale.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// How far a cloud's ink outline reaches past its cloud colour on every side
#define PUFF_INK 2

/// How far under a cloud's base the rain or snow starts. The bumps and end lobes of the biggest
/// puffs sag 5px under the base, so this leaves a clear gap under the outline
#define FALL_GAP 7

/// How far up inside a cloud's base a bolt starts, so it reads as coming out of the cloud
#define BOLT_TUCK 1

/// The fewest drops the column under a drifting cloud gets, so drizzle reads as a fall rather than
/// a few stray specks
#define COLUMN_DROPS_MIN 12

// a puff's lobes, in eighths. across is of the puff's half width, up and the radius are of its
// lobe radius. seven of them, each its own size and height and tallest just off-centre, so the top
// edge reads as fluff rather than a row of matching humps. odd puffs take the row mirrored, so
// neighbours in the band never repeat each other
static const int8_t s_lobes[][3] = {
    {-7, 4, 6}, {-4, 9, 8}, {-1, 13, 10}, {2, 12, 9}, {4, 8, 8}, {6, 6, 7}, {8, 3, 5},
};

/// The circles a puff is built from: its lobes along the top, then two bumps under its base
#define PUFF_CIRCLES (ARRAY_LENGTH(s_lobes) + 2)

// the rows of shading along a puff's underside, as how far up from the base and how far apart the
// dots sit. thick at the base and thinning as it rises
static const int8_t s_shade_rows[][2] = {{1, 2}, {2, 3}, {4, 3}, {6, 5}};

/**
 * @brief A rough brightness for a 64-colour GColor, weighting green highest the way the eye does.
 *
 * @param c The colour.
 * @return Its brightness, from 0 for black up to 18 for white.
 */
static int tone_of(GColor c)
{
    return c.r * 2 + c.g * 3 + c.b;
}

/**
 * @brief The palette colour that shades a cloud's underside: whichever of ink, sky_lo and dim sits
 * furthest below the cloud in brightness.
 *
 * No one field is darker than the cloud on every palette. Ink is dark by day and light by night,
 * and sky_lo is the cloud's own colour on Sketchbook by day and Blueprint by night. Where none is
 * darker it falls back to ink, which at least always stands off the cloud.
 *
 * @param pal The palette in use.
 * @return The colour to stipple the underside with.
 */
static GColor shade_tone(const CommonPalette *pal)
{
    const GColor picks[] = {pal->ink, pal->sky_lo, pal->dim};
    GColor best = pal->ink;
    int best_gap = 0;

    for (unsigned i = 0; i < ARRAY_LENGTH(picks); i++)
    {
        int gap = tone_of(pal->cloud) - tone_of(picks[i]);

        if (gap > best_gap)
        {
            best = picks[i];
            best_gap = gap;
        }
    }

    return best;
}

/**
 * @brief Which way a puff's lobes run. Odd puffs take the row mirrored, so neighbours in a cloud
 * never repeat each other.
 *
 * @param i The puff's place in its cloud.
 * @return 1 for the row as written, -1 for it mirrored.
 */
static int puff_mirror(int i)
{
    return (i & 1) ? -1 : 1;
}

/**
 * @brief One of the circles a puff is built from, at the puff's own size with no outline.
 *
 * The lobes come first, then two small bumps that sag a couple of pixels under the base, so the
 * bottom edge is not ruled straight. The draw and the checks on where a puff reaches both lay
 * their circles out here, so they always agree.
 *
 * @param p The puff.
 * @param mirror 1, or -1 to run the lobes the other way.
 * @param k Which circle, from 0 up to PUFF_CIRCLES.
 * @param centre Where the circle's centre goes.
 * @return The circle's radius.
 */
static int puff_circle(const Puff *p, int mirror, unsigned k, GPoint *centre)
{
    if (k < ARRAY_LENGTH(s_lobes))
    {
        *centre = GPoint(p->x + mirror * (s_lobes[k][0] * p->half_w) / 8, p->y - (s_lobes[k][1] * p->lobe) / 8);
        return (s_lobes[k][2] * p->lobe) / 8;
    }

    int bump = p->lobe / 3 + 1;

    if (k == ARRAY_LENGTH(s_lobes))
    {
        *centre = GPoint(p->x - mirror * p->half_w / 2, p->y + 1 - bump);
    }
    else
    {
        *centre = GPoint(p->x + mirror * p->half_w / 3, p->y + 2 - bump);
    }

    return bump;
}

/**
 * @brief The lowest row a puff paints in a column, outline and all, or -1 where it paints nothing.
 *
 * It walks the same circles and base the draw lays down, at their outlined size. The end lobes
 * reach well past the puff's half width. A circle's smoothed edge reaches about half a pixel past
 * its radius, so a column counts as painted that far out too.
 *
 * @param puffs The cloud's puffs.
 * @param i Which of them.
 * @param x The column.
 * @return The lowest painted row, or -1.
 */
static int puff_bottom_at(const Puff *puffs, int i, int x)
{
    const Puff *p = &puffs[i];
    int bottom = -1;

    if (x >= p->x - p->half_w - PUFF_INK && x < p->x + p->half_w + PUFF_INK)
    {
        bottom = p->y + PUFF_INK - 1;
    }

    for (unsigned k = 0; k < PUFF_CIRCLES; k++)
    {
        GPoint centre;
        int r = puff_circle(p, puff_mirror(i), k, &centre) + PUFF_INK;
        int dx = x - centre.x;

        // r and a half, squared, kept in whole numbers
        int reach = r * r + r;

        if (dx * dx > reach)
        {
            continue;
        }

        int dy = r;

        while (dx * dx + dy * dy > reach)
        {
            dy--;
        }

        if (centre.y + dy > bottom)
        {
            bottom = centre.y + dy;
        }
    }

    return bottom;
}

/**
 * @brief Whether no other puff paints lower in this column, so the shading here sits on the
 * cloud's real underside.
 *
 * A neighbour reaching lower covers this puff's underside, and shading under it would float in the
 * middle of the cloud.
 *
 * @param puffs The cloud's puffs.
 * @param count How many there are.
 * @param i The puff being shaded.
 * @param x The column.
 * @return True when this puff's underside is the cloud's here.
 */
static bool lowest_at(const Puff *puffs, int count, int i, int x)
{
    int own = puff_bottom_at(puffs, i, x);

    for (int j = 0; j < count; j++)
    {
        if (j != i && puff_bottom_at(puffs, j, x) > own)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Draw a group of puffs as one cloud, with a lumpy base and its underside shaded.
 *
 * Every puff is laid down twice: once a couple of pixels fatter in the ink colour across the
 * whole group, then once at size in the cloud colour. Outlining the group rather than each
 * puff is what merges overlapping puffs into a single fluffy silhouette. Outline each puff on
 * its own and every seam shows straight through the middle.
 *
 * The shading is a stipple in the darkest tone the palette offers, since the watch has no alpha to
 * darken the cloud with.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param puffs The puffs making up the cloud.
 * @param count How many puffs there are.
 */
static void draw_puffs(GContext *ctx, const CommonPalette *pal, const Puff *puffs, int count)
{
    for (int pass = 0; pass < 2; pass++)
    {
        int grow = pass == 0 ? PUFF_INK : 0;
        graphics_context_set_fill_color(ctx, pass == 0 ? pal->ink : pal->cloud);

        for (int i = 0; i < count; i++)
        {
            const Puff *p = &puffs[i];

            for (unsigned k = 0; k < PUFF_CIRCLES; k++)
            {
                // the flat base fills in under the lobes, then the bumps sag a couple of pixels
                // under it so the bottom edge is not ruled straight
                if (k == ARRAY_LENGTH(s_lobes))
                {
                    GRect base = GRect(p->x - p->half_w - grow, p->y - p->lobe, p->half_w * 2 + grow * 2, p->lobe + grow);
                    graphics_fill_rect(ctx, base, 0, GCornerNone);
                }

                GPoint centre;
                int r = puff_circle(p, puff_mirror(i), k, &centre);
                graphics_fill_circle(ctx, centre, r + grow);
            }
        }
    }

    graphics_context_set_stroke_color(ctx, shade_tone(pal));

    for (int i = 0; i < count; i++)
    {
        int x = puffs[i].x;
        int y = puffs[i].y;
        int half_w = puffs[i].half_w;

        for (unsigned row = 0; row < ARRAY_LENGTH(s_shade_rows); row++)
        {
            int ry = y - s_shade_rows[row][0];
            int step = s_shade_rows[row][1];
            int inset = 2 + row * 3;

            // the row's start shifts with its height so the dots never stack into upright stripes
            for (int px = x - half_w + inset + (ry * 2) % step; px <= x + half_w - inset; px += step)
            {
                if (lowest_at(puffs, count, i, px))
                {
                    graphics_draw_pixel(ctx, GPoint(px, ry));
                }
            }
        }
    }
}

/**
 * @brief The two puffs of the lone drifting cloud, parked on the disc.
 *
 * @param frame This frame's reading.
 * @param out Where the two puffs go.
 */
static void drifting_puffs(const SceneFrame *frame, Puff out[2])
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    GPoint disc = frame->disc;

    // a single drifting cloud, parked on the disc so a peeking sun always reads as
    // half-covered, and kept clear of the edges wherever the disc has got to. two puffs so
    // even the lone cloud has some fluff to it
    // the right end follows the screen width
    // a round screen pulls both ends in, since the glass curves in over a cloud parked low at
    // either side. the right end comes in further, since the far puff is the bigger and higher of
    // the two
    int x = clamp_int(disc.x + 14, PBL_IF_ROUND_ELSE(55, 52), PBL_DISPLAY_WIDTH - PBL_IF_ROUND_ELSE(62, 54));

    // the low end of the clamp keeps the taller lobe out from under the status bar when the disc
    // is high
    // the high end keeps the cloud off the scene's ground when it is low
    int y = clamp_int(disc.y + g->cloud_drop, g->cloud_y_min, g->cloud_y_max);

    out[0] = (Puff){x - 16, y, 18, g->drift_lobe_a};
    out[1] = (Puff){x + 14, y - 4, 20, g->drift_lobe_b};
}

void sketchbook_fx_draw_clouds(GContext *ctx, const SceneFrame *frame)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    const CommonPalette *pal = frame->palette;
    const SketchbookSky *sky = &frame->sky;

    if (sky->clouds == CLOUDS_NONE)
    {
        return;
    }

    // the overcast band is the scene's own: puffs spaced closer than their own width so they run
    // into each other and come out as one lumpy mass rather than a row of separate clouds, with
    // bases set low enough to keep the tops clear of the status bar and high enough to keep the
    // whole band off the scene's ground
    if (sky->clouds == CLOUDS_OVERCAST)
    {
        draw_puffs(ctx, pal, g->overcast, g->overcast_count);
        return;
    }

    Puff drifting[2];
    drifting_puffs(frame, drifting);
    draw_puffs(ctx, pal, drifting, ARRAY_LENGTH(drifting));
}

/**
 * @brief Where the cloud's outline starts under a column, or -1 where no cloud hangs over it.
 *
 * Under most of a puff that is its flat base. Out at its end lobes the outline curves up above
 * the base, so there it is the top of the outline under the lobe. Where puffs overlap the lowest
 * one wins.
 *
 * @param puffs The cloud's puffs.
 * @param count How many there are.
 * @param x The column.
 * @return The row the outline starts on, or -1.
 */
static int cloud_base_at(const Puff *puffs, int count, int x)
{
    int base = -1;

    for (int i = 0; i < count; i++)
    {
        int bottom = puff_bottom_at(puffs, i, x);

        if (bottom < 0)
        {
            continue;
        }

        // the outline is PUFF_INK rows deep under the flat base, and about that under a lobe too
        int start = bottom - PUFF_INK + 1;

        if (start > puffs[i].y)
        {
            start = puffs[i].y;
        }

        if (start > base)
        {
            base = start;
        }
    }

    return base;
}

/**
 * @brief Draw one falling particle as a thin 1px mark.
 *
 * Rain is a slanted streak in two lengths, with a shorter twin beside every fourth long one. Snow
 * is a small plus, a small cross, or a single flake. Drizzle is a 2px tick.
 *
 * @param ctx The graphics context.
 * @param kind A PrecipKind.
 * @param at Where the particle starts.
 * @param index The particle's number. It picks the half of a mixed fall, then the shape within
 * its own kind.
 */
static void draw_particle(GContext *ctx, uint8_t kind, GPoint at, int index)
{
    // a mixed fall takes turns between its two kinds, so each kind counts only its own particles
    // counting all of them would land every streak on an even number and never draw a twin
    if (kind == PRECIP_MIXED)
    {
        kind = (index & 1) ? PRECIP_DROPS : PRECIP_DASHES;
        index /= 2;
    }

    int x = at.x;
    int y = at.y;

    if (kind == PRECIP_DASHES)
    {
        if (index % 3 == 0)
        {
            graphics_draw_line(ctx, at, GPoint(x - 2, y + 6));
            return;
        }

        graphics_draw_line(ctx, at, GPoint(x - 3, y + 10));

        if (index % 4 == 1)
        {
            graphics_draw_line(ctx, GPoint(x + 4, y + 3), GPoint(x + 2, y + 9));
        }
        return;
    }

    if (kind == PRECIP_FLAKES)
    {
        graphics_draw_pixel(ctx, at);

        if (index % 3 == 0)
        {
            graphics_draw_pixel(ctx, GPoint(x - 1, y));
            graphics_draw_pixel(ctx, GPoint(x + 1, y));
            graphics_draw_pixel(ctx, GPoint(x, y - 1));
            graphics_draw_pixel(ctx, GPoint(x, y + 1));
        }
        else if (index % 3 == 1)
        {
            graphics_draw_pixel(ctx, GPoint(x - 1, y - 1));
            graphics_draw_pixel(ctx, GPoint(x + 1, y - 1));
            graphics_draw_pixel(ctx, GPoint(x - 1, y + 1));
            graphics_draw_pixel(ctx, GPoint(x + 1, y + 1));
        }
        return;
    }

    // a single pixel vanishes on the strongly coloured skies, so drizzle is two tall
    graphics_draw_pixel(ctx, at);
    graphics_draw_pixel(ctx, GPoint(x, y + 1));
}

/**
 * @brief A well-stirred number from a particle's index and the minute.
 *
 * Any fixed stride over evenly spaced rows lines the particles up into a slanted grid, worst of all
 * in a narrow column, so the column under a drifting cloud picks its spots from this instead.
 *
 * @param i The particle's number.
 * @param minute The minute of the day.
 * @return A number whose bits all shift when either input moves by one.
 */
static uint32_t scatter(int i, int minute)
{
    uint32_t mix = (uint32_t)(i + 1) * 0x9E3779B1u ^ (uint32_t)(minute + 1) * 0x85EBCA6Bu;
    mix ^= mix >> 15;
    mix *= 0x2C1B3C6Du;
    mix ^= mix >> 12;
    return mix;
}

/**
 * @brief The first stride from a starting guess up that shares no factor with the span it wraps
 * over.
 *
 * A stride that shares a factor with its span only ever lands on some of the span's rows or
 * columns. A stride of 27 over a 90 row band shares 9 with it, so it reaches only 10 rows and stacks
 * the whole fall onto those.
 *
 * @param stride The stride to start from.
 * @param span How far it wraps.
 * @return The stride, stepped up until it shares no factor with the span.
 */
static int coprime_stride(int stride, int span)
{
    for (;;)
    {
        int a = stride;
        int b = span;

        while (b != 0)
        {
            int rest = a % b;
            a = b;
            b = rest;
        }

        if (a == 1)
        {
            return stride;
        }

        stride++;
    }
}

/**
 * @brief Draw the falling precipitation as a curtain under the cloud.
 *
 * The fall starts under the cloud's base at each column and runs down to the clock's cap line. The
 * overcast band rains right across the sky, and the lone drifting cloud only in a column under
 * itself.
 *
 * Particle positions come from the minute rather than a random number, so the scene shifts
 * once a minute on the tick that is already happening and never costs a timer.
 *
 * @param ctx The graphics context.
 * @param frame This frame's reading.
 */
static void draw_precip(GContext *ctx, const SceneFrame *frame)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    const SketchbookSky *sky = &frame->sky;
    int minute = frame->minute;

    if (sky->precip == PRECIP_NONE)
    {
        return;
    }

    graphics_context_set_stroke_color(ctx, frame->palette->precip);
    graphics_context_set_stroke_width(ctx, 1);

    // the span follows the screen, and the count with it: the table's numbers are tuned for the
    // rectangle, and spreading that many over a wider screen turns a downpour into drizzle. fine
    // marks need half as many again to carry the same weight
    int span = PBL_DISPLAY_WIDTH - 12;
    int drops = (sky->drops * PBL_DISPLAY_WIDTH) / 200 * 3 / 2;

    // the long streak reaches 10px down, so the band stops 4px short of the scene's own bottom to
    // keep the digits clear of rain
    int top = g->precip_top;
    int bottom = top + g->precip_h - 4;

    const Puff *cloud = g->overcast;
    int cloud_count = sky->clouds == CLOUDS_NONE ? 0 : g->overcast_count;
    bool column = sky->clouds == CLOUDS_DRIFTING;
    Puff drifting[2];
    int col_left = 0;
    int col_w = 1;

    if (column)
    {
        drifting_puffs(frame, drifting);
        cloud = drifting;
        cloud_count = ARRAY_LENGTH(drifting);
        col_left = drifting[0].x - drifting[0].half_w + 2;
        col_w = (drifting[1].x + drifting[1].half_w - 2) - col_left;

        // the column takes its share of the count by its width, then 11/8 of that share
        // the share alone is too thin to read as a shower in so narrow a column
        // a shower's 20 drops times 11/8 still comes in just under steady rain's 28, so the column
        // never falls heavier than rain across the whole sky
        drops = clamp_int(drops * col_w * 11 / (span * 8), COLUMN_DROPS_MIN, drops);
    }

    // each stride shares no factor with the span it wraps over, so the particles spread across
    // every column and row rather than stacking onto a few. the minute slides the whole field along
    int band = bottom - top;
    int x_stride = coprime_stride(41, span);
    int y_stride = coprime_stride(27, band);

    for (int i = 0; i < drops; i++)
    {
        uint32_t mix = scatter(i, minute);
        int x = column ? col_left + (int)((mix >> 16) % (uint32_t)col_w) : 6 + (i * x_stride + minute * 13) % span;
        int base = cloud_base_at(cloud, cloud_count, x);
        int from = base < 0 ? top : base + FALL_GAP;

        if (from >= bottom)
        {
            continue;
        }

        int y;

        // a column holds only a handful of drops, so each takes its own row down the column's
        // height, nudged a few pixels either way. a scatter bunches them at the bottom and leaves
        // the air under the cloud empty
        if (column)
        {
            y = from + (i * (bottom - from)) / drops + (int)((mix >> 8) % 7) - 3;
            y = clamp_int(y, from, bottom - 1);
        }
        else
        {
            // the stride runs over the whole band and is then squeezed in under the cloud
            // a stride over just the depth under the cloud lines up with itself at some depths
            // and stacks a run of streaks into a ladder
            y = from + ((i * y_stride + minute * 19) % band) * (bottom - from) / band;
        }

        draw_particle(ctx, sky->precip, GPoint(x, y), i);
    }
}

/**
 * @brief Draw the wash lying over the ground: the scene's fog, haze or mist.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_wash(GContext *ctx, const CommonPalette *pal)
{
    const SceneGeometry *g = sketchbook_scene_geometry();

    // the watch has no alpha, so the wash is stippled instead: rows of cloud-coloured dots over
    // the ground, packed tight low down and thinning as they rise. drawn as a wash rather than
    // as banks because solid lines read as bars ruled across the scene
    graphics_context_set_stroke_color(ctx, pal->cloud);

    for (int y = g->wash_top; y < g->wash_bottom; y += 2)
    {
        int step = clamp_int(9 - (y - g->wash_top) / 6, 2, 9);

        // the row's start shifts with y so the dots never stack into vertical stripes
        for (int x = (y / 2) % step; x < PBL_DISPLAY_WIDTH; x += step)
        {
            graphics_draw_pixel(ctx, GPoint(x, y));
        }
    }

    // a couple of solid wisps through the densest part, which gives the wash an edge to read
    // against and stops it looking like flat noise
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(18, g->wash_wisp_hi_y), GPoint(74, g->wash_wisp_hi_y));
    graphics_draw_line(ctx, GPoint(96, g->wash_wisp_lo_y), GPoint(158, g->wash_wisp_lo_y));
    graphics_context_set_stroke_width(ctx, 1);
}

// a bolt's zigzag, in 64ths of its height, from where it leaves the cloud down to its tip. it
// leans left, and a spot with a flip of -1 mirrors it to lean right
static const int8_t s_zigzag[4][2] = {{0, 0}, {-19, 29}, {-2, 27}, {-29, 64}};

/**
 * @brief Lay a bolt's zigzag out on its spot.
 *
 * The bolt starts a pixel up inside the overcast band's base at its column, so it reads as coming
 * out of the cloud wherever the band's base steps up or down.
 *
 * @param g The scene's geometry, whose overcast band the bolt leaves.
 * @param spot Where the bolt strikes.
 * @param out Where each corner of the zigzag goes, from the cloud down to the tip.
 */
static void bolt_corners(const SceneGeometry *g, const BoltSpot *spot, GPoint out[ARRAY_LENGTH(s_zigzag)])
{
    int base = cloud_base_at(g->overcast, g->overcast_count, spot->x);
    int y0 = (base < 0 ? g->precip_top : base) - BOLT_TUCK;

    for (unsigned k = 0; k < ARRAY_LENGTH(s_zigzag); k++)
    {
        out[k] = GPoint(spot->x + spot->flip * (s_zigzag[k][0] * spot->h) / 64, y0 + (s_zigzag[k][1] * spot->h) / 64);
    }
}

/**
 * @brief Draw the storm's bolts out of the cloud, one zigzag on each of the scene's spots.
 *
 * The first spot is the main bolt, drawn at full weight, and the rest are drawn thinner. Every
 * bolt is laid down fat in ink first, then narrower in the disc colour, so it keeps an outline and
 * stays legible against a pale sky and through the rain. All the outlines go down before any core,
 * so where two bolts cross their cores run into each other the way the puffs of a cloud do. They
 * are drawn as lines rather than a path, so they need no allocation.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
static void draw_bolts(GContext *ctx, const CommonPalette *pal)
{
    const SceneGeometry *g = sketchbook_scene_geometry();
    const BoltSpots *bolts = sketchbook_scene_bolts();

    // each bolt is laid out once and drawn twice, outline then core
    // the clamp only guards the array, since SCENE_BOLTS stops the build on a longer table
    unsigned count = bolts->count < SCENE_BOLTS_MAX ? bolts->count : SCENE_BOLTS_MAX;
    GPoint corners[SCENE_BOLTS_MAX][ARRAY_LENGTH(s_zigzag)];

    for (unsigned s = 0; s < count; s++)
    {
        bolt_corners(g, &bolts->spots[s], corners[s]);
    }

    for (int pass = 0; pass < 2; pass++)
    {
        graphics_context_set_stroke_color(ctx, pass == 0 ? pal->ink : pal->disc);

        for (unsigned s = 0; s < count; s++)
        {
            // only odd stroke widths render, so the thinner bolts step down by two
            int width = (pass == 0 ? 7 : 3) - (s == 0 ? 0 : 2);
            graphics_context_set_stroke_width(ctx, width);

            for (unsigned k = 0; k < ARRAY_LENGTH(s_zigzag) - 1; k++)
            {
                graphics_draw_line(ctx, corners[s][k], corners[s][k + 1]);
            }
        }
    }

    graphics_context_set_stroke_width(ctx, 1);
}

void sketchbook_fx_draw_weather(GContext *ctx, const SceneFrame *frame)
{
    const CommonPalette *pal = frame->palette;

    // rain falls in front of the scene's ground, since it is the nearest thing to you
    // only a round screen's temperature holder goes over it
    draw_precip(ctx, frame);

    if (frame->sky.wash)
    {
        draw_wash(ctx, pal);
    }

    if (frame->sky.bolt)
    {
        draw_bolts(ctx, pal);
    }
}

/** @} */
