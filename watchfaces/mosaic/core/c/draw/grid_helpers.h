/**
 * @file grid_helpers.h
 * @brief Handy building blocks for drawing the inside of a panel. The idea is that a
 * panel works out its text and numbers first and then calls these to put it on screen.
 * The shared 1x2 panel (value on the left, icon on the right) and 2x2 panel (value up
 * top, two caption lines, a corner icon, and a progress bar) live here so every stat
 * panel shares one copy.
 *
 * The little offsets in here hold the exact pixel placement each panel draws with. The
 * font top padding fix comes from metrics.h (metric_baseline). The bigger nudges (like
 * the 2x2 top value) are written out and explained.
 *
 * How things get placed: text starts 5px in from the left. The 1x2 icon sits 5px in
 * from the right with its bottom matching the value's bottom. The 2x2 stat icon is a
 * 5px corner piece instead. Each icon's own empty space is sorted out once in its
 * IconSpec (icons.h) so the callers just say which icon they want and pass no offsets.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include <pebble.h>
#include "engine/grid_engine.h"
#include "mosaic/draw/icons.h"
#include "ui/fonts.h"

/**
 * @addtogroup mosaic_draw
 * @{
 */

// --- primitives ---

/**
 * @brief Draws a value lined up to the left and centred top to bottom, with the font's
 * top padding taken off so it sits where you expect.
 *
 * @param gctx The grid context.
 * @param text The value text.
 * @param font The font for the value.
 * @param box_w How wide the text box is.
 * @param value_h How tall the text box is.
 * @param pad How far in from the left edge to start.
 * @return The box the value was drawn in, handy for putting a label after it.
 */
GRect gh_value_left(GridCtx *gctx, const char *text, FontId font, int box_w, int value_h, int pad);

/**
 * @brief Draws a value pinned to the top and centred left to right. dy is the extra
 * lift we picked on purpose. These big values sit a bit higher than the plain top
 * padding fix would put them.
 *
 * @param gctx The grid context.
 * @param text The value text.
 * @param font The font for the value.
 * @param value_h How tall the text box is.
 * @param dx Sideways nudge.
 * @param dy Up and down nudge for the look we want.
 * @return The box the value was drawn in.
 */
GRect gh_value_top(GridCtx *gctx, const char *text, FontId font, int value_h, int dx, int dy);

/**
 * @brief Draws a centred caption line in Gothic 14 in the subtitle colour, sat down
 * from the top of the body by y.
 *
 * @param gctx The grid context.
 * @param text The caption text.
 * @param y How far below the body top to put it.
 */
void gh_caption(GridCtx *gctx, const char *text, int y);

/**
 * @brief Draws a graph's big value centred across the top of the body in Teko 26,
 * in the value colour, sat just above the body so it clears the low/high labels.
 *
 * Shared by the HR and steps graphs, which both crown the plot with one big number.
 *
 * @param gctx The grid context.
 * @param text The value text.
 */
void gh_graph_value_top(GridCtx *gctx, const char *text);

/**
 * @brief Draws a gauge made of separate cells. Each cell is outlined and the first
 * few are filled in to match the level.
 *
 * @param gctx The grid context.
 * @param area The box the gauge goes in.
 * @param segments How many cells to draw.
 * @param level How full to fill it from 0 to 100.
 */
void gh_gauge(GridCtx *gctx, GRect area, int segments, int level);

/**
 * @brief Puts an icon so its bottom edge matches the visible bottom of a value that
 * was already drawn, sat 5px in from the right edge. We figure out the value's bottom
 * from the font's size facts and then add the icon's own little nudge (spec.dx/dy).
 *
 * @param gctx The grid context.
 * @param icon The icon to place.
 * @param value_rect The box the value was drawn in.
 * @param value_font The font the value used, so we know its cap height.
 */
void gh_icon_bottom_of(GridCtx *gctx, const IconSpec *icon, GRect value_rect, FontId value_font);

/**
 * @brief Formats a 24-hour "HH:MM" string (as the phone sends sunrise/sunset) into the
 * user's clock style, or "--" when the source isn't a real time.
 *
 * @param out Buffer that receives the formatted clock.
 * @param n Size of out.
 * @param src The "HH:MM" source string.
 */
void gh_format_hhmm(char *out, size_t n, const char *src);

// --- composite panels ---

