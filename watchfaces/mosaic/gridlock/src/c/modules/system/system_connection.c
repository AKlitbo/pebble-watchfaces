#include "engine/grid_engine.h"
#include "system_connection.h"
#include "mosaic/draw/grid_helpers.h"
#include "io/stores/system_store.h"

// TODO: re-tune for the taller body when gctx->headerless
static void system_connection_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    bool linked = system_store_bluetooth();

    // the icon carries the state too: a plain bluetooth glyph when linked, the
    // slashed one when the phone is away
    const IconSpec *icon = linked ? &ICON_BLUETOOTH : &ICON_BLUETOOTH_SLASH;

    gh_stat_1x2(gctx, linked ? "LINKED" : "OFFLINE", NULL, FONT_TEKO_26, icon);
}

const ModuleDef mod_system_connection_def = {
    .label = "BLUETOOTH",
    .sizes = SZ_1x2,
    .features = FEATURE_SYSTEM,
    .body = system_connection_body
};
