/**
 * @file shoreline.h
 * @brief The Shoreline scene's ground: what the sea and the sand are painted with, per theme, by
 * day and by night.
 *
 * Shoreline draws its whole background, so a theme is a palette rather than a baked bitmap. The sky
 * over it is the shared one in palette.h. This is the beach under it, which the scene swaps along
 * with the sky as the sun goes down.
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
 * @brief The colours of Shoreline's sea and sand.
 *
 * Linework goes over the water in the sky's `ink`, and the clock sits down on the sand in `text`.
 * The two are rarely the same once the beach is dark.
 */
typedef struct ShorelineGround
{
    GroundReadouts readouts; ///< The clock and the marks on the sand
    GColor sea_far;          ///< Water out at the horizon
    GColor sea;              ///< Water in close, where the waves are breaking
    GColor sand;             ///< The dry beach, the field the clock sits on
    GColor wet;              ///< Sand the tide has just come off, in the strip below the waterline
    GColor foam;             ///< The surf line and the wave crests
} ShorelineGround;

/**
 * @brief The ground for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param night True for the after-dark ground.
 * @return The ground row. It comes back as a const void * so the scene table can hold every scene's
 * ground lookup alike, and the scene casts it to a ShorelineGround.
 */
const void *shoreline_ground_for_theme(uint8_t theme, bool night);

/** @} */
