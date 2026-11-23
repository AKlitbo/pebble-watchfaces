/**
 * @file marks.h
 * @brief Small pen marks more than one scene draws: grass tufts, a tree drawn as a trunk with its
 * branches hanging off the top, and a far-off bird.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Tuft the ground at each point, so it reads as grass rather than blank ground.
 *
 * Three short strokes fanning up off a point, which is how the rest of the face draws. An ordered
 * dot fill covers the same ground but reads as a printing screen rather than as grass. It leaves
 * the stroke colour on the tufts' colour and the width at 1.
 *
 * @param ctx The graphics context.
 * @param pts Where each tuft grows from.
 * @param count How many tufts there are.
 * @param color The tufts' colour.
 */
void marks_tufts(GContext *ctx, const GPoint *pts, int count, GColor color);

/**
 * @brief Draw a tree as a 2px trunk with 1px branches hanging off its top.
 *
 * Each branch goes out to a bend and then on to its tip, so it droops rather than sticking out
 * straight. It leaves the stroke colour on the tree's colour and the width at 1.
 *
 * @param ctx The graphics context.
 * @param foot Where the trunk meets the ground.
 * @param top The top of the trunk, which every branch starts from.
 * @param branches Each branch as its bend and then its tip, both as x and y offsets from the top.
 * @param count How many branches there are.
 * @param color The tree's colour.
 */
void marks_tree(GContext *ctx, GPoint foot, GPoint top, const int8_t (*branches)[4], int count,
    GColor color);

/**
 * @brief Draw a bird as two shallow chevrons meeting at the body, which is all a bird is this far
 * off. It draws in the stroke colour and width already set.
 *
 * @param ctx The graphics context.
 * @param at Where the body is.
 * @param span Half the wingspan.
 * @param lift How far the elbows of the wings rise, or drop when negative. Keep it small, since a
 * wing that rises as far as it reaches comes out as a mountain.
 */
void marks_bird(GContext *ctx, GPoint at, int span, int lift);

/** @} */
