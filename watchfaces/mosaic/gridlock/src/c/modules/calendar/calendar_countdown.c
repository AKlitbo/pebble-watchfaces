/**
 * @file calendar_countdown.c
 * @brief The "Next Event" panel, drawn at two sizes from the next event (index 0) in the store.
 *   2x2  the goal-panel look: the event time up top (big, with a small grey am/pm like the
 *        clock), the title and place below, a date icon bottom-right, and a bar that fills as
 *        the event approaches.
 *   1x4  a single thin line: a grey day code, the accent time, then the title.
 * @ingroup gridlock_mod_calendar
 */
#include "engine/grid_engine.h"
#include "calendar_countdown.h"
#include "calendar_format.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/icons.h"
#include "io/stores/calendar_store.h"
#include "settings_schema.h"

#include <time.h>

// how far ahead the 2x2 countdown bar spans: a full window out the bar reads empty, filling as
// the event nears and topping out right as it starts
#define COUNTDOWN_WINDOW_S (12 * 60 * 60)

// the 1x4 line height (centred in the short body) and the gap between its segments
#define LINE_H 18
#define SEG_GAP 6

// --- 2x2 ---

static void draw_2x2(GridCtx *gctx, const CalendarEvent *event)
{
    // the bar fills as the event approaches: empty a full window out, full at the start
    long remaining = (long)(event->start - time(NULL));
    int progress = 100;
    if (remaining > 0)
    {
        long elapsed = COUNTDOWN_WINDOW_S - remaining;
        if (elapsed < 0)
        {
            elapsed = 0;
        }
        progress = (int)((elapsed * 100) / COUNTDOWN_WINDOW_S);
    }

    if (event->all_day)
    {
        // an all-day event has no clock, so headline the day word (Today / Tomorrow / weekday)
        char day_word[12];
        cal_fmt_time(day_word, sizeof(day_word), event->start, true);
        gh_stat_2x2(gctx, day_word, event->title, event->location, &ICON_DATE_TIME, progress);
    }
    else
    {
        // a timed event headlines the clock time with a small grey am/pm, matching the clock
        struct tm lt = *localtime(&event->start);
        char time_buf[8] = "--:--";
        bool meridiem = gridlock_format_clock(time_buf, sizeof(time_buf), lt.tm_hour, lt.tm_min);
        gh_stat_time_2x2(gctx, time_buf, meridiem, lt.tm_hour < 12,
                         event->title, event->location, &ICON_DATE_TIME, progress);
    }
}

// --- 1x4 ---

// draws left-aligned text: the shared calendar text drawer, left-aligned for the 1x4 line
static void draw_line(GridCtx *gctx, const char *text, FontId font, GColor color, GRect box)
{
    cal_draw_text(gctx, text, font, color, box, GTextAlignmentLeft);
}

// the drawn width of a bit of text, so the next segment can sit right after it
static int text_width(const char *text, FontId font)
{
    return graphics_text_layout_get_content_size(text, fonts_get(font), GRect(0, 0, 1000, LINE_H),
               GTextOverflowModeFill, GTextAlignmentLeft).w;
}

static void draw_1x4(GridCtx *gctx, const CalendarEvent *event)
{
    GRect body = gctx->body;
    int y = body.origin.y + (body.size.h - LINE_H) / 2;
    int x = body.origin.x + PANEL_PAD;
    int right = body.origin.x + body.size.w - PANEL_PAD;

    // the day code leads in grey
    char day_code[4];
    cal_fmt_day_code(day_code, sizeof(day_code), event->start);
    draw_line(gctx, day_code, FONT_STM_14, gctx->color_subtitle, GRect(x, y, right - x, LINE_H));
    x += text_width(day_code, FONT_STM_14) + SEG_GAP;

    // then the clock time in the accent colour, skipped for an all-day event
    if (!event->all_day)
    {
        char time_buf[12];
        cal_fmt_time(time_buf, sizeof(time_buf), event->start, false);
        draw_line(gctx, time_buf, FONT_STM_14, gctx->color_accent, GRect(x, y, right - x, LINE_H));
        x += text_width(time_buf, FONT_STM_14) + SEG_GAP;
    }

    // then the title fills the rest, clipped
    draw_line(gctx, event->title, FONT_STM_14, gctx->color_value, GRect(x, y, right - x, LINE_H));
}

// --- dispatch ---

static void calendar_countdown_body(GridCtx *gctx)
{
    const CalendarEvent *event = calendar_store_event(0);

    // no reading yet: a placeholder that suits each size
    if (!event)
    {
        if (gctx->size == MSIZE_2x2)
        {
            gh_stat_2x2(gctx, "--", "No Events", "", &ICON_DATE_TIME, 0);
        }
        else
        {
            draw_line(gctx, "--", FONT_STM_14, gctx->color_subtitle,
                      GRect(gctx->body.origin.x + PANEL_PAD, gctx->body.origin.y,
                            gctx->body.size.w - PANEL_PAD * 2, gctx->body.size.h));
        }
        return;
    }

    if (gctx->size == MSIZE_2x2)
    {
        draw_2x2(gctx, event);
    }
    else if (gctx->size == MSIZE_1x4)
    {
        draw_1x4(gctx, event);
    }
}

const ModuleDef mod_calendar_countdown_def = {
    .label = "NEXT EVENT",
    .sizes = SZ_2x2 | SZ_1x4,
    .features = FEATURE_CALENDAR,
    .body = calendar_countdown_body
};
