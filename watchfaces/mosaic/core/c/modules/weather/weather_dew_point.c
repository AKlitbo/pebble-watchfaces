#include "weather_dew_point.h"
#include "weather_forecast_common.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/weather_store.h"

static void dew_point_value(char *out, size_t n, const char **unit_out)
{
    weather_temp_value(weather_store_dew_point(), out, n, unit_out);
}

static const StatPanel1x2 dew_point_desc = {
    .value_1x2 = dew_point_value,
    .icon = &ICON_HUMIDITY,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_dew_point, "DEW POINT", SZ_1x2, FEATURE_WEATHER, MOD_WEATHER_PRECIP, &dew_point_desc);
