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
    int8_t   t_dx;     ///< Extra nudge right for a label that starts with T, which reads as pushed in otherwise
} HeaderFontSpec;

/**
 * @brief Looks up a header font by its setting value.
 *
 * @param choice The Header Font setting value. Out of range falls back to the default (0).
 * @return The chosen font's spec.
 */
const HeaderFontSpec *header_font_spec(uint8_t choice);

/**
 * @brief How far right to nudge one header label in a header font.
 *
 * A T leaves open space under its bar, so in some fonts it reads as pushed in from the edge
 * where a straight-sided letter does not. Those fonts pull a leading T back with t_dx.
 *
 * @param spec The header font.
 * @param label The label as it will be drawn.
 * @return The nudge in pixels, the font's dx plus t_dx for a label that starts with T.
 */
int header_label_dx(const HeaderFontSpec *spec, const char *label);

/**
 * @brief Loads a header font in place of the one in use, and frees the old one.
 *
 * The new font loads before the old one is freed. The firmware keeps the last font it drew
 * with by address and only drops what it cached for it when that address changes, so a new
 * font loaded into the old one's freed memory draws with the old font's glyph offsets, and
 * comes out with letters missing and noise in their place.
 *
 * @param old The font in use, or NULL when there is none yet.
 * @param choice The Header Font setting value to load.
 * @return The new font.
 */
GFont header_font_swap(GFont old, uint8_t choice);

/** @} */
