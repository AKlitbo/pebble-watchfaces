/**
 * @file fx.c
 * @brief The cloud, precipitation, wash and lightning draws.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/fx/fx.h"

#include "sketchbook/config.h"

/**
 * @addtogroup family-sketchbook
 * @{
 */

// the overcast band: puffs spaced closer than their own width so they run into each other and
// come out as one lumpy mass rather than a row of separate clouds. the base line rises and falls
// across the sky so the underside is not a ruled edge
//
// a puff reaches (2 * lobe + 11) above its base once the tallest lobe and the ink pass are
// counted, so the face sets bases low enough to keep the tops clear of the status bar and high
// enough to keep the whole band off its own ground
static const Puff s_overcast[] = SKETCHBOOK_OVERCAST_PUFFS;

/**
 * @brief Draw a group of puffs as one cloud.
 *
 * Every puff is laid down twice: once a couple of pixels fatter in the ink colour across the
 * whole group, then once at size in the cloud colour. Outlining the group rather than each
 * puff is what merges overlapping puffs into a single fluffy silhouette. Outline each puff on
 * its own and every seam shows straight through the middle.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param puffs The puffs making up the cloud.
 * @param count How many puffs there are.
 */
static void draw_puffs(GContext *ctx, const Palette *pal, const Puff *puffs, int count)
{
    for (int pass = 0; pass < 2; pass++)
    {
        int grow = pass == 0 ? 2 : 0;
        graphics_context_set_fill_color(ctx, pass == 0 ? pal->ink : pal->cloud);

        for (int i = 0; i < count; i++)
        {
            int x = puffs[i].x;
            int y = puffs[i].y;
            int half_w = puffs[i].half_w;
            int lobe = puffs[i].lobe;

            // five bumps at four different heights, tallest just off-centre, so the top edge
            // reads as fluff instead of a row of matching humps
            graphics_fill_circle(ctx, GPoint(x - half_w + lobe, y - lobe + 2), lobe + grow);
            graphics_fill_circle(ctx, GPoint(x - half_w / 2, y - lobe - 3), lobe + 1 + grow);
            graphics_fill_circle(ctx, GPoint(x, y - lobe - 6), lobe + 3 + grow);
            graphics_fill_circle(ctx, GPoint(x + half_w / 2, y - lobe - 2), lobe + grow);
            graphics_fill_circle(ctx, GPoint(x + half_w - lobe, y - lobe + 1), lobe - 1 + grow);
            graphics_fill_rect(ctx, GRect(x - half_w - grow, y - lobe, half_w * 2 + grow * 2, lobe + grow), 0, GCornerNone);
        }
    }
}

void sketchbook_fx_draw_clouds(GContext *ctx, const Palette *pal, const SketchbookSky *sky, GPoint disc)
{
    if (sky->clouds == CLOUDS_NONE)
    {
        return;
    }

    if (sky->clouds == CLOUDS_OVERCAST)
    {
        draw_puffs(ctx, pal, s_overcast, ARRAY_LENGTH(s_overcast));
        return;
    }

    // a single drifting cloud, parked on the disc so a peeking sun always reads as
    // half-covered, and kept clear of the edges wherever the disc has got to. two puffs so
    // even the lone cloud has some fluff to it
    int x = disc.x + 14;
    if (x < 52)
    {
        x = 52;
    }
    if (x > 146)
    {
        x = 146;
    }

    // the low end of the clamp is what keeps the taller lobe out from under the status bar when
    // the disc is high; the high end keeps the cloud off the face's ground when it is low
    int y = disc.y + SKETCHBOOK_CLOUD_DROP;
    if (y < SKETCHBOOK_CLOUD_Y_MIN)
    {
        y = SKETCHBOOK_CLOUD_Y_MIN;
    }
    if (y > SKETCHBOOK_CLOUD_Y_MAX)
    {
        y = SKETCHBOOK_CLOUD_Y_MAX;
    }

    const Puff drifting[] = {
        {x - 16, y, 18, SKETCHBOOK_DRIFT_LOBE_A},
        {x + 14, y - 4, 20, SKETCHBOOK_DRIFT_LOBE_B},
    };
    draw_puffs(ctx, pal, drifting, ARRAY_LENGTH(drifting));
}

/**
 * @brief Draw one falling particle.
 *
 * @param ctx The graphics context.
 * @param kind A PrecipKind.
 * @param at Where the particle sits.
 * @param index The particle's number, which picks the half of a mixed fall.
 */
