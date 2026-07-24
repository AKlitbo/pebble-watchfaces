/**
 * @file distance.h
 * @brief Pure distance conversions + formatting (no SDK)
 *
 * @ingroup lib_core
 */
#pragma once
#include <stddef.h>
#include <stdbool.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Unit label for the current distance mode ("MI" or "KM").
 *
 * @param miles Miles if true, otherwise km.
 * @return Static unit string.
 */
const char *distance_unit(bool miles);

/**
 * @brief Format meters as "N.N" (rounded to tenth), no unit suffix.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void distance_format_value(char *buffer, size_t size, int meters, bool miles);

/**
 * @brief Format meters as "N.N MI" or "N.N KM" (rounded to tenth).
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param meters Distance in meters.
 * @param miles Format as miles if true, otherwise km.
 */
void distance_format(char *buffer, size_t size, int meters, bool miles);

/** @} */
