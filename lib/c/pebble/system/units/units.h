/**
 * @file units.h
 * @brief Unit conversion + distance formatting helpers
 *
 * @ingroup lib_system
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_system
 * @{
 */

/**
 * @brief Current swatch internet time in .beats.
 *
 * @return Swatch internet time in .beats (0..999).
 */
int units_swatch_beats(void);

/**
 * @brief Ms until the next .beats boundary.
 *
 * @return Milliseconds remaining until next boundary.
 */
uint32_t units_ms_until_next_beat(void);

/**
 * @brief Format meters as "N.N MI" or "N.N KM".
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void units_format_distance(char *buffer, size_t size, int meters, bool miles);

/**
 * @brief Format meters as "N.N" (no unit suffix), rounded to a tenth.
 *
 * Pair with units_distance_unit() when the unit wants its own small-font slot.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void units_format_distance_value(char *buffer, size_t size, int meters, bool miles);

/**
 * @brief Unit label for the current distance mode ("MI" or "KM").
 *
 * @param miles Miles if true, otherwise km.
 * @return Static unit string.
 */
const char *units_distance_unit(bool miles);

/** @} */
