/**
 * @file moon.h
 * @brief Pure moon-phase math you can test off the watch. It counts from a known new moon
 * using the average length of a moon cycle, so it needs nothing but a UTC timestamp.
 *
 * @ingroup lib_core
 */
#pragma once
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

#define MOON_SYNODIC_SEC 2551443   ///< Average length of one moon cycle in seconds (about 29.53 days)
#define MOON_EPOCH_UTC   947182440 ///< A known new moon to count from: 2000-01-06 18:14 UTC

/**
 * @brief How far into the current moon cycle we are, in seconds.
 *
 * @param utc The time to read, as a UTC timestamp.
 * @return Seconds since the last new moon (0 means new moon, up to MOON_SYNODIC_SEC).
 */
int32_t moon_age_sec(time_t utc);

/**
 * @brief Picks one of @p count evenly spaced phase icons. It wraps back round at the end.
 *
 * @param utc The time to read, as a UTC timestamp.
 * @param count How many phase icons there are.
 * @return The icon to show (0 means new moon).
 */
int moon_glyph_index(time_t utc, int count);

/**
 * @brief How lit up the moon looks, from 0 to 100. This is a rough straight-line guess, so it
 * reads 0 at new moon and 100 at full.
 *
 * @param utc The time to read, as a UTC timestamp.
 * @return The lit fraction from 0 to 100.
 */
int moon_illumination_pct(time_t utc);

/**
 * @brief Short name for the current phase (one of the eight main phases).
 *
 * @param utc The time to read, as a UTC timestamp.
 * @return The phase name.
 */
const char *moon_phase_name(time_t utc);

/**
 * @brief How many days until the next full or new moon, rounded to the nearest whole day.
 *
 * @param utc The time to read, as a UTC timestamp.
 * @param to_full True counts to the next full moon, false to the next new moon.
 * @return The days to wait (0 means it is about now, up to 30).
 */
int moon_days_to_phase(time_t utc, bool to_full);

/** @} */
