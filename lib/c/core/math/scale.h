/**
 * @file scale.h
 * @brief The clamp and segment maths a bar or gauge is built from, no SDK.
 *
 * A gauge, a battery glyph, and a graph all do the same handful of sums: pin a value into a range,
 * carve a strip into even cells, and map a reading to a pixel. Those live here as plain integer
 * functions so the drawing that uses them stays on the SDK side and the maths can be tested off the
 * watch. Nothing here reaches for a GRect: a caller hands over the width or the height it read.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Pin a value into a range, so it lands on the nearer end when it falls outside.
 *
 * @param value The value to pin.
 * @param lo The low end, returned when the value is below it.
 * @param hi The high end, returned when the value is above it.
 * @return The value, or whichever end it sat outside.
 */
int clamp_int(int value, int lo, int hi);

/**
 * @brief The width of one cell when a strip is carved into evenly spaced segments.
 *
 * The gaps sit between the cells, so there is one fewer gap than there are cells. The division
 * truncates, which leaves a few pixels of remainder for the caller to centre or absorb.
 *
 * @param total The strip's full width.
 * @param gap The space between two cells.
 * @param count How many cells there are.
 * @return The width of one cell, or 0 for a count of zero so there is no divide by it. Can come
 *   back below 1 when the cells do not fit, which the caller is expected to notice.
 */
int segment_width(int total, int gap, int count);

/**
 * @brief A fraction of a length in pixels, e.g. how far along a bar a moment sits.
 *
 * @param total The full length.
 * @param num The numerator of the fraction.
 * @param den The denominator of the fraction.
 * @return total * num / den, or 0 for a denominator of zero so there is no divide by it.
 */
int fraction_px(int total, int num, int den);

/**
 * @brief How many cells of a gauge a percentage lights, rounding up so any charge lights one.
 *
 * The level is pinned to 0..100 first, so a stray reading cannot light more cells than there are.
 * Rounding up means 1% still shows a cell and 100% fills them all.
 *
 * @param level The fill from 0 to 100.
 * @param segments How many cells the gauge has.
 * @return The number of lit cells, from 0 to segments.
 */
int segments_filled(int level, int segments);

/**
 * @brief Map a reading to its pixel row in a plot, with the low end at the bottom.
 *
 * The window runs from lo at the bottom row to hi at the top, so a bigger reading sits higher up
 * the panel. The reading is expected to be inside the window already. A caller that cannot promise
 * that clamps it with clamp_int first.
 *
 * @param y0 The top row of the plot area.
 * @param height The plot area's height in rows.
 * @param lo The value at the bottom of the window.
 * @param hi The value at the top of the window.
 * @param value The reading to place.
 * @return The pixel row for the reading. A window with no height (hi <= lo) gives the bottom row.
 */
int plot_y(int y0, int height, int lo, int hi, int value);

/** @} */
