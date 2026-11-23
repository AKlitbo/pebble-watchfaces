/**
 * @file skyline.h
 * @brief The Skyline scene's ground: what the city, the bridge, and the river are painted with, per
 * theme, by day and by night.
 *
 * Skyline draws its whole background, so a theme is a palette rather than a baked bitmap. The sky
 * over it is the shared one in palette.h. This is the crossing under it, which the scene swaps along
 * with the sky as the sun goes down, turning the same bridge from a daytime silhouette into a string
 * of lights on water. The landmarks that stand on water paint with it too.
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
 * @brief The colours of Skyline's crossing, and of a landmark that stands on water.
 *
 * There is one water colour rather than a near and a far. This river is a mirror, not a shore, and
 * the only thing that gives it depth is what it reflects.
 */
typedef struct SkylineGround
{
    GroundReadouts readouts; ///< The clock and the marks on the water
    GColor city;             ///< The blocks standing back behind the bridge
    GColor bridge;           ///< Towers, cables and deck, and an unlit lamp
    GColor water;            ///< The river, and the field the clock sits on
    GColor lamp;             ///< A lamp or window with the light on, and its reflection below
} SkylineGround;

/**
 * @brief The ground for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param night True for the after-dark ground.
 * @return The ground row. It comes back as a const void * so the scene table can hold every scene's
 * ground lookup alike, and the scene casts it to a SkylineGround.
 */
const void *skyline_ground_for_theme(uint8_t theme, bool night);

/** @} */
