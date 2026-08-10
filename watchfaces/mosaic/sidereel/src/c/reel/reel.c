/**
 * @file reel.c
 * @brief The minute reel: six rows drawn around a centre, and an app_timer scroll.
 *
 * The invariant is that s_minute is always the destination and s_offset is how far the reel
 * still has to travel to get there. At a full row of offset the row visually centred is the
 * previous minute, and as the offset decays every row's top edge decreases, so the new minute
 * rises into the centre the way an odometer turns.
 *
 * @ingroup watchface-sidereel
 */
#include "reel.h"

#include "draw/fonts.h"
#include "draw/text.h"
#include "layout.h"
#include "theme/theme.h"
#include "ui/engine/engine.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

// 300ms at roughly 30fps. long enough to read as motion, short enough that a tick can never
// land on top of one in normal running
#define REEL_SCROLL_MS 300
#define REEL_FRAME_MS  33

// fixed-point unit for the easing. 256 keeps the squared term inside an int32 comfortably
#define EASE_ONE 256

static int s_minute;          // the settled centre minute, 0 to 59
static int s_offset;          // pixels still to travel, 0 to REEL_ROW_H, 0 when settled
static AppTimer *s_timer;     // the scroll in flight, or NULL
static uint16_t s_elapsed_ms;

/**
 * @brief Ease-out curve: 1 - (1 - t)^2, in EASE_ONE units.
 *
 * Gives the reel a thrown-and-settling feel rather than a constant slide.
 *
 * @param progress How far through the scroll, 0 to EASE_ONE.
 * @return The eased distance covered, 0 to EASE_ONE.
 */
static int32_t ease_out(int32_t progress)
{
    int32_t remaining = EASE_ONE - progress;

    return EASE_ONE - (remaining * remaining) / EASE_ONE;
}

static void scroll_tick(void *data)
{
    s_timer = NULL;
    s_elapsed_ms = (uint16_t)(s_elapsed_ms + REEL_FRAME_MS);

    if (s_elapsed_ms >= REEL_SCROLL_MS)
    {
        // land exactly on zero rather than wherever the last frame fell, so a settled reel is
        // never a pixel off
        s_offset = 0;
        engine_mark_dirty_tags(TAG_REEL);
        return;
    }

    int32_t progress = ((int32_t)s_elapsed_ms * EASE_ONE) / REEL_SCROLL_MS;
    int32_t moved = ((int32_t)REEL_ROW_H * ease_out(progress)) / EASE_ONE;

    s_offset = REEL_ROW_H - (int)moved;
    engine_mark_dirty_tags(TAG_REEL);

    s_timer = app_timer_register(REEL_FRAME_MS, scroll_tick, NULL);
}

void reel_cancel(void)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }

    s_offset = 0;
    s_elapsed_ms = 0;
}

void reel_set_minute(int minute, bool animate)
{
    bool one_step = (minute == (s_minute + 1) % 60);

    // settle whatever was running first, so two scrolls can never compound their offsets
    reel_cancel();

    s_minute = minute;

    if (!animate || !one_step)
    {
        engine_mark_dirty_tags(TAG_REEL);
        return;
    }

    // the previous minute is still the one centred, and the scroll takes it away
    s_offset = REEL_ROW_H;
    s_timer = app_timer_register(REEL_FRAME_MS, scroll_tick, NULL);

    if (!s_timer)
    {
        s_offset = 0;  // no room for a timer, so just land on the new minute
    }

    engine_mark_dirty_tags(TAG_REEL);
}

void reel_draw(GContext *ctx, GRect bounds, const void *data)
{
    const Chrome *chrome = theme_chrome();

    graphics_context_set_fill_color(ctx, chrome->panel);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // the highlight sits under the digits, and its frame is absolute so shift it into the
    // panel's own coordinates
    GRect highlight = HIGHLIGHT;
    highlight.origin.x = 0;
    graphics_context_set_fill_color(ctx, chrome->highlight);
    graphics_fill_rect(ctx, highlight, 0, GCornerNone);

    graphics_context_set_text_color(ctx, chrome->reel_ink);

    for (int row = REEL_ROW_FIRST; row <= REEL_ROW_LAST; row++)
    {
        // s_minute is 0 to 59 and row bottoms out at -3, so one +60 is enough to stay positive.
        // this is the only place on the face that knows 59 rolls to 00
        int value = (s_minute + row + 60) % 60;

        char digits[4];
        snprintf(digits, sizeof(digits), "%02d", value);

        // the digits sit 2px above the row's true middle: Teko's descender space makes a
        // dead-centred cap look low against the highlight band
        int top = REEL_CENTRE_TOP + REEL_ROW_H * row + s_offset - 2;
        side_draw_centred(ctx, digits, FONT_CLOCK_56, GRect(0, top, bounds.size.w, REEL_ROW_H));
    }
}

/** @} */
