#include "engine/grid_engine.h"
#include "time_julian.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/time_store.h"
#include "clock/astro.h"
#include <time.h>
#include <stdio.h>

static void julian_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    // read the epoch off the time store so it freezes with the dev clock. a copy so mktime can
    // take a non-const pointer
    struct tm now = *time_store_tm();
    int32_t jd = astro_jd_centi(mktime(&now)) / 100; // the whole Julian Date

    char val[16];
    snprintf(val, sizeof(val), "%ld", (long)jd);

    // the long day count in the mono font, lifted 3px so it sits centred like the Epoch tile
    GRect box = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD, 14), GAlignLeft, PANEL_PAD, -3);
    box = metric_baseline(FONT_STM_14, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(FONT_STM_14), box,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

const ModuleDef mod_time_julian_def = {
    .label = "JULIAN",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    // share the Epoch tile's colour, both being raw running time counts
    .theme_alias = MOD_TIME_EPOCH,
    .body = julian_body,
};
