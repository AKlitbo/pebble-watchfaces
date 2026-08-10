#include "engine/grid_engine.h"
#include "system_battery.h"
#include "mosaic/draw/grid_helpers.h"
#include "math/scale.h"

#include <stdio.h>

#include "io/stores/system_store.h"
#include "ui/fonts.h"

static void system_battery_inner_2x2(GridCtx *gctx, int level, const char *pct)
{
    // number sits up top in the middle
    // shoved right 4px so the % sign doesn't look left-heavy
    gh_value_top(gctx, pct, FONT_TEKO_34, 38, 4, -7);

    gh_caption(gctx, "OF 100% LEFT", 27);

    GRect gauge_rect = grid_anchor(gctx, GSize(gctx->body.size.w - 16, 16), GAlignTop, 0, 46);
    gh_gauge(gctx, gauge_rect, 10, level);
}

static void system_battery_inner_1x2(GridCtx *gctx, int level, const char *pct)
{
    int value_h = 30;
    GSize ts = graphics_text_layout_get_content_size(pct, fonts_get(FONT_TEKO_26),
                   GRect(0, 0, gctx->body.size.w, value_h), GTextOverflowModeFill, GTextAlignmentLeft);

    gh_value_left(gctx, pct, FONT_TEKO_26, ts.w + 2, value_h, PANEL_PAD);

    // gauge takes up the room to the right of the number
    // size it to how wide it really draws so the chopped-up
    // segments sit flush against the right edge
    int max_gauge_w = gctx->body.size.w - PANEL_PAD - (ts.w + 2) - 6 - EDGE_PAD(gctx);
    int segments = 5;
    int gap = 2;
    int seg_w = segment_width(max_gauge_w, gap, segments);
    int actual_gauge_w = segments * seg_w + gap * (segments - 1);

    GRect gauge_rect = grid_anchor(gctx, GSize(actual_gauge_w, 17), GAlignRight, -EDGE_PAD(gctx), 0);
    gh_gauge(gctx, gauge_rect, segments, level);
}

// TODO: re-tune for the taller body when gctx->headerless
static void system_battery_body(GridCtx *gctx)
{
    int level = clamp_int(system_store_battery(), 0, 100);

    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", level);

    if (gctx->size == MSIZE_1x2)
    {
        system_battery_inner_1x2(gctx, level, pct);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        system_battery_inner_2x2(gctx, level, pct);
    }
}

const ModuleDef mod_system_battery_def = {
    .label = "BATTERY",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_SYSTEM,
    .body = system_battery_body
};
