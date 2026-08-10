/**
 * @file panel_styles.h
 * @brief The user-selectable panel styles. One table maps the Panel Style setting to the corner
 * radius the grid engine frames a cell with. Everything else about the panel is untouched, so the
 * header keeps its label block, checker and divider whichever style is picked.
 *
 * The style never moves the body. HEADER_H and the body rect are the same either way, so every
 * module draws at the same offsets.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup mosaic_draw
 * @{
 */

/// How many panel styles the picker offers. Feeds the settings enum_count and the Clay select
#define PANEL_STYLE_COUNT 2

/**
 * @brief The corner radius a panel style frames its cells with.
 *
 * @param choice The Panel Style setting value. Out of range falls back to the default (0).
 * @return The radius in pixels. 0 draws a plain square rect.
 */
uint8_t panel_style_radius(uint8_t choice);

/** @} */
