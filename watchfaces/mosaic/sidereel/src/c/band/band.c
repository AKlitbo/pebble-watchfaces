/**
 * @file band.c
 * @brief Maps the day onto the screen's outline and paints it.
 *
 * The outline breaks into five straight runs rather than four, because the track opens at the
 * middle of the top edge so midnight sits at twelve o'clock the way it would on a dial. Every
 * run knows where it starts along the perimeter and which way it travels, so painting a stretch
 * of the day is the same overlap sum five times over.
 *
 * @ingroup watchface-sidereel
 */
#include "band.h"

#include "clock/clockstr.h"
#include "clock/timeband.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "layout.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/** @brief One straight stretch of the outline. */
typedef struct
{
    int  base;        /**< Where this run opens along the perimeter */
    int  len;         /**< How far it runs */
    bool horizontal;  /**< Whether it travels sideways rather than up and down */
    int  fixed;       /**< The edge coordinate it sits on */
    int  start;       /**< Where along the travelling axis it opens */
    int  step;        /**< Which way it travels, +1 or -1 */
} BandRun;

// clockwise from top centre. the corners are covered twice over, once by each run that meets
// there, which costs nothing on a fill and is what stops a gap opening at the turn
static const BandRun RUNS[] = {
    {0,                                                  SCREEN_W / 2, true,  0,                     SCREEN_W / 2, 1},
    {SCREEN_W / 2,                                       SCREEN_H,     false, SCREEN_W - BAND_THICK, 0,            1},
    {SCREEN_W / 2 + SCREEN_H,                            SCREEN_W,     true,  SCREEN_H - BAND_THICK, SCREEN_W,    -1},
    {SCREEN_W / 2 + SCREEN_H + SCREEN_W,                 SCREEN_H,     false, 0,                     SCREEN_H,    -1},
    {SCREEN_W / 2 + SCREEN_H + SCREEN_W + SCREEN_H,      SCREEN_W / 2, true,  0,                     0,            1},
};

/**
 * @brief Fill whatever part of a perimeter stretch lands on one run.
 *
 * @param ctx The graphics context.
 * @param run The run to paint on.
 * @param from Where the stretch opens along the perimeter.
 * @param to Where it closes.
 */
static void fill_run(GContext *ctx, const BandRun *run, int from, int to)
{
    int first = from - run->base;
    int last = to - run->base;

    if (first < 0)
    {
        first = 0;
    }

    if (last > run->len)
    {
        last = run->len;
    }

    if (last <= first)
    {
        return;  // none of this stretch is on this run
    }

    // a run travelling backwards hands back its ends the other way round, so take the lower one
    // as the origin rather than trusting the direction of travel
    int head = run->start + run->step * first;
    int tail = run->start + run->step * last;
    int low = head < tail ? head : tail;
    int span = head < tail ? tail - head : head - tail;

    GRect rect = run->horizontal ? GRect(low, run->fixed, span, BAND_THICK)
                                 : GRect(run->fixed, low, BAND_THICK, span);

    graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

/** @brief Fill a stretch of the perimeter in the current fill colour. */
static void fill_span(GContext *ctx, int from, int to)
{
    for (uint8_t index = 0; index < ARRAY_LENGTH(RUNS); index++)
    {
        fill_run(ctx, &RUNS[index], from, to);
    }
}

/**
 * @brief Fill a stretch that may hang off either end of the perimeter.
 *
 * The now marker is centred on its moment, so around midnight half of it belongs at the far end
 * of the track. Splitting it here keeps that from either vanishing or drawing at the wrong place.
 */
static void fill_wrapped(GContext *ctx, int from, int to)
{
    if (from < 0)
    {
        fill_span(ctx, from + BAND_PERIMETER, BAND_PERIMETER);
        from = 0;
    }

    if (to > BAND_PERIMETER)
    {
        fill_span(ctx, 0, to - BAND_PERIMETER);
        to = BAND_PERIMETER;
    }

    fill_span(ctx, from, to);
}

void band_draw(GContext *ctx, GRect bounds, const Chrome *chrome)
{
    TimeBand band = timeband_full_day();

    int rise = clockstr_minutes(weather_store_sunrise());
    int set = clockstr_minutes(weather_store_sunset());
    bool has_sun = rise >= 0 && set >= 0;

    // with no sun reading the honest answer is one flat colour, and dim is what the rest of the
    // face already uses to say it has nothing. painting it all night would claim the sun never rose
    graphics_context_set_fill_color(ctx, has_sun ? chrome->band_night : chrome->highlight);
    fill_span(ctx, 0, BAND_PERIMETER);

    if (has_sun)
    {
        // a window running midnight to midnight sees the daylight in one piece, but the same call
        // covers a stretch that wraps, which is what a polar summer hands back
        TimeBandSpan daylight[2];
        int pieces = timeband_clip_daily(band, rise, set, daylight, 2);

        graphics_context_set_fill_color(ctx, chrome->band_day);

        for (int piece = 0; piece < pieces; piece++)
        {
            int from = timeband_pos_offset(band, BAND_PERIMETER, daylight[piece].from);
            int to = timeband_pos_offset(band, BAND_PERIMETER, daylight[piece].to);

            if (from >= 0 && to >= 0)
            {
                fill_span(ctx, from, to);
            }
        }
    }

    const struct tm *now = time_store_tm();
    int offset = timeband_offset(band, now->tm_hour * 60 + now->tm_min);

    if (offset >= 0)
    {
        int at = timeband_pos_offset(band, BAND_PERIMETER, offset);

        if (at >= 0)
        {
            graphics_context_set_fill_color(ctx, chrome->band_now);
            fill_wrapped(ctx, at - BAND_MARK_HALF, at + BAND_MARK_HALF);
        }
    }

}

/** @} */
