#include "engine/grid_engine.h"
#include "time_moon.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include <time.h>
#include <stdio.h>
#include "clock/moon.h"

// the 28 phase glyphs in cycle order (0 new .. 14 full .. wrapping back to new). the index
// comes straight from moon_glyph_index so this table just turns it into a resource id
static const uint16_t s_moon_res[28] = {
    RESOURCE_ID_ICON_TIME_MOON_0,  RESOURCE_ID_ICON_TIME_MOON_1,  RESOURCE_ID_ICON_TIME_MOON_2,  RESOURCE_ID_ICON_TIME_MOON_3,
    RESOURCE_ID_ICON_TIME_MOON_4,  RESOURCE_ID_ICON_TIME_MOON_5,  RESOURCE_ID_ICON_TIME_MOON_6,  RESOURCE_ID_ICON_TIME_MOON_7,
    RESOURCE_ID_ICON_TIME_MOON_8,  RESOURCE_ID_ICON_TIME_MOON_9,  RESOURCE_ID_ICON_TIME_MOON_10, RESOURCE_ID_ICON_TIME_MOON_11,
    RESOURCE_ID_ICON_TIME_MOON_12, RESOURCE_ID_ICON_TIME_MOON_13, RESOURCE_ID_ICON_TIME_MOON_14, RESOURCE_ID_ICON_TIME_MOON_15,
    RESOURCE_ID_ICON_TIME_MOON_16, RESOURCE_ID_ICON_TIME_MOON_17, RESOURCE_ID_ICON_TIME_MOON_18, RESOURCE_ID_ICON_TIME_MOON_19,
    RESOURCE_ID_ICON_TIME_MOON_20, RESOURCE_ID_ICON_TIME_MOON_21, RESOURCE_ID_ICON_TIME_MOON_22, RESOURCE_ID_ICON_TIME_MOON_23,
    RESOURCE_ID_ICON_TIME_MOON_24, RESOURCE_ID_ICON_TIME_MOON_25, RESOURCE_ID_ICON_TIME_MOON_26, RESOURCE_ID_ICON_TIME_MOON_27,
};

// the same 28 phases at the bigger 2x2 hero size
static const uint16_t s_moon_lg_res[28] = {
    RESOURCE_ID_ICON_TIME_MOON_LG_0,  RESOURCE_ID_ICON_TIME_MOON_LG_1,  RESOURCE_ID_ICON_TIME_MOON_LG_2,  RESOURCE_ID_ICON_TIME_MOON_LG_3,
    RESOURCE_ID_ICON_TIME_MOON_LG_4,  RESOURCE_ID_ICON_TIME_MOON_LG_5,  RESOURCE_ID_ICON_TIME_MOON_LG_6,  RESOURCE_ID_ICON_TIME_MOON_LG_7,
    RESOURCE_ID_ICON_TIME_MOON_LG_8,  RESOURCE_ID_ICON_TIME_MOON_LG_9,  RESOURCE_ID_ICON_TIME_MOON_LG_10, RESOURCE_ID_ICON_TIME_MOON_LG_11,
    RESOURCE_ID_ICON_TIME_MOON_LG_12, RESOURCE_ID_ICON_TIME_MOON_LG_13, RESOURCE_ID_ICON_TIME_MOON_LG_14, RESOURCE_ID_ICON_TIME_MOON_LG_15,
    RESOURCE_ID_ICON_TIME_MOON_LG_16, RESOURCE_ID_ICON_TIME_MOON_LG_17, RESOURCE_ID_ICON_TIME_MOON_LG_18, RESOURCE_ID_ICON_TIME_MOON_LG_19,
    RESOURCE_ID_ICON_TIME_MOON_LG_20, RESOURCE_ID_ICON_TIME_MOON_LG_21, RESOURCE_ID_ICON_TIME_MOON_LG_22, RESOURCE_ID_ICON_TIME_MOON_LG_23,
    RESOURCE_ID_ICON_TIME_MOON_LG_24, RESOURCE_ID_ICON_TIME_MOON_LG_25, RESOURCE_ID_ICON_TIME_MOON_LG_26, RESOURCE_ID_ICON_TIME_MOON_LG_27,
};

// TODO: re-tune for the taller body when gctx->headerless
static void time_moon_body(GridCtx *gctx)
{
    time_t now = time(NULL);
    int idx = moon_glyph_index(now, 28);
    IconSpec glyph = { s_moon_res[idx], 0, 0 };

    char pct[8];
    snprintf(pct, sizeof(pct), "%d%%", moon_illumination_pct(now));

    if (gctx->size == MSIZE_1x2)
    {
        // the % on the left, the moon glyph vertically centred on the right. the full disc
        // reads better centred than pinned to the bottom like a normal stat icon
        gh_value_left(gctx, pct, FONT_TEKO_26, gctx->body.size.w - PANEL_PAD, 30, PANEL_PAD);

        GSize msz = icon_size(glyph.res);
        GRect mdst = grid_anchor(gctx, msz, GAlignRight, -EDGE_PAD(gctx) + 2, 0);
        icon_draw_rect(gctx, glyph.res, mdst);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        // the big moon is the hero up top, with the phase name and illumination under it.
        // draw the full glyph frame (no trim) so a half-lit moon still centres like a whole
        // disc instead of shifting toward its lit side
        GSize msz = icon_size(s_moon_lg_res[idx]);
        GRect mdst = grid_anchor(gctx, msz, GAlignTop, 0, 3);
        icon_draw_rect(gctx, s_moon_lg_res[idx], mdst);

        // one space (not two) so "WAN CRESC 2%" clears the caption's padded box, and sit it a
        // touch lower under the moon
        char caption[24];
        snprintf(caption, sizeof(caption), "%s %s", moon_phase_name(now), pct);
        gh_caption(gctx, caption, 3 + msz.h + 5);
    }
}

const ModuleDef mod_time_moon_def = {
    .label = "MOON PHASE",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_TIME,
    .body = time_moon_body
};
