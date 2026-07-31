/**
 * @file battery.h
 * @brief The battery gauge's warning colours.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup family-sketchbook
 * @{
 */

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
GColor sketchbook_battery_fill(uint8_t theme, GColor ink, int level);

/** @} */
