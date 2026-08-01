#include "weather_wind.h"
#include "weather_forecast_common.h"
#include "draw/stat_panel.h"
#include "io/stores/weather_store.h"
#include "settings_schema.h"

static void wind_value(char *out, size_t n, const char **unit_out)
{
    weather_wind_value_str(out, n);
    *unit_out = gridlock_wind_unit_label();
}

// the arrow glyph turns with the wind direction, so it is a per-reading icon
static IconSpec wind_icon(void)
{
    return icon_wind_spec(weather_store_wind_dir());
}

static const StatPanel1x2 wind_desc = {
    .value_1x2 = wind_value,
    .icon_fn = wind_icon,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_wind, "WIND SPEED", SZ_1x2, FEATURE_WEATHER, 0, &wind_desc);
