/**
 * @file battery.h
 * @brief The battery gauge's warning colours.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "theme/palette.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief The lit-segment colour for the battery gauge at a given charge level.
 *
 * The warning colours come from the palette on screen, so each theme picks its own. Colour themes
 * warn in amber when low and red when critical. Mono's warning colours are its `bar_ink`, so its
 * gauge never changes colour and the count of lit segments carries the charge. That match is kept
 * by hand, in Mono's rows of the sky table in palette.c.
 *
 * @param pal The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param level Battery charge level percentage.
 * @return The fill colour for lit segments.
 */
GColor sketchbook_battery_fill(const CommonPalette *pal, int level);

/** @} */
