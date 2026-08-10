#include "engine/grid_engine.h"
#include "system_status.h"
#include "mosaic/draw/grid_helpers.h"
#include "io/stores/system_store.h"
#include "math/scale.h"

// a little battery glyph: an accent-outlined casing with a nub on the right and five segments
// inside that light up with the charge. there is no battery icon in resources so this stands in
// for one, matching the bluetooth and quiet glyphs beside it
static void battery_glyph(GridCtx *gctx, GRect area, int level)
{
    level = clamp_int(level, 0, 100);

    GContext *ctx = gctx->ctx;
    GRect nub = GRect(area.origin.x + area.size.w, area.origin.y + area.size.h / 4 + 1, 2, area.size.h / 2);

    graphics_context_set_stroke_color(ctx, gctx->color_accent);
    graphics_draw_round_rect(ctx, area, 2);
    graphics_context_set_fill_color(ctx, gctx->color_accent);
    graphics_fill_rect(ctx, nub, 1, GCornersRight);

    GRect inner = GRect(area.origin.x + 2, area.origin.y + 2, area.size.w - 4, area.size.h - 4);
    const int segments = 5, gap = 1;
    int seg_w = segment_width(inner.size.w, gap, segments);
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    // round to nearest so a middling charge lights the fair number of cells, but never zero
    // while there is still some charge left
    int lit = (level * segments + 50) / 100;
    if (lit == 0 && level > 0)
    {
        lit = 1;
    }

    graphics_context_set_fill_color(ctx, gctx->color_icon);
    for (int i = 0; i < lit; i++)
    {
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y, seg_w, inner.size.h);
        graphics_fill_rect(ctx, cell, 0, GCornerNone);
    }
}

// the combined 1x2 device panel: battery, bluetooth, and quiet time share the tight body, one
// indicator per third, each as its own glyph. the last pixel tweaks happen in the emulator
// TODO: re-tune for the taller body when gctx->headerless
static void system_status_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    GRect body = gctx->body;
    int colw = body.size.w / 3;

    // left third: the battery gauge, centred in its column with a touch of room for the nub
    int bw = 22, bh = 11;
    GRect batt = GRect(body.origin.x + (colw - bw) / 2 + 2,
                       body.origin.y + (body.size.h - bh) / 2, bw, bh);
    battery_glyph(gctx, batt, system_store_battery());

    // middle third: bluetooth, the plain glyph when linked and the slashed one when away
    bool linked = system_store_bluetooth();
    icon_draw(gctx, linked ? &ICON_BLUETOOTH : &ICON_BLUETOOTH_SLASH, GAlignCenter, 0, 0);

    // right third: quiet time, a muted speaker while it holds and a full one otherwise
    uint32_t vol = quiet_time_is_active() ? RESOURCE_ID_ICON_SYSTEM_VOLUME_MUTED
                                          : RESOURCE_ID_ICON_SYSTEM_VOLUME_FULL;
    icon_draw_res(gctx, vol, GAlignCenter, colw + 1, 0);
}

const ModuleDef mod_system_status_def = {
    .label = "STATUS",
    .sizes = SZ_1x2,
    .features = FEATURE_SYSTEM,
    // rides under the battery module's colours for customization
    .theme_alias = MOD_SYSTEM_BATTERY,
    .body = system_status_body
};
