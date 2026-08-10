#include "weather_sun.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/weather_store.h"

static void sunrise_value(char *out, size_t n, const char **unit_out)
{
    gh_format_hhmm(out, n, weather_store_sunrise());
}

static const StatPanel1x2 sunrise_desc = {
    .value_1x2 = sunrise_value,
    .icon = &ICON_SUNRISE,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_sunrise, "SUNRISE", SZ_1x2, FEATURE_WEATHER, 0, &sunrise_desc);

static void sunset_value(char *out, size_t n, const char **unit_out)
{
    gh_format_hhmm(out, n, weather_store_sunset());
}

static const StatPanel1x2 sunset_desc = {
    .value_1x2 = sunset_value,
    .icon = &ICON_SUNSET,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_sunset, "SUNSET", SZ_1x2, FEATURE_WEATHER, 0, &sunset_desc);
