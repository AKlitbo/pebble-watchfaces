/**
 * @file wire_read.h
 * @brief Reads the values the phone packs into a message.
 *
 * The phone is free to send an integer one, two, or four bytes wide, and it says which by the
 * length it puts beside the value rather than by the type. The values sit back to back with
 * nothing between them, so a reader that takes the type's word for the width and grabs four bytes
 * off a one-byte value walks straight into whatever is packed behind it. That is what these are
 * for: the width decides how many bytes get read, and anything else is refused.
 *
 * Header only on purpose. There is one caller on the watch (tuple_read.c) and the bodies are
 * smaller than a call would be, so they fold into it and nothing is spent on getting there.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "io/bytes_le.h"

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Reads a little-endian integer of exactly @p length bytes.
 *
 * @param is_signed Whether the value is signed. Decides only what the top bit means.
 * @param length How many bytes the value occupies. 1, 2, and 4 are the widths a sender may use,
 *   and anything else is refused rather than guessed at.
 * @param value The first byte of the value.
 * @param fallback Returned when the width is not one a sender may use, or there is no value.
 * @return The value, or @p fallback.
 */
static inline int32_t wire_int_or(bool is_signed, uint16_t length, const uint8_t *value, int32_t fallback)
{
    if (!value)
    {
        return fallback;
    }

    switch (length)
    {
        // a byte read signed is -128 to 127 and unsigned is 0 to 255
        // that is the only thing the signedness decides at this width
        case 1: return is_signed ? (int32_t)(int8_t)value[0] : (int32_t)value[0];

        // read the pair as signed and take the sign back off when it is not wanted
        // so the unsigned half lands on 0 to 65535 rather than going negative at the top
        case 2: return is_signed ? (int32_t)read_i16_le(value)
                                 : (int32_t)(uint16_t)read_i16_le(value);

        // four bytes fill the whole width either way so the bits are the same
        // and only the caller's reading of the top one differs
        case 4: return read_i32_le(value);

        default: return fallback;
    }
}

/**
 * @brief Whether a string of @p length bytes carries its terminator inside itself.
 *
 * A sender is supposed to include the terminator and nothing re-checks that it did. A string
 * without one is not a string: reading it runs on past the end looking for a zero byte that
 * belongs to something else, or to nothing at all.
 *
 * @param s The first byte of the string.
 * @param length How many bytes the string occupies, terminator included.
 * @return Whether it is safe to read as a C string.
 */
static inline bool wire_cstring_terminated(const char *s, uint16_t length)
{
    return s && length > 0 && s[length - 1] == '\0';
}

/** @} */
