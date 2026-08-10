/**
 * @file weather_forecast_common.h
 * @brief The shared drawer for the two forecast rows (hourly and daily). Both rows look the
 * same: tight boxes, each with a little label stacked over a temperature on the left and a
 * small sky icon to their right. The 1x4 is one row of four, the 2x4 stacks two rows for eight.
 * The hourly and daily modules just fill in the labels and numbers and hand the strip here.
 *
 * @ingroup mosaic_mod_weather
 */
#pragma once
#include "engine/grid_engine.h"
#include "mosaic/draw/icons.h" // IconSpec for the shared weather-panel helpers

/**
 * @addtogroup mosaic_mod_weather
 * @{
 */

/// The widest a column label gets is "11A" or a two digit hour so five chars plus the NUL
#define FORECAST_LABEL_LEN 6

/**
 * @brief One column of the forecast strip: a sky code and a temperature.
 */
typedef struct
{
    uint8_t code;  ///< Sky code that picks an icon from the shared list (255 means unknown)
    int16_t temp;  ///< Temperature in the user's unit, WEATHER_NO_TEMP for no reading
} ForecastCell;

/**
 * @brief Paints a forecast strip across the module's body.
 *
 * Draws the columns as tight boxes: one row of four at 1x4, or two stacked rows (up to eight
 * columns) at 2x4. A count of 0 draws a single placeholder so an empty row isn't just blank.
 *
 * @param gctx The grid context.
 * @param labels One short label per column (e.g. "9A" or "SA").
 * @param cells One cell per column.
 * @param count How many columns to draw (0 for none yet).
 */
void forecast_row_draw(GridCtx *gctx, const char labels[][FORECAST_LABEL_LEN],
                       const ForecastCell *cells, uint8_t count);

/**
 * @brief Formats a temperature with @p fmt, or "--" when there is no reading. The shared
 * "show the number, or dashes when there is no reading" helper the weather panels lean on.
 *
 * @param buf Output buffer.
 * @param n Buffer size.
 * @param temp The temperature, or WEATHER_NO_TEMP for no reading.
 * @param fmt The printf format applied when present (one int), e.g. "%d" or "%d°".
 */
void weather_fmt_temp(char *buf, size_t n, int temp, const char *fmt);

/**
 * @brief Formats a temperature into @p out ("--" for no reading) and sets @p unit_out to the
 * main temp's "°C" or "°F" so a derived reading's small unit matches it. The value half of the
 * feels-like and dew-point stat panels.
 *
 * @param temp The temperature, or WEATHER_NO_TEMP for no reading.
 * @param out The value output buffer.
 * @param n The buffer size.
 * @param unit_out Set to the trailing unit label (left untouched for no reading).
 */
void weather_temp_value(int temp, char *out, size_t n, const char **unit_out);

/**
 * @brief Formats the current wind speed in the user's unit into @p buf, or "--" for no reading.
 * Handles the no-reading case (a negative km/h) that both wind panels need, since running that
 * through gridlock_wind_value would print a bogus number.
 *
 * @param buf Output buffer.
 * @param n Buffer size.
 */
void weather_wind_value_str(char *buf, size_t n);

/** @} */
