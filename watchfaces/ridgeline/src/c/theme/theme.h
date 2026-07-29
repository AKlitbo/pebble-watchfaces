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
    GColor sky_hi;   ///< Sky at the top of the screen
    GColor sky_lo;   ///< Sky just above the ridges, so the two bands give the air some depth
    GColor land_far; ///< Body of the back ridge
    GColor land;     ///< Body of the front ridge, the field the clock sits on
    GColor ink;      ///< Linework over the sky: ridgelines, cloud and disc outlines, the guide arc
    GColor text;     ///< The clock, over the land
    GColor dim;      ///< Date and the stats row
    GColor bar_ink;  ///< The status bar's contents, which always sit on the dark strip
    GColor disc;     ///< Sun or moon fill, and the sun's rays
    GColor cloud;    ///< Cloud fill
    GColor precip;   ///< Rain, sleet, snow, and the fog banks
} Palette;

/// Themes the Clay picker offers, in its option order. The count caps KNOWN_THEME in the schema
typedef enum
{
    THEME_SKETCHBOOK = 0,
    THEME_DAYBREAK   = 1,
    THEME_ALPENGLOW  = 2,
    THEME_BLUEPRINT  = 3,
    THEME_FOREST     = 4,
    THEME_MONO       = 5,
    THEME_CYBERPUNK  = 6,
    THEME_NEO_TOKYO  = 7,
    THEME_COUNT
} RidgelineTheme;

/**
 * @brief The palette for a theme at this hour of the day.
 *
 * An out-of-range theme falls back to Sketchbook, matching how the settings parser clamps.
 *
 * @param theme The theme setting value.
 * @param night True for the after-dark palette.
 * @return The palette to paint with.
 */
const Palette *palette_for(uint8_t theme, bool night);

/**
 * @brief The lit-segment colour for the battery gauge at a given charge level.
 *
 * Colour themes warn with red (critical) and amber (low). Mono stays greyscale, leaving the
 * lit-segment count to signal charge.
 *
 * @param theme The theme setting value.
 * @param ink The colour the gauge is drawn in when the charge is healthy.
 * @param level Battery charge level percentage.
 * @return The fill colour for lit segments.
 */
GColor battery_fill_for(uint8_t theme, GColor ink, int level);

/** @} */
