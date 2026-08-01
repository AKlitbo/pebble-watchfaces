/**
 * @file calendar_freebusy.c
 * @brief The Timeline panel, drawn at three sizes around one free/busy strip that buckets the next
 * few hours into cells (accent-filled where an event overlaps, shaded at its real offset in the
 * hour). This is the one family that reads an event's end time, not just its start.
 *   1x4  the strip with the next event named underneath.
 *   2x4  the Agenda list with the eight-hour strip on top.
 *   2x2  the next event's time and title with a short four-hour strip below.
 * @ingroup gridlock_mod_calendar
 */
#include "engine/grid_engine.h"
#include "calendar_freebusy.h"
#include "calendar_format.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include "io/stores/calendar_store.h"
#include "settings_schema.h"

#include <stdio.h>
#include <time.h>

// the full strip spans eight hours, the short 2x2 strip four. the strip is BAR_H tall
#define FREEBUSY_HOURS 8
#define NEXT_HOURS 4
#define BAR_H 10

// gap between the day code, time, and title as they flow across the 1x4 caption
#define CAPTION_GAP 5

// pixel width of a 1x4 caption segment, so the day code, time, and title flow left to right
static int caption_seg_width(const char *text)
{
    return graphics_text_layout_get_content_size(text, fonts_get(FONT_STM_12),
               GRect(0, 0, 1000, 14), GTextOverflowModeFill, GTextAlignmentLeft).w;
}

/**
 * @brief Draws the free/busy strip: `hours` hourly cells across `area`, each outlined, with any busy
 * time shaded at its real offset in the hour. All-day events are skipped since they would blanket
 * every hour and leave the strip solid and meaningless.
 *
 * @param gctx The grid context (for the colours and the graphics context).
 * @param area The rect to lay the cells across.
 * @param hours How many hours to span, starting at the top of the current hour.
 */
static void draw_timeline(GridCtx *gctx, GRect area, int hours)
{
    const CalendarStrip *strip = calendar_store_strip();

    // the window starts at the top of the current hour so the cells line up with the clock
    time_t now = time(NULL);
    struct tm lt = *localtime(&now);
    lt.tm_min = 0;
    lt.tm_sec = 0;
    time_t window_start = mktime(&lt);

    int gap = 1;
    int cell_w = (area.size.w - gap * (hours - 1)) / hours;

    for (int hour = 0; hour < hours; hour++)
    {
        time_t cell_start = window_start + hour * 3600;
        time_t cell_end = cell_start + 3600;

        // mark the minutes any timed event covers in this hour, so overlapping events count once
        // and a short meeting fills only its share of the cell
        bool covered[60] = {0};
        for (uint8_t i = 0; i < strip->count; i++)
        {
            const CalendarEvent *event = &strip->event[i];
            if (event->all_day || event->start >= cell_end || event->end <= cell_start)
            {
                continue;
            }
            // clamp the event to this hour: floor the start minute, ceil the end, so any minute
            // the event touches at all reads busy
            int from = (event->start > cell_start) ? (int)((event->start - cell_start) / 60) : 0;
            int to = (event->end < cell_end) ? (int)((event->end - cell_start + 59) / 60) : 60;
            for (int minute = from; minute < to; minute++)
            {
                covered[minute] = true;
            }
        }

        // the busy span within the hour: the first and last minute any event covers. filling that
        // range (not just from the left) puts the block where the event actually sits, so a 4:30
        // start shades the right half of the hour instead of the left
        int first_busy = -1;
        int last_busy = -1;
        for (int minute = 0; minute < 60; minute++)
        {
            if (covered[minute])
            {
                if (first_busy < 0)
                {
                    first_busy = minute;
                }
                last_busy = minute;
            }
        }

        GRect cell = GRect(area.origin.x + hour * (cell_w + gap), area.origin.y, cell_w, area.size.h);
        graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_rect(gctx->ctx, cell);

        if (first_busy >= 0)
        {
            int fill_x = (cell_w * first_busy) / 60;
            int fill_w = (cell_w * (last_busy + 1)) / 60 - fill_x;
            if (fill_w < 1)
            {
                fill_w = 1; // a sliver so even a few busy minutes still register
            }
            graphics_context_set_fill_color(gctx->ctx, gctx->color_accent);
            graphics_fill_rect(gctx->ctx, GRect(cell.origin.x + fill_x, cell.origin.y, fill_w, cell.size.h),
                               0, GCornerNone);
        }
    }
}

// --- 1x4: the strip with the next event named underneath ---

