/**
 * @file weather_solar.h
 * @brief The "Sun" module. One module, three looks depending on the slot it lands in: a
 * 1x2 countdown to the next sun event, a 2x2 sun-on-an-arc visual with the rise and set
 * times, and a 1x4 sunrise to sunset track with a marker at now.
 * @ingroup gridlock_mod_weather
 */
#pragma once
#include <pebble.h>
#include "engine/catalog.h"

/** @brief The sun panel the grid can drop in at 1x2, 2x2 or 1x4. */
extern const ModuleDef mod_weather_solar_def;
