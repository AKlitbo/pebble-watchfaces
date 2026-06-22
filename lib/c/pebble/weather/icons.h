/**
 * @file icons.h
 * @brief condition-string to weather-icon resource lookup
 */
#pragma once
#include <pebble.h>

/**
 * @brief Look up the weather-icon resource for a condition abbreviation.
 *
 * @param condition The condition abbreviation string.
 * @return The corresponding resource ID.
 */
uint32_t wx_resource_for(const char *condition);
