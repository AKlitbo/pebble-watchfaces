/**
 * @file appmessage.h
 * @brief AppMessage transport: decodes the inbox and notifies the app via handlers
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

// per-channel handler types. a consumer (a store or a face) registers only the channels
// it cares about so the transport stays decoupled from who reads the data
typedef void (*WeatherHandler)(const char *temp, const char *condition);
typedef void (*CoordsHandler)(const char *lat, const char *lon);
typedef void (*SettingsChangedHandler)(bool time_or_date_changed);
// extra weather (humidity plus wind plus sunrise/sunset). absent values arrive as -1 / "".
typedef void (*WeatherExtraHandler)(int humidity, int wind_kmh, const char *wind_dir,
                                    const char *sunrise, const char *sunset);
// daily forecast (uv plus today's high/low plus precip). absent values arrive as INT_MIN so a
// real negative temp is not read as no-data.
typedef void (*WeatherForecastHandler)(int uv, int temp_max, int temp_min, int precip_chance);
typedef void (*LocationNameHandler)(const char *name);

void appmessage_on_weather(WeatherHandler cb);                   /**< temp + condition */
void appmessage_on_coords(CoordsHandler cb);                     /**< lat + lon */
void appmessage_on_settings_changed(SettingsChangedHandler cb);  /**< a setting changed on the phone */
void appmessage_on_weather_extra(WeatherExtraHandler cb);        /**< humidity/wind/sun */
void appmessage_on_weather_forecast(WeatherForecastHandler cb);  /**< uv/hi-lo/precip */
void appmessage_on_location_name(LocationNameHandler cb);        /**< location name */

/**
 * @brief Register the SDK callbacks and open the inbox/outbox.
 *
 * Call after the channel handlers are installed.
 */
void appmessage_open(void);

/**
 * @brief Asks the phone for a fresh weather reading. The weather store calls this on its
 * own poll timer.
 */
void appmessage_request_weather(void);

/** @} */
