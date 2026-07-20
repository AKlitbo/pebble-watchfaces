/**
 * @file weather_forecast_hourly.c
 * @brief The hourly forecast row. Reads the hourly strip from the weather store, works out
 * each column's time label from the base hour and step, and hands the strip to the shared
 * drawer.
 * @ingroup gridlock_mod_weather
 */
#include "engine/grid_engine.h"
#include "weather_forecast_hourly.h"
#include "weather_forecast_common.h"
#include "io/stores/weather_store.h"
#include "settings_schema.h"
#include <stdio.h>

// builds one column's time label like "9A" or "13", matching the clock's 12/24 hour style
static void hour_label(char *out, size_t n, int hour24)
{
    hour24 = ((hour24 % 24) + 24) % 24;

    if (gridlock_clock_is_24h())
    {
        snprintf(out, n, "%d", hour24);
        return;
    }

    int hour12 = hour24 % 12;
    if (hour12 == 0)
    {
        hour12 = 12;
    }
    snprintf(out, n, "%d%c", hour12, hour24 < 12 ? 'A' : 'P');
}

static void hourly_body(GridCtx *gctx)
{
    const WeatherHourly *strip = weather_store_forecast_hourly();
    uint8_t count = strip->count;
    if (count > WEATHER_FORECAST_COLS)
    {
        count = WEATHER_FORECAST_COLS;
    }

    char labels[WEATHER_FORECAST_COLS][FORECAST_LABEL_LEN];
    ForecastCell cells[WEATHER_FORECAST_COLS];
    for (uint8_t i = 0; i < count; i++)
    {
        hour_label(labels[i], FORECAST_LABEL_LEN, strip->base_hour + i * strip->step_hours);
        cells[i].code = strip->col[i].code;
        cells[i].temp = strip->col[i].temp;
    }

    forecast_row_draw(gctx, labels, cells, count);
}

const ModuleDef mod_weather_forecast_hourly_def = {
    .label = "HOURLY FORECAST",
    .sizes = SZ_1x4 | SZ_2x4,
    .features = FEATURE_WEATHER,
    .body = hourly_body
};
