/**
 * @file shoreline.h
 * @brief The Shoreline scene's palette: one set of colours per theme, in a day and a night variant.
 *
 * Shoreline draws its whole background, so a theme is a palette rather than a baked bitmap. Each
 * theme carries two of them and the scene swaps between them as the sun goes down, which is what
 * makes the same beach read as morning or midnight. The common block at the top is what the shared
 * sky and weather code paints with, and the fields below are the sea and sand this scene adds.
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
 * @brief Every colour the Shoreline scene paints with.
 *
 * Split by what a colour sits *on* rather than by what draws it: `ink` is linework over the sky
 * and the water, `text` is the clock down on the sand, and the two are rarely the same once the
 * beach is dark.
 */
typedef struct ShorelinePalette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor sea_far;  ///< Water out at the horizon
    GColor sea;      ///< Water in close, where the waves are breaking
    GColor sand;     ///< The dry beach, the field the clock sits on
    GColor wet;      ///< Sand the tide has just come off, in the strip below the waterline
    GColor foam;     ///< The surf line and the wave crests
} ShorelinePalette;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const ShorelinePalette *shoreline_palette_for_theme(uint8_t theme, bool night);

/** @} */
