/**
 * @file tide.h
 * @brief The rise and fall of a semidiurnal tide, from a plain running minute count.
 *
 * Tides run on the moon rather than the sun, so they come round every 12h 25m and drift about
 * 50 minutes later each day. That drift is the whole character of a tide, and it is why this
 * takes a continuous minute count rather than a time of day: a clock that resets at midnight
 * would snap the water back every night.
 *
 * This is a rhythm, not a prediction. A real tide table needs the port, the coastline and the
 * lunar transit, none of which a watchface has. What it gives is water that moves at the right
 * speed and never sits still, which is what the eye is actually reading.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/// The lunar semidiurnal period, 12h 25m. Two of these make one tidal day
#define TIDE_PERIOD_MIN 745

/**
 * @brief How far in the tide is, from 0 at dead low to 100 at high water.
 *
 * Eased at both ends rather than run at a constant rate, because a real tide stands almost
 * still around the turn and moves fastest halfway through. Water creeping up the sand and then
 * pausing is the part that reads as tidal.
 *
 * @param minutes A running minute count, e.g. the epoch in minutes. Negative values are fine.
 * @return The level from 0 to 100.
 */
int tide_level(int32_t minutes);

/**
 * @brief Whether the tide is on its way in.
 *
 * @param minutes A running minute count, on the same base as tide_level.
 * @return True while it is flooding, false while it ebbs.
 */
bool tide_rising(int32_t minutes);

/** @} */
