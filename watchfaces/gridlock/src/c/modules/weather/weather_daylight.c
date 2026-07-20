#include "engine/grid_engine.h"
#include "weather_daylight.h"
#include "draw/grid_helpers.h"
#include "io/stores/weather_store.h"

// TODO: re-tune for the taller body when gctx->headerless
static void weather_daylight_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    char rise[8];
    char set[8];
    gh_format_hhmm(rise, sizeof(rise), weather_store_sunrise());
    gh_format_hhmm(set, sizeof(set), weather_store_sunset());

    int half_h = gctx->body.size.h / 2;
    gh_mini_row(gctx, "SUNRISE", rise, NULL, &ICON_SUNRISE, 0);
    gh_mini_row(gctx, "SUNSET", set, NULL, &ICON_SUNSET, half_h - 3);
}

const ModuleDef mod_weather_daylight_def = {
    .label = "DAYLIGHT",
    .sizes = SZ_2x2,
    .features = FEATURE_WEATHER,
    .theme_alias = MOD_WEATHER_SOLAR,
    .body = weather_daylight_body
};
