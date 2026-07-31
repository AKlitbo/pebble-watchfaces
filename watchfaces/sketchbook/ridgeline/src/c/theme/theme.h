/**
 * @file theme.h
 * @brief The scene palette: one set of colours per theme, in a day and a night variant.
 *
 * Ridgeline draws its whole background, so a theme is a palette rather than a baked bitmap.
 * Each theme carries two of them and the face swaps between them as the sun goes down, which
 * is what makes the same scene read as morning or midnight.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "sketchbook/theme/palette.h"

/**
 * @defgroup watchface-ridgeline Ridgeline Watchface
 * @brief The Ridgeline Emery watchface: a drawn sun/moon and mountain scene that follows the sky.
 * @{
 */

/**
 * @brief Every colour the scene paints with.
 *
 * Split by what a colour sits *on* rather than by what draws it: `ink` is linework over the
 * sky, `text` is the clock over the land, and the two are rarely the same once the land is
 * dark.
 */
typedef struct Palette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor land_far; ///< Body of the back ridge
    GColor land;     ///< Body of the front ridge, the field the clock sits on
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
