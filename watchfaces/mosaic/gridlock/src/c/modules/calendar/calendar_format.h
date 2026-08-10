/**
 * @file calendar_format.h
 * @brief Small formatting and drawing helpers shared by the calendar modules: an event's clock
 * time (or day word for an all-day event), a two-letter weekday code, the shared text drawer, and
 * one agenda-list row.
 *
 * @ingroup gridlock_mod_calendar
 */
#pragma once
#include <pebble.h>
#include "mosaic/draw/grid_helpers.h"        // GridCtx, FontId, PANEL_PAD
#include "io/stores/calendar_store.h" // CalendarEvent

// one agenda-list row's height, so a panel laying out a list can step by it
#define CAL_ROW_H 16

/**
 * @brief Formats an event start for display.
 *
 * A timed event becomes a clock time ("19:30", honouring the 12/24h setting). An all-day event
 * has no clock, so it becomes the day it lands on ("Today" / "Tomorrow" / a short weekday).
 *
 * @param out The buffer to fill.
 * @param n Its size.
 * @param start The event start epoch.
 * @param all_day True when the event is all-day.
 */
void cal_fmt_time(char *out, size_t n, time_t start, bool all_day);

/**
 * @brief Writes a two-letter uppercase weekday code for an event ("MO", "TU", ... "SU"). The
 * one-week lookahead means these never repeat, so they place an event on its day unambiguously.
 *
 * @param out The buffer to fill.
 * @param n Its size.
 * @param start The event start epoch.
 */
void cal_fmt_day_code(char *out, size_t n, time_t start);

/**
 * @brief Draws text into a box with the font's top padding removed and a trailing ellipsis. The
 * shared text drawer for the calendar rows and captions.
 *
 * @param gctx The grid context.
 * @param text The text to draw.
 * @param font The font.
 * @param color The text colour.
 * @param box The box to draw into.
 * @param align The horizontal alignment.
 */
void cal_draw_text(GridCtx *gctx, const char *text, FontId font, GColor color, GRect box,
                   GTextAlignment align);

/**
 * @brief Draws one agenda row at y: a grey day code, the accent clock time (blank for an all-day
 * event), then the title filling the rest. Shared by the Agenda 2x4 and the Timeline 2x4 list.
 *
 * @param gctx The grid context.
 * @param event The event to draw.
 * @param body The panel body rect (for the row's left inset and width).
 * @param y The row's top, body-relative already applied by the caller.
 */
void cal_draw_agenda_row(GridCtx *gctx, const CalendarEvent *event, GRect body, int y);
