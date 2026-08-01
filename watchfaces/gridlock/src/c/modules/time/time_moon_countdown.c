#include "engine/grid_engine.h"
#include "time_moon_countdown.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include "clock/moon.h"
#include <time.h>
#include <stdio.h>

static void moon_countdown_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    time_t now = time(NULL);
    int to_full = moon_days_to_phase(now, true);
    int to_new = moon_days_to_phase(now, false);

    // count to whichever big phase lands first and show that phase's glyph
    bool full = to_full <= to_new;
    int days = full ? to_full : to_new;

    static const IconSpec full_glyph = { RESOURCE_ID_ICON_TIME_MOON_14, 0, 0 };
    static const IconSpec new_glyph = { RESOURCE_ID_ICON_TIME_MOON_0, 0, 0 };

    char val[8];
    if (days == 0)
    {
        snprintf(val, sizeof(val), "NOW");
    }
    else
    {
        snprintf(val, sizeof(val), "%dD", days);
    }

    gh_stat_1x2(gctx, val, full ? "FULL" : "NEW", FONT_TEKO_26,
                full ? &full_glyph : &new_glyph);
}

const ModuleDef mod_time_moon_countdown_def = {
    .label = "NEXT MOON",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_MOON,
    .body = moon_countdown_body,
};
