/**
 * @file ridgeline.h
 * @brief The Ridgeline scene's ground: what the land is painted with, per theme, by day and by night.
 *
 * Ridgeline draws its whole background, so a theme is a palette rather than a baked bitmap. The sky
 * over it is the shared one in palette.h. This is the land under it, which the scene swaps along with
 * the sky as the sun goes down.
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
 * @brief The colours of Ridgeline's land.
 *
 * The clock sits on the land and the linework sits over the sky, which is why a dark theme pairs a
 * light `text` with a dark `land`.
 */
typedef struct RidgelineGround
{
    GroundReadouts readouts; ///< The clock and the marks on the land
    GColor land_far;         ///< Body of the back ridge
    GColor land;             ///< Body of the front ridge, the field the clock sits on
} RidgelineGround;

/**
 * @brief The ground for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param night True for the after-dark ground.
 * @return The ground row. It comes back as a const void * so the scene table can hold every scene's
 * ground lookup alike, and the scene casts it to a RidgelineGround.
 */
const void *ridgeline_ground_for_theme(uint8_t theme, bool night);

/** @} */
