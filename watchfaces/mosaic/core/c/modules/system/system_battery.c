#include "engine/grid_engine.h"
#include "system_battery.h"
#include "mosaic/draw/grid_helpers.h"
#include "math/scale.h"
#include "text/number_format.h"

#include "io/stores/system_store.h"
#include "ui/fonts.h"

int battery_text_w(const char *text, FontId font)
{
    GSize ts = graphics_text_layout_get_content_size(text, fonts_get(font),
                   GRect(0, 0, 1000, BATTERY_VALUE_H), GTextOverflowModeFill, GTextAlignmentLeft);
    return ts.w;
}

/**
 * @brief The 1x2: the number on the left and a five segment gauge filling the room to its right,
 * flush with the right edge.
 *
 * @param gctx The grid context.
 * @param level The level from 0 to 100, or -1 for an empty gauge.
 * @param text The number to show.
 */
static void battery_row(GridCtx *gctx, int level, const char *text)
{
    int value_w = battery_text_w(text, FONT_TEKO_26) + 2;
    gh_value_left(gctx, text, FONT_TEKO_26, value_w, BATTERY_VALUE_H, PANEL_PAD);

    // gauge takes up the room to the right of the number
    // size it to how wide it really draws so the chopped-up
    // segments sit flush against the right edge
    int max_gauge_w = gctx->body.size.w - PANEL_PAD - value_w - 6 - EDGE_PAD(gctx);
    int segments = 5;
    int gap = 2;
    int seg_w = segment_width(max_gauge_w, gap, segments);
    int actual_gauge_w = segments * seg_w + gap * (segments - 1);

    GRect gauge_rect = grid_anchor(gctx, GSize(actual_gauge_w, 17), GAlignRight, -EDGE_PAD(gctx), 0);
    gh_gauge(gctx, gauge_rect, segments, level);
}

static void battery_inner_2x2(GridCtx *gctx, int level, const char *text)
{
    // number sits up top in the middle
    // a reading is shoved right 4px so the % sign doesn't look left-heavy. the dashes have no sign
    gh_value_top(gctx, text, FONT_TEKO_34, 38, level < 0 ? 0 : 4, -7);

    gh_caption(gctx, level < 0 ? "NO READING" : "OF 100% LEFT", 27);

    GRect gauge_rect = grid_anchor(gctx, GSize(gctx->body.size.w - 16, 16), GAlignTop, 0, 46);
    gh_gauge(gctx, gauge_rect, 10, level);
}

// TODO: re-tune for the taller body when gctx->headerless
void battery_body(GridCtx *gctx, int level)
{
    char text[8];
    fmt_int_or_dash(text, sizeof(text), level, "%d%%");

    if (gctx->size == MSIZE_1x2)
    {
        battery_row(gctx, level, text);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        battery_inner_2x2(gctx, level, text);
    }
}

static void system_battery_body(GridCtx *gctx)
{
    battery_body(gctx, clamp_int(system_store_battery(), 0, 100));
}

const ModuleDef mod_system_battery_def = {
    .label = "BATTERY",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_SYSTEM,
    .body = system_battery_body
};
