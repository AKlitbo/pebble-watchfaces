/**
 * @file duration.h
 * @brief A minute count turned into a short human duration, no SDK.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stddef.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Format minutes as a clock-style "H:MM", e.g. 445 -> "7:25".
 *
 * The minutes are zero-padded, so seven hours and five minutes reads "7:05" and not "7:5". A
 * negative count is nothing and reads "0:00".
 *
 * @param out Output buffer.
 * @param n Buffer size.
 * @param minutes The duration in minutes.
 */
void duration_hm(char *out, size_t n, int minutes);

/**
 * @brief Format minutes as "Xh Ym", dropping the hours once it is under one ("45m").
 *
 * For a countdown, where the hours are only worth showing once there is at least one. A negative
 * count is nothing and reads "0m".
 *
 * @param out Output buffer.
 * @param n Buffer size.
 * @param minutes The duration in minutes.
 */
void duration_hm_compact(char *out, size_t n, int minutes);

/** @} */
