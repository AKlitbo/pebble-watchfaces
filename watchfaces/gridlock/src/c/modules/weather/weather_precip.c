#include "weather_precip.h"
#include "draw/stat_panel.h"
#include "text/number_format.h"
#include "draw/common.h"
#include "io/stores/weather_store.h"

static void precip_value(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, weather_store_precip_chance(), "%d%%");
}

static const StatPanel1x2 precip_desc = {
    .value_1x2 = precip_value,
    .icon = &ICON_RAIN,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(weather_precip, "PRECIPITATION", SZ_1x2, FEATURE_WEATHER, 0, &precip_desc);
