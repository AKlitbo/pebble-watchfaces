/**
 * @file time_progress_calc.h
 * @brief Pure maths for the progress panels: how far through the year, day, hour and week the
 * wall clock is, as a whole percent. It takes the same fields a struct tm carries, one at a
 * time, so it needs no Pebble services and can be tested off the watch.
 *
 * Every answer runs on the wall clock, so a day is always 24 hours. On the one day a year the
 * clocks change, the day bar therefore reads a little ahead or behind rather than jumping.
 *
 * @ingroup mosaic_mod_time
 */
#pragma once

/**
 * @addtogroup mosaic_mod_time
 * @{
 */

/**
 * @brief How far through the current hour it is.
 *
 * @param min The minute, 0 to 59.
 * @param sec The second, 0 to 59.
 * @return Whole percent from 0 to 99.
 */
int progress_hour_pct(int min, int sec);

/**
 * @brief How far through the current day it is.
 *
 * @param hour The hour, 0 to 23.
 * @param min The minute, 0 to 59.
 * @param sec The second, 0 to 59.
 * @return Whole percent from 0 to 99.
 */
int progress_day_pct(int hour, int min, int sec);

/**
 * @brief How far through the current week it is, counting from the day the week starts on.
 *
 * @param wday Today's weekday, 0 Sunday to 6 Saturday.
 * @param week_start The weekday the week begins on, 0 Sunday to 6 Saturday.
 * @param hour The hour, 0 to 23.
 * @param min The minute, 0 to 59.
 * @param sec The second, 0 to 59.
 * @return Whole percent from 0 to 99.
 */
int progress_week_pct(int wday, int week_start, int hour, int min, int sec);

/**
 * @brief How far through the current year it is, with February getting its leap day.
 *
 * @param year The full year (2026, not 126).
 * @param yday The day of the year counting January 1st as 0 (like tm_yday).
 * @param hour The hour, 0 to 23.
 * @param min The minute, 0 to 59.
 * @param sec The second, 0 to 59.
 * @return Whole percent from 0 to 99.
 */
int progress_year_pct(int year, int yday, int hour, int min, int sec);

/**
 * @brief How many days the week has run, counting the first day as day 1.
 *
 * @param wday Today's weekday, 0 Sunday to 6 Saturday.
 * @param week_start The weekday the week begins on, 0 Sunday to 6 Saturday.
 * @return The day number from 1 to 7.
 */
int progress_week_day(int wday, int week_start);

/** @} */
