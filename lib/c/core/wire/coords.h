/**
 * @file coords.h
 * @brief Telling a real fix from the phone's way of saying it has none.
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
 * @brief Whether a pair of coordinates is a place rather than the phone shrugging.
 *
 * The phone sends coordinates as text and says "no fix" by sending something that is not one: an
 * empty string, or a word like "NO LOCK". A coordinate always carries digits and those never do,
 * which is the whole of the test.
 *
 * Both halves have to hold up. Half a fix is not half a place, it is not a place, and keeping one
 * is how a longitude goes missing and stays missing.
 *
 * @param lat The latitude as the phone sent it.
 * @param lon The longitude as the phone sent it.
 * @return Whether the pair is worth keeping.
 */
bool coords_look_real(const char *lat, const char *lon);

/** @} */