static void draw_particle(GContext *ctx, uint8_t kind, GPoint at, int index)
{
    if (kind == PRECIP_MIXED)
    {
        kind = (index & 1) ? PRECIP_DROPS : PRECIP_DASHES;
    }

    if (kind == PRECIP_DASHES)
    {
        graphics_draw_line(ctx, at, GPoint(at.x - 2, at.y + 6));
        return;
    }

    if (kind == PRECIP_FLAKES)
    {
        graphics_draw_line(ctx, GPoint(at.x - 2, at.y), GPoint(at.x + 2, at.y));
        graphics_draw_line(ctx, GPoint(at.x, at.y - 2), GPoint(at.x, at.y + 2));
        return;
    }

    graphics_fill_circle(ctx, at, 1);
}

void sketchbook_fx_draw_precip(GContext *ctx, const Palette *pal, const SketchbookSky *sky, int minute)
{
    if (sky->precip == PRECIP_NONE)
    {
        return;
    }

    graphics_context_set_stroke_color(ctx, pal->precip);
    graphics_context_set_fill_color(ctx, pal->precip);

    // only odd stroke widths render, so rain gets the fat one and everything finer stays at 1
    graphics_context_set_stroke_width(ctx, sky->precip == PRECIP_DASHES ? 3 : 1);

    // the two strides are coprime with the span, so the particles spread out instead of
    // lining up, and the minute slides the whole field along. the band stops short of the
    // clock's cap line, so the digits never have rain drawn through them
    //
    // the span follows the screen, and the count with it: the table's numbers are tuned for the
    // rectangle, and spreading that many drops over a wider screen turns a downpour into drizzle
    int span = PBL_DISPLAY_WIDTH - 12;
    int drops = (sky->drops * PBL_DISPLAY_WIDTH) / 200;

    for (int i = 0; i < drops; i++)
    {
        int x = 6 + (i * 41 + minute * 13) % span;
        int y = SKETCHBOOK_PRECIP_TOP + (i * 27 + minute * 19) % SKETCHBOOK_PRECIP_H;
        draw_particle(ctx, sky->precip, GPoint(x, y), i);
    }

    graphics_context_set_stroke_width(ctx, 1);
}

void sketchbook_fx_draw_wash(GContext *ctx, const Palette *pal)
{
    // the watch has no alpha, so the wash is stippled instead: rows of cloud-coloured dots over
    // the ground, packed tight low down and thinning as they rise. drawn as a wash rather than
    // as banks because solid lines read as bars ruled across the scene
    graphics_context_set_stroke_color(ctx, pal->cloud);

    for (int y = SKETCHBOOK_WASH_TOP; y < SKETCHBOOK_WASH_BOTTOM; y += 2)
    {
        int step = 9 - (y - SKETCHBOOK_WASH_TOP) / 6;
        if (step < 2)
        {
            step = 2;
        }

        // the row's start shifts with y so the dots never stack into vertical stripes
        for (int x = (y / 2) % step; x < PBL_DISPLAY_WIDTH; x += step)
        {
            graphics_draw_pixel(ctx, GPoint(x, y));
        }
    }

    // a couple of solid wisps through the densest part, which gives the wash an edge to read
    // against and stops it looking like flat noise
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(18, SKETCHBOOK_WASH_WISP_HI_Y), GPoint(74, SKETCHBOOK_WASH_WISP_HI_Y));
    graphics_draw_line(ctx, GPoint(96, SKETCHBOOK_WASH_WISP_LO_Y), GPoint(158, SKETCHBOOK_WASH_WISP_LO_Y));
    graphics_context_set_stroke_width(ctx, 1);
}

void sketchbook_fx_draw_bolt(GContext *ctx, const Palette *pal)
{
    // a zigzag out of the cloud base, drawn as lines rather than a path so it needs no
    // allocation and keeps the same hand-drawn weight as the rest of the linework
    static const GPoint stroke[][2] = SKETCHBOOK_BOLT_STROKE;

    // laid down fat in ink first, then narrower in the disc colour, so the fork keeps an
    // outline and stays legible against a pale sky and through the rain in front of it
    for (int pass = 0; pass < 2; pass++)
    {
        graphics_context_set_stroke_color(ctx, pass == 0 ? pal->ink : pal->disc);
        graphics_context_set_stroke_width(ctx, pass == 0 ? 7 : 3);

        for (unsigned i = 0; i < ARRAY_LENGTH(stroke); i++)
        {
            graphics_draw_line(ctx, stroke[i][0], stroke[i][1]);
        }
    }

    graphics_context_set_stroke_width(ctx, 1);
}

/** @} */
