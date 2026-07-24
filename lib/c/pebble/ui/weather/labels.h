/**
 * @file labels.h
 * @brief Condition-string to display-label lookup
 *
 * Two widths: a compact label for tiny faces / small slots and a full readable
 * label for roomy faces. Both accept the wire token (including the "_NIGHT"
 * forms) and return a string literal, so callers never strip the night marker.
 *
 * @ingroup lib_ui
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_ui
 * @{
 */

/**
 * @brief Compact display label for a condition token (e.g. "PCLDY").
 *
 * @param condition The condition token string.
 * @return The short label, or the fallback ("UNKNOWN") when unrecognized.
 */
const char *wx_label_short(const char *condition);

/**
 * @brief Full readable display label for a condition token (e.g. "Partly Cloudy").
 *
 * @param condition The condition token string.
 * @return The long label, or the fallback ("Unknown") when unrecognized.
 */
const char *wx_label_long(const char *condition);

/** @} */