/**
 * @brief The shared 2x2 stat panel. It has a value up top that shrinks itself if it
 * is too wide, two centred caption lines, an icon in the bottom-right corner, and a
 * progress bar in the bottom-left.
 *
 * @param gctx The grid context.
 * @param val The value text.
 * @param line1 The first caption line.
 * @param line2 The second caption line.
 * @param icon The corner icon. Its own little nudge is added for you.
 * @param progress_pct How full the progress bar is from 0 to 100.
 */
void gh_stat_2x2(GridCtx *gctx, const char *val, const char *line1, const char *line2,
                 const IconSpec *icon, int progress_pct);

/**
 * @brief Draws a clock time centred up top with a small grey AM/PM flush after it (the same
 * big-time-plus-meridiem look as the digital clock), at the caller's chosen font. The shared
 * value-top used by both gh_stat_time_2x2's shrink ladder and the calendar panels.
 *
 * @param gctx The grid context.
 * @param time_str The clock time without any am/pm suffix (e.g. "12:32").
 * @param meridiem True to draw an AM/PM label (a 12h clock); false draws none (a 24h clock).
 * @param is_am True for AM, false for PM (ignored when meridiem is false).
 * @param font The value font.
 * @param v_h The value box height.
 * @param v_dy The value's vertical nudge from the top.
 */
void gh_value_top_time(GridCtx *gctx, const char *time_str, bool meridiem, bool is_am,
                       FontId font, int v_h, int v_dy);

/**
 * @brief A gh_stat_2x2 whose value is a clock time with a small grey AM/PM after it, the same
 * big-time-plus-meridiem look as the digital clock. For panels that headline an event time.
 *
 * @param gctx The grid context.
 * @param time_str The clock time without any am/pm suffix (e.g. "12:32").
 * @param meridiem True to draw an AM/PM label (a 12h clock); false draws none (a 24h clock).
 * @param is_am True for AM, false for PM (ignored when meridiem is false).
 * @param line1 The first caption line.
 * @param line2 The second caption line.
 * @param icon The corner icon.
 * @param progress_pct How full the progress bar is from 0 to 100.
 */
void gh_stat_time_2x2(GridCtx *gctx, const char *time_str, bool meridiem, bool is_am,
                      const char *line1, const char *line2, const IconSpec *icon, int progress_pct);

/**
 * @brief A gh_stat_2x2 for the goal-tracking panels (steps, calories, distance, sleep,
 * heart rate). Wraps the goal string as the "OF <goal>" first caption line and works the
 * progress bar out from value and goal, so each module just supplies its formatted goal.
 *
 * @param gctx The grid context.
 * @param val The value text.
 * @param goal_str The formatted goal, without the "OF " prefix (e.g. "10,000", "8H").
 * @param caption The second caption line under the "OF" line (e.g. "GOAL", "LIMIT").
 * @param icon The corner icon. Its own little nudge is added for you.
 * @param value The raw value, for the progress percent.
 * @param goal The raw goal, for the progress percent.
 */
void gh_stat_goal_2x2(GridCtx *gctx, const char *val, const char *goal_str, const char *caption,
                      const IconSpec *icon, int value, int goal);

/**
 * @brief The shared 1x2 stat panel. A value on the left with an optional little unit
 * label after it and an icon on the right whose bottom matches the value's bottom.
 *
 * @param gctx The grid context.
 * @param val The value text.
 * @param trailing A small label after the value if you want one. Pass NULL or "" for
 * none. For example "BPM".
 * @param font The font for the value.
 * @param icon The right-hand icon. Its own little nudge is added for you.
 */
void gh_stat_1x2(GridCtx *gctx, const char *val, const char *trailing, FontId font,
                 const IconSpec *icon);

/**
 * @brief One stacked "mini row" used by the 2x2 conditions and daylight panels. It
 * has a small XS caption label, a value under it, an optional unit after the value and
 * a right-hand icon whose bottom matches the value. Two of these stack up to fill a
 * 2x2 body.
 *
 * @param gctx The grid context.
 * @param label The XS caption above the value.
 * @param value The value text.
 * @param trailing A unit after the value if you want one. Pass NULL or "" for none.
 * For example "KM/H".
 * @param icon The right-hand icon. Its own little nudge is added for you.
 * @param top How far down the body the row starts.
 */
void gh_mini_row(GridCtx *gctx, const char *label, const char *value, const char *trailing,
                 const IconSpec *icon, int top);

/** @} */
