/**
 * @file readouts.h
 * @brief Text formatters shared by the vitals faces: pull a value from a store, honour the
 * relevant setting, and format it into a buffer. Each matches the engine's text-slot
 * signature, so a face binds a slot straight to one (e.g. `{ .zone = &Z_TIME, .text =
 * readout_time }`).
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

void readout_time(char *out, size_t n);     // clock: 24h / 12h / system / .beats
void readout_meridiem(char *out, size_t n); // AM/PM on a 12-hour clock else empty
void readout_date(char *out, size_t n);     // date in the chosen format upper-cased
void readout_hr(char *out, size_t n);       // heart rate or "--"
void readout_steps(char *out, size_t n);    // step count or distance per the steps mode
void readout_weather_temp(char *out, size_t n); // temperature or "--"
void readout_weather_cond(char *out, size_t n); // condition word (night marker trimmed) or "--"
void readout_lat(char *out, size_t n);      // latitude string or "--"
void readout_lon(char *out, size_t n);      // longitude string or "--"

/** @} */
