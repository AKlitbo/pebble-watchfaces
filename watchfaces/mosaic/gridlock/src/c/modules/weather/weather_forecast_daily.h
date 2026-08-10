/**
 * @file weather_forecast_daily.h
 * @brief The 7-day forecast row. A full-width strip of upcoming days, each with the weekday,
 * a sky icon and the day's high.
 * @ingroup gridlock_mod_weather
 */
#pragma once
#include <pebble.h>
#include "mosaic/engine/catalog.h"

/** @brief The 7-day forecast row the grid can drop in. */
extern const ModuleDef mod_weather_forecast_daily_def;
