/**
 * @file weekday.h
 * @brief Short weekday names shared by the panels that label a day.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief The two-letter short name for a weekday, indexed like tm_wday and the phone's
 * base_weekday (0 = Sunday through 6 = Saturday).
 *
 * @param wday The weekday, 0 (Sunday) through 6 (Saturday).
 * @return The short name ("SU".."SA"), or "" when the index is out of range.
 */
const char *weekday_short(int wday);

/** @} */
