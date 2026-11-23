/**
 * @file line.h
 * @brief Lines drawn from a table of points: a run joined end to end, loose pairs, and a dashed
 * path.
 *
 * None of these pick a colour or a width. They draw with whatever stroke the context already has,
 * so the caller sets it first. That keeps one helper for a crest drawn 3px wide in ink and a river
 * drawn 1px wide in the dim colour.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// One straight stroke, a pair of points. A typedef rather than GPoint(*)[2] written out, since
/// GPoint is a function-like macro and the parenthesis beside it would be taken for a call.
typedef GPoint LineSegment[2];

/**
 * @brief Draw a line through a run of points, each one joined to the next.
 *
 * @param ctx The graphics context, with its stroke colour and width already set.
 * @param pts The points, in order.
 * @param count How many there are. Fewer than two draws nothing.
 */
void line_polyline(GContext *ctx, const GPoint *pts, int count);

/**
 * @brief Draw each pair of points as its own stroke.
 *
 * @param ctx The graphics context, with its stroke colour and width already set.
 * @param segments The pairs.
 * @param count How many pairs there are.
 */
void line_segments(GContext *ctx, const LineSegment *segments, int count);

/**
 * @brief Draw a run of points as a dashed path, so it reads as a trodden path rather than a drawn
 * line.
 *
 * Each leg is walked in whole pixels and starts a fresh dash at its own first point, so the
 * dashes stay even up a shallow slope.
 *
 * @param ctx The graphics context, with its stroke colour and width already set.
 * @param pts The points, in order.
 * @param count How many there are.
 * @param on How long each dash runs, in pixels. Has to be above 0, or nothing draws.
 * @param period How far one dash starts from the next, so the gap is period less on. Has to be
 * above 0, or nothing draws.
 */
void line_dashed(GContext *ctx, const GPoint *pts, int count, int on, int period);

/** @} */
