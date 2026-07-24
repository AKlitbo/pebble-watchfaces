/**
 * @file appmessage.h
 * @brief AppMessage transport: decodes the inbox and notifies the app via handlers
 *
 * @ingroup lib_io
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_io
 * @{
 */

// per-channel handler types
// a store or a face registers only the channels it cares about
// so the appmessage pipeline stays separate from who reads the data
// temp is the reading in the user's unit (WEATHER_NO_TEMP when there is none)
// the pipeline does not know the unit label
// the face adds "°C" or "°F" from its own setting when it draws
typedef void (*WeatherHandler)(int temp, const char *condition);
typedef void (*CoordsHandler)(const char *lat, const char *lon);
typedef void (*SettingsChangedHandler)(bool time_or_date_changed);
// extra weather (humidity plus wind plus sunrise and sunset)
// missing values arrive as -1 or ""
typedef void (*WeatherExtraHandler)(int humidity, int wind_kmh, const char *wind_dir,
                                    const char *sunrise, const char *sunset);
// daily forecast (uv plus today's high and low plus rain chance)
// missing values arrive as INT_MIN so a real negative temp is not read as missing
typedef void (*WeatherForecastHandler)(int uv, int temp_max, int temp_min, int precip_chance);
// air readings (feels-like plus surface pressure plus dew point)
// missing values arrive as INT_MIN so a real negative reading is not read as missing
typedef void (*WeatherAirHandler)(int feels_like, int pressure, int dew_point);
// the forecast strips (hourly and 7 day) arrive as packed byte arrays
// the handler owns the wire format so appmessage just hands the raw bytes and their length across
typedef void (*WeatherForecastStripHandler)(const uint8_t *buf, uint16_t len);
typedef void (*LocationNameHandler)(const char *name);
// the watchlist strip arrives as a packed byte array too
// the handler owns the wire format
typedef void (*StockStripHandler)(const uint8_t *buf, uint16_t len);
// the agenda strip arrives as a packed byte array as well
// the store owns the wire format
typedef void (*CalendarStripHandler)(const uint8_t *buf, uint16_t len);
// the custom-colours string can be too big for one persist key
// so it rides its own handler instead of the settings table
// inbound the face splits it and stores it
// outbound the provider rebuilds it into the buffer appmessage hands over
// so nobody keeps a worst-case buffer alive between sends
typedef void (*CustomColorsHandler)(const char *combined);
typedef void (*CustomColorsProvider)(char *out, size_t n);

/// Worst case combined custom-colours string. Two persist-key-sized halves plus the bar and the NUL
#define APPMESSAGE_CUSTOM_COLORS_MAX 502

// fires once after every inbox message is fully handled
// so a store that took several channels in one push can save them together in one write
typedef void (*InboxCompleteHandler)(void);

void appmessage_on_weather(WeatherHandler cb);                   /**< Temperature and condition */
void appmessage_on_coords(CoordsHandler cb);                     /**< Latitude and longitude */
void appmessage_on_settings_changed(SettingsChangedHandler cb);  /**< A setting changed on the phone */
void appmessage_on_weather_extra(WeatherExtraHandler cb);        /**< Humidity, wind and sun times */
void appmessage_on_weather_forecast(WeatherForecastHandler cb);  /**< UV, today's high and low, rain chance */
void appmessage_on_weather_air(WeatherAirHandler cb);            /**< Feels-like, pressure and dew point */
void appmessage_on_weather_forecast_hourly(WeatherForecastStripHandler cb); /**< Packed hourly strip */
void appmessage_on_weather_forecast_daily(WeatherForecastStripHandler cb);  /**< Packed 7 day strip */
void appmessage_on_location_name(LocationNameHandler cb);        /**< Location name */
void appmessage_on_stock_strip(StockStripHandler cb);            /**< Packed watchlist strip */
void appmessage_on_calendar_strip(CalendarStripHandler cb);      /**< Packed agenda strip */
void appmessage_on_custom_colors(CustomColorsHandler cb);        /**< Inbound: splits and stores the combined string */
void appmessage_set_custom_colors_provider(CustomColorsProvider cb); /**< Outbound: rebuilds the combined string */
void appmessage_on_inbox_complete(InboxCompleteHandler cb);      /**< Fires once after a whole inbox is handled */

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

/**
 * @brief Asks the phone for fresh quotes. The stock store calls this on its own poll timer.
 * A no-op on faces that do not declare the stock keys.
 */
void appmessage_request_stock(void);

/**
 * @brief Asks the phone for a fresh agenda. The calendar store calls this on its own poll
 * timer. A no-op on faces that do not declare the calendar keys.
 */
void appmessage_request_calendar(void);

/** @} */
