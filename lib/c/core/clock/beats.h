/**
 * @file beats.h
 * @brief Pure swatch .beats math, operates on ms since Biel Mean Time (UTC+1) midnight.
 * Also holds the token a date format uses to ask for a .beats reading.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

// shared time constants for the .beats clock and the BMT conversion
#define MS_PER_BEAT      86400     ///< Milliseconds in one .beat which is one thousandth of a day
#define MS_PER_DAY       86400000  ///< Milliseconds in a full day
#define MS_PER_SEC       1000      ///< Milliseconds in one second
#define SECS_PER_DAY     86400     ///< Seconds in a full day
#define SECS_PER_HOUR    3600      ///< Seconds in one hour
#define SECS_PER_MIN     60        ///< Seconds in one minute
#define BMT_UTC_OFFSET_S 3600      ///< Biel Mean Time is UTC plus 1 hour, in seconds

/**
 * @brief The marker a date format carries where a .beats reading should go.
 *
 * Braces rather than a percent, because the date format is handed to strftime first and every
 * percent belongs to it (%B is already the month name). strftime copies anything else through
 * untouched, so the token survives the pass and can be filled in afterwards.
 *
 * Three characters wide on purpose. A reading is always three digits, so swapping one for the
 * other is a same-length overwrite that cannot grow the string past the buffer it arrived in.
 */
#define BEATS_TOKEN     "{B}"
#define BEATS_TOKEN_LEN 3          ///< Length of BEATS_TOKEN, which matches a three digit reading

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Calculates swatch internet time in .beats (0..999) for a given ms.
 *
 * @param ms_into_bmt_day Milliseconds elapsed since Biel Mean Time (UTC+1) midnight.
 * @return The current time in .beats (0 to 999).
 */
int beats_from_ms(int32_t ms_into_bmt_day);

/**
 * @brief Calculates milliseconds until the next .beats boundary.
 *
 * A single beat is exactly 86,400 milliseconds. This is useful for
 * scheduling timers to trigger exactly when the beat rolls over.
 *
 * @param ms_into_bmt_day Milliseconds elapsed since Biel Mean Time (UTC+1) midnight.
 * @return Milliseconds remaining until the next beat boundary (1 to 86400).
 */
uint32_t ms_until_next_beat(int32_t ms_into_bmt_day);

/**
 * @brief Milliseconds into the Biel Mean Time day, from a UTC wall clock.
 *
 * Takes the clock already broken apart rather than a time_t, because breaking one apart means
 * gmtime, and on the watch gmtime and struct tm belong to the SDK rather than to time.h. Reaching
 * for either here would drag the whole SDK into a file whose point is not needing it. The caller
 * holds the clock and this holds the sum.
 *
 * The two above want ms rather than seconds because a beat boundary falls mid-second, so flooring
 * to whole seconds reads a beat behind at exactly the moment a timer fires to redraw.
 *
 * @param hour The UTC hour (0 to 23).
 * @param minute The UTC minute (0 to 59).
 * @param second The UTC second (0 to 59).
 * @param ms The milliseconds within that second.
 * @return Milliseconds into the current BMT day (0 to 86,399,999).
 */
int32_t beats_ms_from_hms(int hour, int minute, int second, uint16_t ms);

/**
 * @brief Whether a date format asks for a .beats reading.
 *
 * The face asks this to decide whether to run the .beats ticker. A beat is 86.4 seconds and the
 * date line otherwise redraws on the minute, so without the ticker the reading sits stale and
 * reports every rollover up to a minute late.
 *
 * @param format The date format string, or NULL.
 * @return true if the format carries BEATS_TOKEN.
 */
bool beats_has_token(const char *format);

/**
 * @brief Overwrite every BEATS_TOKEN in a string with a three digit .beats reading.
 *
 * Works in place because the token and the reading are the same width, so the string neither
 * grows nor needs its tail shifted. A reading outside 0 to 999 is clamped rather than written,
 * which is what keeps that width promise true.
 *
 * @param text The string to fill in, NUL terminated. Left alone if NULL or tokenless.
 * @param beats The reading to write, 0 to 999.
 */
void beats_expand_token(char *text, int beats);

/** @} */
