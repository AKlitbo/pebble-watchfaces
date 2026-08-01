/**
 * @file calendar_format.c
 * @brief Formatting shared by the calendar modules. Everything reads the watch's own local
 * clock so the readouts stay right no matter when the phone last synced.
 * @ingroup gridlock_mod_calendar
 */
#include "calendar_format.h"
#include "settings_schema.h"
#include "clock/weekday.h"
#include "draw/metrics.h"
#include "draw/fonts.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DAY_SECONDS (24 * 60 * 60)

// the local midnight at or before t, so two events can be compared by calendar day
static time_t local_midnight(time_t t)
{
    struct tm lt = *localtime(&t);
    lt.tm_hour = 0;
    lt.tm_min = 0;
    lt.tm_sec = 0;
    return mktime(&lt);
}

// whole calendar days from one time to another. the half-day nudge absorbs a DST shift between
// the two midnights so a day never rounds to the wrong count
static int day_diff(time_t from, time_t to)
{
    return (int)((local_midnight(to) - local_midnight(from) + DAY_SECONDS / 2) / DAY_SECONDS);
}

void cal_fmt_time(char *out, size_t n, time_t start, bool all_day)
{
    if (!all_day)
    {
        struct tm lt = *localtime(&start);
        // gridlock_format_clock returns true when it wrote a 12h time. it never adds am/pm, so
        // tack on a compact 'a'/'p' ourselves to keep an agenda time from reading ambiguously
        bool is_12h = gridlock_format_clock(out, n, lt.tm_hour, lt.tm_min);
        if (is_12h)
        {
            size_t len = strlen(out);
            if (len + 1 < n)
            {
                out[len] = (lt.tm_hour < 12) ? 'a' : 'p';
                out[len + 1] = '\0';
            }
        }
        return;
    }

    // an all-day event has no clock, so name the day it lands on instead
    int days = day_diff(time(NULL), start);
    if (days <= 0)
    {
        snprintf(out, n, "Today");
    }
    else if (days == 1)
    {
        snprintf(out, n, "Tomorrow");
    }
    else
    {
        struct tm lt = *localtime(&start);
        strftime(out, n, "%a", &lt); // short weekday, e.g. "Wed"
    }
}

void cal_fmt_day_code(char *out, size_t n, time_t start)
{
    struct tm lt = *localtime(&start);
    snprintf(out, n, "%s", weekday_short(lt.tm_wday));
}

// the two fixed left columns of an agenda row: a two-letter day code, then the clock time (wide
// enough for a 12h time with the am/pm letter, e.g. "12:32p")
#define CAL_DAY_COL 22
#define CAL_TIME_COL 56

void cal_draw_text(GridCtx *gctx, const char *text, FontId font, GColor color, GRect box,
                   GTextAlignment align)
{
    box = metric_baseline(font, box);
    graphics_context_set_text_color(gctx->ctx, color);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), box, GTextOverflowModeTrailingEllipsis,
                       align, NULL);
}

void cal_draw_agenda_row(GridCtx *gctx, const CalendarEvent *event, GRect body, int y)
{
    int day_x = body.origin.x + PANEL_PAD;
    int time_x = day_x + CAL_DAY_COL;
    int title_x = time_x + CAL_TIME_COL;

    // the day code leads the row in grey so a glance places the event on its weekday
    char day_code[4];
    cal_fmt_day_code(day_code, sizeof(day_code), event->start);
    cal_draw_text(gctx, day_code, FONT_STM_14, gctx->color_subtitle,
                  GRect(day_x, y, CAL_DAY_COL, CAL_ROW_H), GTextAlignmentLeft);

    // the clock time next, in the accent colour. an all-day event leaves this blank since its
    // day code already says when it is
    if (!event->all_day)
    {
        char time_buf[12];
        cal_fmt_time(time_buf, sizeof(time_buf), event->start, false);
        cal_draw_text(gctx, time_buf, FONT_STM_14, gctx->color_accent,
                      GRect(time_x, y, CAL_TIME_COL, CAL_ROW_H), GTextAlignmentLeft);
    }

    // the title takes the rest of the row, clipped. an all-day event has no time, so its title
    // shifts left into that slot instead of a gap
    int this_title_x = event->all_day ? time_x : title_x;
    int title_w = body.origin.x + body.size.w - PANEL_PAD - this_title_x;
    cal_draw_text(gctx, event->title, FONT_STM_14, gctx->color_value,
                  GRect(this_title_x, y, title_w, CAL_ROW_H), GTextAlignmentLeft);
}
