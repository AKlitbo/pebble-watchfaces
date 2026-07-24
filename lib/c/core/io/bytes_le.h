/**
 * @file bytes_le.h
 * @brief Little-endian byte readers shared by the wire-decoding stores.
 *
 * Each byte is read on its own and shifted into place rather than casting the pointer to a wider
 * type, so the value comes out the same whatever the compiler believes about alignment or byte
 * order. The casts are what carry the sign: the phone packs a temperature and a percent as signed
 * values, and reading them unsigned turns a small negative into a large positive.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/** @brief Reads a little-endian signed 16-bit value out of a byte pair. */
static inline int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)(p[0] | (p[1] << 8));
}

/** @brief Reads a little-endian signed 32-bit value out of four bytes. */
static inline int32_t read_i32_le(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

/** @} */
