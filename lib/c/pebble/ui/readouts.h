/**
 * @file readouts.h
 * @brief Text formatters shared by the faces: pull a value from a store, honour the
 * relevant setting, and format it into a buffer. Each matches the engine's text-slot
 * signature, so a slot binds straight to one (e.g. `{ .zone = &s_zones[ZONE_TIME],
 * .text = readout_time }`).
 *
 * A face only binds the ones it shows, so the slots it leaves out cost it nothing on
 * screen.
 *
 * @ingroup lib_ui
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_ui
 * @{
 */

/**
 * @brief Format the clock per the Time Format setting: 24h, 12h, system, or .beats.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_time(char *out, size_t n);

/**
 * @brief Format the AM/PM marker, or empty on a 24-hour or .beats clock.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_meridiem(char *out, size_t n);

/**
 * @brief Format the date in the chosen format, upper-cased.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_date(char *out, size_t n);

/**
 * @brief Format the heart rate, or "--" when there is no reading.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_hr(char *out, size_t n);

/**
 * @brief Format the step count, or the distance walked per the Steps Mode setting.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_steps(char *out, size_t n);

/**
 * @brief Format the temperature with its unit letter, or "--" when there is no reading.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_weather_temp(char *out, size_t n);

/**
 * @brief Format the condition word, or "--" when there is none.
 *
 * The night marker is trimmed, so "CLEAR_NIGHT" reads as "CLEAR". The glyph still uses
 * the full token.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_weather_cond(char *out, size_t n);

/**
 * @brief Format the latitude string, or "--" when there is no fix.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_lat(char *out, size_t n);

/**
 * @brief Format the longitude string, or "--" when there is no fix.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
void readout_lon(char *out, size_t n);

/** @} */
