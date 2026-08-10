#include "engine/grid_engine.h"
#include "weather_conditions.h"
#include "weather_forecast_common.h"
#include "mosaic/draw/grid_helpers.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "io/stores/weather_store.h"
#include "settings_schema.h"

// TODO: re-tune for the taller body when gctx->headerless
static void weather_conditions_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    char hum_val[16];
    fmt_int_or_dash(hum_val, sizeof(hum_val), weather_store_humidity(), "%d%%");

    char wind_val[16];
    weather_wind_value_str(wind_val, sizeof(wind_val));

    IconSpec wind = icon_wind_spec(weather_store_wind_dir());

    int half_h = gctx->body.size.h / 2;
    gh_mini_row(gctx, "HUMIDITY", hum_val, NULL, &ICON_HUMIDITY, 0);
    gh_mini_row(gctx, "WIND SPEED", wind_val, gridlock_wind_unit_label(), &wind, half_h - 3);
}

const ModuleDef mod_weather_conditions_def = {
    .label = "CONDITIONS",
    .sizes = SZ_2x2,
    .features = FEATURE_WEATHER,
    .body = weather_conditions_body
};