static void draw_1x4(GridCtx *gctx)
{
    GRect body = gctx->body;
    const CalendarStrip *strip = calendar_store_strip();

    GRect bar = GRect(body.origin.x + PANEL_PAD, body.origin.y + 2, body.size.w - PANEL_PAD * 2, BAR_H);
    draw_timeline(gctx, bar, FREEBUSY_HOURS);

    // name the next event under the strip, or say the window is clear
    GRect caption_rect = GRect(bar.origin.x, bar.origin.y + BAR_H, bar.size.w, 14);
    if (strip->count > 0)
    {
        // colour the parts like the Next Event 1x4: a grey day code, the accent time, then the
        // title in the value colour so the event name reads in the vibrant colour
        const CalendarEvent *next = &strip->event[0];
        int x = caption_rect.origin.x;
        int right = caption_rect.origin.x + caption_rect.size.w;

        char day_code[4];
        cal_fmt_day_code(day_code, sizeof(day_code), next->start);
        cal_draw_text(gctx, day_code, FONT_STM_12, gctx->color_subtitle,
                      GRect(x, caption_rect.origin.y, right - x, caption_rect.size.h), GTextAlignmentLeft);
        x += caption_seg_width(day_code) + CAPTION_GAP;

        if (!next->all_day)
        {
            char time_buf[12];
            cal_fmt_time(time_buf, sizeof(time_buf), next->start, false);
            cal_draw_text(gctx, time_buf, FONT_STM_12, gctx->color_accent,
                          GRect(x, caption_rect.origin.y, right - x, caption_rect.size.h), GTextAlignmentLeft);
            x += caption_seg_width(time_buf) + CAPTION_GAP;
        }

        cal_draw_text(gctx, next->title, FONT_STM_12, gctx->color_value,
                      GRect(x, caption_rect.origin.y, right - x, caption_rect.size.h), GTextAlignmentLeft);
    }
    else
    {
        cal_draw_text(gctx, "Nothing Scheduled", FONT_STM_12, gctx->color_subtitle, caption_rect,
                      GTextAlignmentLeft);
    }
}

// --- 2x4: the agenda list with the eight-hour strip on top ---

static void draw_2x4(GridCtx *gctx)
{
    GRect body = gctx->body;
    const CalendarStrip *strip = calendar_store_strip();

    GRect bar = GRect(body.origin.x + PANEL_PAD, body.origin.y + 2, body.size.w - PANEL_PAD * 2, BAR_H);
    draw_timeline(gctx, bar, FREEBUSY_HOURS);

    int list_top = bar.origin.y + BAR_H + 4;

    if (strip->count == 0)
    {
        cal_draw_text(gctx, "Nothing Scheduled", FONT_STM_14, gctx->color_subtitle,
                      GRect(body.origin.x, list_top + 8, body.size.w, 16), GTextAlignmentCenter);
        return;
    }

    // the next few events under the strip, up to three
    int rows = strip->count < 3 ? strip->count : 3;
    for (int i = 0; i < rows; i++)
    {
        cal_draw_agenda_row(gctx, &strip->event[i], body, list_top + i * CAL_ROW_H);
    }
}

// --- 2x2: the next event with a short four-hour strip ---

static void draw_2x2(GridCtx *gctx)
{
    const CalendarEvent *event = calendar_store_event(0);

    if (!event)
    {
        gh_value_top(gctx, "--", FONT_TEKO_26, 30, 0, -3);
        gh_caption(gctx, "No Events", 24);
        return;
    }

    // the event's clock time (or day word for an all-day event) up top in the same hero size as
    // the Next Event panel, then the title as a caption
    if (event->all_day)
    {
        char day_word[12];
        cal_fmt_time(day_word, sizeof(day_word), event->start, true);
        gh_value_top(gctx, day_word, FONT_TEKO_34, 38, 0, -7);
    }
    else
    {
        // a timed event headlines the clock time with the small grey am/pm, matching Next Event
        struct tm lt = *localtime(&event->start);
        char time_buf[8] = "--:--";
        bool meridiem = gridlock_format_clock(time_buf, sizeof(time_buf), lt.tm_hour, lt.tm_min);
        gh_value_top_time(gctx, time_buf, meridiem, lt.tm_hour < 12, FONT_TEKO_34, 38, -7);
    }
    gh_caption(gctx, event->title, 27);

    // the date icon bottom-right, matching the Next Event panel
    icon_draw(gctx, &ICON_DATE_TIME, GAlignBottomRight, -EDGE_PAD(gctx), -5);

    // the short four-hour strip sits where the Next Event progress bar does: bottom-left, leaving
    // room for the icon on the right (the same width maths draw_progress_bar uses)
    int icon_w = icon_size(ICON_DATE_TIME.res).w;
    int bar_w = gctx->body.size.w - PANEL_PAD * 2 - icon_w - 6;
    if (bar_w > 0)
    {
        GRect bar = grid_anchor(gctx, GSize(bar_w, BAR_H), GAlignBottomLeft, PANEL_PAD + 1, -5);
        draw_timeline(gctx, bar, NEXT_HOURS);
    }
}

// --- dispatch ---

static void calendar_freebusy_body(GridCtx *gctx)
{
    if (gctx->size == MSIZE_1x4)
    {
        draw_1x4(gctx);
    }
    else if (gctx->size == MSIZE_2x4)
    {
        draw_2x4(gctx);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        draw_2x2(gctx);
    }
}

const ModuleDef mod_calendar_freebusy_def = {
    .label = "TIMELINE",
    .sizes = SZ_1x4 | SZ_2x4 | SZ_2x2,
    .features = FEATURE_CALENDAR,
    .theme_alias = MOD_CALENDAR_COUNTDOWN, // borrow the Calendar group's colour
    .body = calendar_freebusy_body
};
