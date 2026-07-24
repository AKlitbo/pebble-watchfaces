/**
 * @file astro.h
 * @brief Pure astronomy math you can test off the watch. Right now just the Julian Date, worked
 * out from a UTC timestamp with plain integer math, so it needs no Pebble services and no floats.
 *
 * @ingroup lib_core
 */
#pragma once
#include <time.h>
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief The Julian Date multiplied by 100, so the whole part is the day and the last two digits
 * are hundredths. Kept scaled to stay in plain 32-bit integers.
 *
 * @param utc The time to read, as a UTC timestamp.
 * @return The Julian Date times 100 (e.g. 245154500 for JD 2451545.00).
 */
int32_t astro_jd_centi(time_t utc);

/** @} */
