/**
 * @file tuple_read.h
 * @brief Reads a value out of an inbound message tuple.
 *
 * The SDK hands the phone's message over as tuples, each one a key, a type, a length, and the
 * bytes. This unwraps that shape and hands the bytes to wire_read.h, which owns the actual
 * reading. The split is the point: the tuple is the SDK's business and the width is not, and the
 * width is where the reading goes wrong.
 *
 * Everything that reads an inbound value goes through here. A decoder that unwraps a tuple itself
 * is trusting the type to tell it the width, which the type does not do.
 *
 * @ingroup lib_io
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_io
 * @{
 */

/**
 * @brief Reads an int tuple, whatever width the phone sent it at.
 *
 * A sender may pick TUPLE_INT or TUPLE_UINT for an integer, so both are accepted and the choice
 * only decides what the top bit means.
 *
 * @param tuple The tuple, or NULL when the key was absent.
 * @param fallback Returned when the tuple is absent, is not an int, or carries a width no sender
 *   may use.
 * @return The value, or @p fallback.
 */
int32_t tuple_int_or(const Tuple *tuple, int32_t fallback);

/**
 * @brief Reads a cstring tuple, or the fallback when it is not one that can be read.
 *
 * @param tuple The tuple, or NULL when the key was absent.
 * @param fallback Returned when the tuple is absent, is not a cstring, or carries no terminator
 *   inside its own length.
 * @return The string, or @p fallback.
 */
const char *tuple_str_or(const Tuple *tuple, const char *fallback);

/** @} */
