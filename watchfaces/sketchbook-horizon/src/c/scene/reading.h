/**
 * @file reading.h
 * @brief The temperature a round scene carries on a holder of its own, a sign or a buoy.
 *
 * Every round scene but Shoreline sizes its holder to the reading, so a long one never runs off
 * its edges, and letters it with draw/letter.h. Shoreline's pennant is a fixed width, and it
 * measures and letters its reading on its own.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Write out the temperature for a holder, and say how wide it letters.
 *
 * With no temperature this returns false and leaves width alone, and the scene leaves its holder
 * off rather than lettering it "--".
 *
 * @param out Where the temperature text goes.
 * @param n How big out is.
 * @param width Where its lettered width goes.
 * @return True when there is a temperature to show.
 */
bool reading_temp(char *out, size_t n, int *width);

/** @} */
