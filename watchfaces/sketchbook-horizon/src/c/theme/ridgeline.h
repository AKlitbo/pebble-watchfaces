/**
 * @file ridgeline.h
 * @brief The Ridgeline scene's palette: one set of colours per theme, in a day and a night variant.
 *
 * Ridgeline draws its whole background, so a theme is a palette rather than a baked bitmap. Each
 * theme carries two of them and the scene swaps between them as the sun goes down, which is what
 * makes the same scene read as morning or midnight. The common block at the top is what the shared
 * sky and weather code paints with, and the two fields below are the ridges this scene adds.
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
 * @brief Every colour the Ridgeline scene paints with.
 *
 * The clock sits on the land and the linework sits over the sky, which is why a dark theme pairs a
 * light `text` with a dark `land` and a pale `ink` over a near-black sky.
 */
typedef struct RidgelinePalette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor land_far; ///< Body of the back ridge
    GColor land;     ///< Body of the front ridge, the field the clock sits on
} RidgelinePalette;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const RidgelinePalette *ridgeline_palette_for_theme(uint8_t theme, bool night);

/** @} */
