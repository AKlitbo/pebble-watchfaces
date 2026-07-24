/**
 * @file number_format.h
 * @brief Pure number and string formatting (no SDK)
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Format an integer with apostrophe thousands separators (e.g. 20358 -> "20'358").
 *
 * Negative values keep their leading sign. The Swiss-style apostrophe keeps big
 * counts readable on the small display.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param value The value to format.
 */
void number_group(char *buffer, size_t size, int value);

/**
 * @brief Format an integer, or "--" when there is no reading yet.
 *
 * A negative value is the no-data sentinel the readouts share, so this is the one place that turns
 * "I have nothing" into something to draw. The format takes exactly one int, so "%d" or "%d%%".
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param value The reading, or a negative number for no data yet.
 * @param fmt The printf format applied when the value is there.
 */
void fmt_int_or_dash(char *buffer, size_t size, int value, const char *fmt);

/**
 * @brief Format hundredths as a two-decimal number, e.g. 26174 -> "261.74".
 *
 * Prices and the like ride the wire as whole hundredths so they never touch a float. The sign is
 * written out front rather than left to the whole part, which is what makes -50 read as "-0.50"
 * instead of losing its minus to a whole part of zero.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param value The number times 100.
 */
void fmt_hundredths(char *buffer, size_t size, int value);

/**
 * @brief Format hundredths as a signed two-decimal percent, e.g. 125 -> "+1.25%".
 *
 * A gain takes a plus, a loss a minus, and flat takes neither. Same reason as fmt_hundredths for
 * writing the sign out front: a change under one percent has a whole part of zero and would lose
 * it otherwise.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param value The percent times 100, signed.
 */
void fmt_pct_signed(char *buffer, size_t size, int value);

/**
 * @brief Copy bytes into a fixed buffer, taking what fits and always terminating.
 *
 * For the length-prefixed text the phone sends, where the length is the phone's word and the room
 * is ours. What does not fit is dropped rather than written past the end.
 *
 * @param dst Where to write. Always ends terminated.
 * @param cap How much room dst has, terminator included. Must be at least 1: a buffer with no room
 *   for even a terminator has nothing this can do with it.
 * @param src The bytes to copy.
 * @param len How many bytes there are.
 */
void copy_bounded(char *dst, uint8_t cap, const uint8_t *src, uint8_t len);

/**
 * @brief Whether a byte range holds a string worth keeping.
 *
 * Worth keeping means it ends inside its own room, it says something, and it carries no control
 * bytes. Everything from 0x20 up counts as text, UTF-8 included, so a place name spelled with an
 * umlaut is text and not damage. Getting that wrong is how a saved city turns back into the
 * default one on the next boot.
 *
 * @param s The bytes to look at.
 * @param size How much room they have.
 * @return Whether it reads as a string.
 */
bool cstring_is_clean(const char *s, uint16_t size);

/** @} */
