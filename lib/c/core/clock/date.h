/**
 * @file date.h
 * @brief Pure calendar math you can test off the watch. Week numbers, day counts, and month
 * shapes worked out from plain numbers, so it needs no Pebble services and no struct tm.
 *
 * The inputs are the same fields a struct tm carries, handed in one at a time so this stays
 * pure core code. Year is the full year (2026). Month is 0 January to 11 December. Weekday is
 * 0 Sunday to 6 Saturday. Day of the year (yday) counts January 1st as 0, like tm_yday.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief The ISO 8601 week number, which always counts weeks from Monday.
 *
 * A week near a year boundary can belong to the neighbouring year, so the number this returns
 * is not always counted within the year you passed in. Pair it with date_iso_week_year to find
 * out which year it belongs to, and total it against that year rather than this one.
 *
 * @param year The full year (2026, not 126).
 * @param yday The day of the year counting January 1st as 0 (like tm_yday).
 * @param wday The weekday, 0 Sunday to 6 Saturday.
 * @return The week number from 1 to 53.
 */
int date_iso_week(int year, int yday, int wday);

/**
 * @brief The ISO 8601 year the week number belongs to, which is not always the calendar year.
 *
 * Early January can fall in the previous ISO year and late December in the next one. Feeding
 * this to date_iso_weeks_in_year gives the total that matches what date_iso_week returned, so
 * a "week of total" readout can't end up with a number bigger than its own total.
 *
 * @param year The full year (2026, not 126).
 * @param yday The day of the year counting January 1st as 0 (like tm_yday).
 * @param wday The weekday, 0 Sunday to 6 Saturday.
 * @return The ISO year, either year, year - 1, or year + 1.
 */
int date_iso_week_year(int year, int yday, int wday);

/**
 * @brief How many ISO 8601 weeks a year holds, either 52 or 53.
 *
 * @param year The full year (2026, not 126).
 * @return The week count, 52 or 53.
 */
int date_iso_weeks_in_year(int year);

/**
 * @brief Which day of the year it is, counting January 1st as day 1.
 *
 * @param yday The day of the year counting January 1st as 0 (like tm_yday).
 * @return The day number from 1 to 366.
 */
int date_day_of_year(int yday);

/**
 * @brief How many days are left before the year ends, so December 31st reads 0.
 *
 * @param year The full year (2026, not 126).
 * @param yday The day of the year counting January 1st as 0 (like tm_yday).
 * @return The days remaining from 0 to 365.
 */
int date_days_left_in_year(int year, int yday);

/**
 * @brief How many days a month has, with February getting its leap day.
 *
 * @param year The full year (2026, not 126).
 * @param mon0 The month indexed like tm_mon (0 January to 11 December).
 * @return The day count, or 0 for a month index that does not exist.
 */
int date_days_in_month(int year, int mon0);

/**
 * @brief The weekday the 1st of the month lands on, walked back from today's weekday.
 *
 * @param wday Today's weekday, 0 Sunday to 6 Saturday.
 * @param mday Today's day of the month, 1 to 31.
 * @return The weekday of the 1st, 0 Sunday to 6 Saturday.
 */
int date_first_wday(int wday, int mday);

/** @} */
