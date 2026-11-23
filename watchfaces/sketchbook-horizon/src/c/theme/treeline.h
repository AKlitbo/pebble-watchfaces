/**
 * @file treeline.h
 * @brief The Treeline scene's ground: what the firs, the cabin, and the clearing are painted with,
 * per theme, by day and by night.
 *
 * Treeline draws its whole background, so a theme is a palette rather than a baked bitmap. The sky
 * over it is the shared one in palette.h. This is the forest under it, which the scene swaps along
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
 * @brief The colours of Treeline's forest and clearing.
 *
 * Linework goes over the sky in `ink`, and the clock sits down on the clearing floor in `text`. The
 * two are rarely the same once the forest floor is dark.
 */
typedef struct TreelineGround
{
    GroundReadouts readouts; ///< The clock and the marks on the clearing floor
    GColor tree_far;         ///< The back treeline, small and crowded
    GColor tree;             ///< The near treeline, the big firs at the clearing's edge
    GColor ground;           ///< The clearing floor, the field the clock sits on
    GColor cabin;            ///< The cabin's log walls
    GColor roof;             ///< Its roof, and the chimney
    GColor glow;             ///< The lit window after dark
    GColor smoke;            ///< The plume off the chimney
} TreelineGround;

/**
 * @brief The ground for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The palette on screen, which Shuffle or the night or Quiet Time palette can pick.
 * @param night True for the after-dark ground.
 * @return The ground row. It comes back as a const void * so the scene table can hold every scene's
 * ground lookup alike, and the scene casts it to a TreelineGround.
 */
const void *treeline_ground_for_theme(uint8_t theme, bool night);

/** @} */
