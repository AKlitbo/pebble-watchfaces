#include "engine/grid_engine.h"
#include "time_epoch.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/time_store.h"
#include <time.h>
#include <stdio.h>

static void epoch_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    // read the epoch off the time store so it tracks the minute tick like the other panels
    // and freezes with the dev clock for deterministic screenshots. a copy so mktime can take
    // a non-const pointer
    struct tm now = *time_store_tm();
    char val[16];
    snprintf(val, sizeof(val), "%ld", (long)mktime(&now));

    // the raw seconds in the mono font, lifted 3px so it sits centred rather than low.
    // gh_value_left pins its vertical nudge to 0 so draw it directly with the lift
    GRect box = grid_anchor(gctx, GSize(gctx->body.size.w - PANEL_PAD, 14), GAlignLeft, PANEL_PAD, -3);
    box = metric_baseline(FONT_STM_14, box);
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, val, fonts_get(FONT_STM_14), box,
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

const ModuleDef mod_time_epoch_def = {
    .label = "EPOCH",
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .body = epoch_body,
};
