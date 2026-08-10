/**
 * @file header_fonts.h
 * @brief The user-selectable header fonts. One table maps the Header Font setting to the font
 * resource plus the pixel nudge that seats it in the 14px header strip. main.c loads the chosen
 * resource under FONT_HEADER and the grid engine reads the nudge, so this is the single source of
 * truth both sides share.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup mosaic_draw
 * @{
 */

/// How many header fonts the picker offers. Feeds the settings enum_count and the Clay select
#define HEADER_FONT_COUNT 8

/**
 * @brief One header font: the resource to load and the nudge that seats it in the header strip.
 */
typedef struct
{
    uint32_t resource; ///< The FONT_* resource id
    int8_t   dx;       ///< Header label nudge right, to line the font up in the block
    int8_t   dy;       ///< Header label nudge down, to seat the font in the 14px strip
} HeaderFontSpec;

/**
 * @brief Looks up a header font by its setting value.
 *
 * @param choice The Header Font setting value. Out of range falls back to the default (0).
 * @return The chosen font's spec.
 */
const HeaderFontSpec *header_font_spec(uint8_t choice);

/** @} */
