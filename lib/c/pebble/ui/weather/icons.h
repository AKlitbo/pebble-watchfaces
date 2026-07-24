/**
 * @file icons.h
 * @brief Condition-string to weather-icon resource lookup
 *
 * @ingroup lib_ui
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_ui
 * @{
 */

/// High bit set on an hourly forecast code to mark a night hour, so the column loads the night
/// icon. Mirrors conditions.js FORECAST_NIGHT_BIT. Real condition numbers are 0 to 11 so the bit
/// is free, and the unknown value (255) still lands on WI_NA
#define WX_FORECAST_NIGHT_BIT 0x80

/**
 * @brief Look up the weather-icon resource for a condition abbreviation.
 *
 * @param condition The condition abbreviation string.
 * @return The corresponding resource ID.
 */
uint32_t wx_resource_for(const char *condition);

/**
 * @brief Look up the weather-icon resource for a forecast condition code.
 *
 * The forecast row carries each condition as a compact byte (its number in the
 * shared list) rather than a token string. An unknown code lands on the
 * WI_NA icon.
 *
 * @param code The condition code, or UNKNOWN_CODE (255) for a mystery sky.
 * @return The corresponding resource ID.
 */
uint32_t wx_resource_for_forecast_code(uint8_t code);

/** @} */
