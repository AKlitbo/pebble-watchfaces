#include "weather_feels_like.h"
#include "weather_forecast_common.h"
#include "draw/stat_panel.h"
#include "io/stores/weather_store.h"

static void feels_like_value(char *out, size_t n, const char **unit_out)
{
    weather_temp_value(weather_store_feels_like(), out, n, unit_out);
}

static const StatPanel1x2 feels_like_desc = {
    .value_1x2 = feels_like_value,
    .icon = &ICON_THERMOMETER,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_feels_like, "FEELS LIKE", SZ_1x2, FEATURE_WEATHER, MOD_WEATHER_TEMPERATURE, &feels_like_desc);
