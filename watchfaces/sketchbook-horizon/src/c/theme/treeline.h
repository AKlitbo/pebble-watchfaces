/**
 * @file treeline.h
 * @brief The Treeline scene's palette: one set of colours per theme, in a day and a night variant.
 *
 * Treeline draws its whole background, so a theme is a palette rather than a baked bitmap. Each
 * theme carries two of them and the scene swaps between them as the sun goes down, which is what
 * makes the same clearing read as morning or midnight. The common block at the top is what the
 * shared sky and weather code paints with, and the fields below are the forest and cabin this
 * scene adds.
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
 * @brief Every colour the Treeline scene paints with.
 *
 * Split by what a colour sits *on* rather than by what draws it: `ink` is linework over the
 * sky, `text` is the clock down on the ground, and the two are rarely the same once the forest
 * floor is dark.
 */
typedef struct TreelinePalette
{
    SKETCHBOOK_PALETTE_COMMON
    GColor tree_far; ///< The back treeline, small and crowded
    GColor tree;     ///< The near treeline, the big firs at the clearing's edge
    GColor ground;   ///< The clearing floor, the field the clock sits on
    GColor cabin;    ///< The cabin's log walls
    GColor roof;     ///< Its roof, and the chimney
    GColor glow;     ///< The lit window after dark
    GColor smoke;    ///< The plume off the chimney
} TreelinePalette;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const TreelinePalette *treeline_palette_for_theme(uint8_t theme, bool night);

/** @} */
