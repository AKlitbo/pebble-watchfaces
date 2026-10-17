/**
 * @file skyline.h
 * @brief The Skyline scene's palette: one set of colours per theme, in a day and a night variant.
 *
 * Skyline draws its whole background, so a theme is a palette rather than a baked bitmap. Each
 * theme carries two of them and the scene swaps between them as the sun goes down, which is what
 * turns the same bridge from a daytime silhouette into a string of lights on water. The common
 * block at the top is what the shared sky and weather code paints with, and the four fields below
 * are the crossing this scene adds.
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
 * @brief Every colour the Skyline scene paints with.
 *
 * The nine the family shares, plus what a river crossing is made of. There is one water colour
 * rather than a near and a far: this river is a mirror, not a shore, and the only thing that
 * gives it depth is what it reflects.
 */
typedef struct SkylinePalette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor city;   ///< The blocks standing back behind the bridge
    GColor bridge; ///< Towers, cables and deck, and an unlit lamp
    GColor water;  ///< The river, and the field the clock sits on
    GColor lamp;   ///< A lamp or window with the light on, and its reflection below
} SkylinePalette;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const SkylinePalette *skyline_palette_for_theme(uint8_t theme, bool night);

/** @} */
