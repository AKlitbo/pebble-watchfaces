/**
 * @file solar.h
 * @brief How the day's light is going, worked out from three plain clock readings.
 *
 * Sunrise, sunset, and now all arrive as minutes past midnight, so this is pure arithmetic with no
 * watch behind it. Reading the phone's strings and the clock is the caller's job. Handing over the
 * three numbers is the seam. Any reading of -1 means the caller had nothing, and every function
 * here answers "no data" for it rather than doing maths on a missing time.
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
 * @brief How far through the daylight it is, from 0 at sunrise to 100 at sunset.
 *
 * @param rise Sunrise, minutes past midnight, or -1 for no data.
 * @param set Sunset, minutes past midnight, or -1 for no data.
 * @param now The clock, minutes past midnight, or -1 for no data.
 * @return The progress from 0 to 100, or -1 when it is night or a reading is missing, so the
 *   caller can hide the sun rather than pin it to an end.
 */
int solar_day_progress(int rise, int set, int now);

/**
 * @brief How far through the night it is, from 0 at sunset to 100 at the next sunrise.
 *
 * The night straddles midnight, so the span runs from sunset up over midnight to tomorrow's
 * sunrise, and a "now" in the small hours is measured as if it sat after today's sunset.
 *
 * @param rise Sunrise, minutes past midnight, or -1 for no data.
 * @param set Sunset, minutes past midnight, or -1 for no data.
 * @param now The clock, minutes past midnight, or -1 for no data.
 * @return The progress from 0 to 100, or -1 when it is day or a reading is missing, so the caller
 *   knows to draw the sun instead of the moon.
 */
int solar_night_progress(int rise, int set, int now);

/**
 * @brief Minutes until the next sun event, and which one it is.
 *
 * Sunrise while it is dark, sunset while the sun is up, and tomorrow's sunrise once it has set,
 * wrapping midnight the same way the night progress does.
 *
 * @param rise Sunrise, minutes past midnight, or -1 for no data.
 * @param set Sunset, minutes past midnight, or -1 for no data.
 * @param now The clock, minutes past midnight, or -1 for no data.
 * @param is_sunrise Receives whether the next event is a sunrise. Untouched when there is no data.
 * @return Minutes until the event, or -1 when a reading is missing.
 */
int solar_next_event(int rise, int set, int now, bool *is_sunrise);

/** @} */
