#include "engine/grid_engine.h"
#include "time_clock.h"
#include "draw/grid_helpers.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include "io/stores/weather_store.h"

// TODO: re-tune for the taller body when gctx->headerless
static void time_clock_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    char val[16] = "--:--";
    const struct tm *t = time_store_tm();
    bool h12 = gridlock_format_clock(val, sizeof(val), t->tm_hour, t->tm_min);
    bool is_am = t->tm_hour < 12;

    gh_stat_1x2(gctx, val, h12 ? (is_am ? "AM" : "PM") : NULL, FONT_TEKO_26, &ICON_CLOCK);
}

const ModuleDef mod_time_clock_def = {
    .label = "TIME",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .body = time_clock_body
};
