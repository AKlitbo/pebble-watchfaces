/**
 * @file sprockets.h
 * @brief The perforated strip where the left field meets the reel panel.
 *
 * A stipple ramp between the two halves reads as noise rather than as a join. Film perforations
 * say it deliberately instead: the panel runs on past the field, and a row of holes punched down
 * its edge is what the numbers turn on.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief Paint the perforated strip.
 *
 * The strip is panel-coloured, so it reads as part of the reel rather than as a gutter between
 * two things, and the holes are punched through to the field colour behind.
 *
 * @param ctx The graphics context.
 * @param strip Where to paint, in the layer's own coordinates.
 * @param panel The strip's own colour.
 * @param hole The colour showing through each perforation.
 */
void sprockets_draw(GContext *ctx, GRect strip, GColor panel, GColor hole);

/** @} */
