/**
 * @file theme.h
 * @brief The scene palette: one set of colours per theme, in a day and a night variant.
 *
 * Skyline draws its whole background, so a theme is a palette rather than a baked bitmap.
 * Each theme carries two of them and the face swaps between them as the sun goes down, which
 * is what turns the same bridge from a daytime silhouette into a string of lights on water.
 *
 * @ingroup watchface-skyline
 */
#pragma once
#include <pebble.h>

#include "sketchbook/theme/palette.h"

/**
 * @defgroup watchface-skyline Skyline Watchface
 * @brief The Skyline Emery watchface: a river bridge whose lamps light at dusk and lay their
 * reflections down the water.
 * @{
 */

/**
 * @brief Every colour the scene paints with.
 *
 * The nine the family shares, plus what a river crossing is made of. There is one water colour
 * rather than a near and a far: this river is a mirror, not a shore, and the only thing that
 * gives it depth is what it reflects.
 */
typedef struct Palette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor city;   ///< The blocks standing back behind the bridge
    GColor bridge; ///< Towers, cables and deck, and an unlit lamp
    GColor water;  ///< The river, and the field the clock sits on
    GColor lamp;   ///< A lamp or window with the light on, and its reflection below
} Palette;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const Palette *palette_for_theme(uint8_t theme, bool night);

/** @} */
