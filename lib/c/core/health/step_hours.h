/**
 * @file step_hours.h
 * @brief The hour arithmetic behind the step chart's buckets.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/** How many hourly buckets a day holds. */
#define STEP_HOURS_PER_DAY 24

/**
 * @brief How many whole hours have settled, meaning they can never gain another step.
 *
 * The hour in progress never counts. Nor does the hour just gone while the clock is still on its
 * first minute: the watch writes a minute's record at the top of the minute after it, and it does
 * that below the watchface in priority, so on the turn an hour rolls over its last record is
 * usually not there to read yet.
 *
 * @param cur_hour The hour now, 0 to 23.
 * @param cur_min The minute within that hour, 0 to 59.
 * @return How many hours have settled, so buckets below it are final.
 */
int step_hours_settled(int cur_hour, int cur_min);

/**
 * @brief Which hourly bucket a minute record belongs in.
 *
 * A batched read hands back records from several hours at once, and the watch shuffles them
 * towards the front of the array when the window opens on a gap, so where a record sits in that
 * array says nothing about where it sits in the day. Only its own time does.
 *
 * @param seconds_into_day How far past midnight the record sits.
 * @return The bucket index, or -1 when the record falls outside the day.
 */
int step_hours_bucket(int seconds_into_day);

/** @} */
