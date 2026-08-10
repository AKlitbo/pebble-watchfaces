#include "weather_pressure.h"
#include "mosaic/draw/stat_panel.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "io/stores/weather_store.h"

static void pressure_value(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, weather_store_pressure(), "%d");
    *unit_out = "HPA";
}

static const StatPanel1x2 pressure_desc = {
    .value_1x2 = pressure_value,
    .icon = NULL,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_pressure, "PRESSURE", SZ_1x2, FEATURE_WEATHER, 0, &pressure_desc);
