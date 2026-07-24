/**
 * @file clockstr.h
 * @brief Reads the "HH:MM" clock strings the phone sends.
 *
 * Sunrise, sunset and the like arrive as text rather than numbers, so this is the seam between a
 * string the phone chose and an hour the face can draw. The phone is not trusted to send a good
 * one, so anything that does not read as a real time is refused rather than guessed at.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Reads "HH:MM" into an hour and a minute.
 *
 * The hour is one or two digits and the minute is exactly the two after the colon. Anything past
 * those is ignored, so "06:30" and "06:30:00" both read as half six.
 *
 * @param src The string to read. Must not be NULL.
 * @param hour_out Receives the hour (0-23) on success, untouched otherwise.
 * @param minute_out Receives the minute (0-59) on success, untouched otherwise.
 * @return Whether it read as a real time.
 */
bool clockstr_parse(const char *src, int *hour_out, int *minute_out);

/**
 * @brief Reads "HH:MM" straight into minutes past midnight.
 *
 * The same parse as clockstr_parse, folded into the one number a day-progress or countdown
 * actually wants. "06:30" is 390.
 *
 * @param src The string to read. Must not be NULL.
 * @return Minutes past midnight (0-1439), or -1 when it does not read as a real time.
 */
int clockstr_minutes(const char *src);

/** @} */
