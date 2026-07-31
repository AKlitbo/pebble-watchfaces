/**
 * @file theme.h
 * @brief The scene palette: one set of colours per theme, in a day and a night variant.
 *
 * Shoreline draws its whole background, so a theme is a palette rather than a baked bitmap.
 * Each theme carries two of them and the face swaps between them as the sun goes down, which
 * is what makes the same beach read as morning or midnight.
 *
 * @ingroup watchface-shoreline
 */
#pragma once
#include <pebble.h>

#include "sketchbook/theme/palette.h"

/**
 * @defgroup watchface-shoreline Shoreline Watchface
 * @brief The Shoreline Emery watchface: a drawn sea and beach with a tide that comes and goes.
 * @{
 */

/**
 * @brief Every colour the scene paints with.
 *
 * Split by what a colour sits *on* rather than by what draws it: `ink` is linework over the sky
 * and the water, `text` is the clock down on the sand, and the two are rarely the same once the
 * beach is dark.
 */
typedef struct Palette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor sea_far;  ///< Water out at the horizon
    GColor sea;      ///< Water in close, where the waves are breaking
    GColor sand;     ///< The dry beach, the field the clock sits on
    GColor wet;      ///< Sand the tide has just come off, in the strip below the waterline
    GColor foam;     ///< The surf line and the wave crests
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
