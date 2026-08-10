/**
 * @file weather_forecast_daily.c
 * @brief The 7-day forecast row. Reads the daily strip from the weather store, turns each
 * column's weekday index into a short name, and hands the strip to the shared drawer. The
 * temperature shown is the day's high.
 * @ingroup gridlock_mod_weather
 */
#include "engine/grid_engine.h"
#include "weather_forecast_daily.h"
#include "mosaic/modules/weather/weather_forecast_common.h"
#include "io/stores/weather_store.h"
#include "clock/weekday.h"
#include <stdio.h>
#include <string.h>

static void daily_body(GridCtx *gctx)
{
    const WeatherDaily *strip = weather_store_forecast_daily();
    uint8_t count = strip->count;
    if (count > WEATHER_FORECAST_COLS)
    {
        count = WEATHER_FORECAST_COLS;
    }

    char labels[WEATHER_FORECAST_COLS][FORECAST_LABEL_LEN];
    ForecastCell cells[WEATHER_FORECAST_COLS];
    for (uint8_t i = 0; i < count; i++)
    {
        int weekday = (strip->base_weekday + i) % 7;
        snprintf(labels[i], FORECAST_LABEL_LEN, "%s", weekday_short(weekday));
        cells[i].code = strip->col[i].code;
        cells[i].temp = strip->col[i].temp_max;
    }

    forecast_row_draw(gctx, labels, cells, count);
}

// the header names how many days actually show: four in the single-row 1x4, eight in the
// two-row 2x4 stack
static const char *daily_get_label(ModuleSize size)
{
    return (size == MSIZE_1x4) ? "4 DAY FORECAST" : "8 DAY FORECAST";
}

const ModuleDef mod_weather_forecast_daily_def = {
    .label = "FORECAST",
    .get_label = daily_get_label,
    .sizes = SZ_1x4 | SZ_2x4,
    .features = FEATURE_WEATHER,
    .theme_alias = MOD_WEATHER_FORECAST_HOURLY,
    .body = daily_body
};
