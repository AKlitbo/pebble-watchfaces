/**
 * @file band.h
 * @brief The day wrapped around the outside of the screen.
 *
 * A track hugging all four edges carries one full day, midnight at top centre running clockwise.
 * The stretch the sun is up for is shaded apart from the rest, and a marker rides round it at the
 * current time, so a glance at the frame says where in the day you are without reading a number.
 *
 * The sums come from lib/c/core/clock/timeband, which measures everything as an offset from the
 * window's start in whatever unit the caller keeps. A ring hands it a full turn and asks for an
 * angle. This hands it the rectangle's perimeter and asks for a distance along the outline, so
 * nothing here ever has to think about midnight.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief Paint the day track: the night ground, the daylight over it, and the now marker.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param chrome The face colours to paint from.
 */
void band_draw(GContext *ctx, GRect bounds, const Chrome *chrome);

/** @} */
