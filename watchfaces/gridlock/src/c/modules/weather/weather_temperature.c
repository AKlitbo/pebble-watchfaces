#include "weather_temperature.h"
#include "weather_forecast_common.h"
#include "draw/stat_panel.h"
#include "io/stores/weather_store.h"

static void temperature_value(char *out, size_t n, const char **unit_out)
{
    weather_temp_value(weather_store_temp(), out, n, unit_out);
}

static const StatPanel1x2 temperature_desc = {
    .value_1x2 = temperature_value,
    .icon = &ICON_THERMOMETER,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_temperature, "TEMPERATURE", SZ_1x2, FEATURE_WEATHER, 0, &temperature_desc);
