#include "weather_humidity.h"
#include "mosaic/draw/stat_panel.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "io/stores/weather_store.h"

static void humidity_value(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, weather_store_humidity(), "%d%%");
}

static const StatPanel1x2 humidity_desc = {
    .value_1x2 = humidity_value,
    .icon = &ICON_HUMIDITY,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_humidity, "HUMIDITY", SZ_1x2, FEATURE_WEATHER, 0, &humidity_desc);
