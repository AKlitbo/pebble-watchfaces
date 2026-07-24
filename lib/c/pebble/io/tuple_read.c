/**
 * @file tuple_read.c
 * @brief Reads a value out of an inbound message tuple.
 */
#include "io/tuple_read.h"

#include "wire/wire_read.h"

int32_t tuple_int_or(const Tuple *tuple, int32_t fallback)
{
    if (!tuple || (tuple->type != TUPLE_INT && tuple->type != TUPLE_UINT))
    {
        return fallback;
    }

    return wire_int_or(tuple->type == TUPLE_INT, tuple->length,
                       (const uint8_t *)tuple->value, fallback);
}

const char *tuple_str_or(const Tuple *tuple, const char *fallback)
{
    if (!tuple || tuple->type != TUPLE_CSTRING)
    {
        return fallback;
    }

    // take the address through a char pointer so gcc does not flag the zero-length cstring[]
    // flexible array member subscript
    const char *cstring = tuple->value->cstring;
    return wire_cstring_terminated(cstring, tuple->length) ? cstring : fallback;
}
