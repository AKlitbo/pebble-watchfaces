/**
 * @file number_format.h
 * @brief Pure number formatting (no SDK)
 *
 * @ingroup lib
 */
#pragma once
#include <stddef.h>

/** @addtogroup lib @{ */

/**
 * @brief Format an integer with apostrophe thousands separators (e.g. 20358 -> "20'358").
 *
 * Negative values keep their leading sign. The Swiss-style apostrophe matches the
 * Modular mockup's readouts.
 *
 * @param buffer Output buffer.
 * @param size Buffer size.
 * @param value The value to format.
 */
void number_group(char *buffer, size_t size, int value);

/** @} */
