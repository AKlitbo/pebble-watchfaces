/**
 * @file wind_dir.h
 * @brief The compass direction a wind is blowing from, as a bearing and as a sideways component.
 *
 * The wire carries a 16-point abbreviation like "NW" or "SSE". That is fine to print and no use
 * for arithmetic, so this turns it into degrees, and degrees into the part of the wind that runs
 * across a view seen from the side.
 *
 * Pure arithmetic on a string, so there is no watch behind it. Reading the store is the caller's
 * job. An unknown or empty direction answers "no data" rather than guessing north.
 *
 * See units/wind.h for the other half of a wind reading, which converts its speed.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief The bearing a 16-point compass abbreviation stands for.
 *
 * Meteorological convention: the bearing is where the wind is coming *from*, so "N" is 0 and a
 * northerly blows towards the south.
 *
 * @param dir The abbreviation, e.g. "N", "ESE", "NW". Case insensitive.
 * @return Degrees from 0 to 337, or -1 when the string is not one of the sixteen points.
 */
int wind_bearing(const char *dir);

/**
 * @brief The east-west component of a wind, as a signed proportion.
 *
 * Anything drawn from the side only shows the part of a wind that runs across the view: one
 * blowing straight towards or away from the viewer has no sideways component at all, and one
 * blowing along the view has the whole of it. Positive is eastward, which is the way a wind out
 * of the west goes.
 *
 * @param bearing Degrees, or -1 for no data.
 * @return -100 (full westward) to 100 (full eastward), or 0 when there is no reading.
 */
int wind_lean(int bearing);

/** @} */
