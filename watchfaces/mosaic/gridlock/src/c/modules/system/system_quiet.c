#include "engine/grid_engine.h"
#include "system_quiet.h"
#include "mosaic/draw/grid_helpers.h"

// TODO: re-tune for the taller body when gctx->headerless
static void system_quiet_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    bool quiet = quiet_time_is_active();

    // muted speaker while quiet time holds, a full one when sound is back on
    IconSpec icon = { quiet ? RESOURCE_ID_ICON_SYSTEM_VOLUME_MUTED : RESOURCE_ID_ICON_SYSTEM_VOLUME_FULL, 0, 0 };

    gh_stat_1x2(gctx, quiet ? "ON" : "OFF", NULL, FONT_TEKO_26, &icon);
}

const ModuleDef mod_system_quiet_def = {
    .label = "QUIET TIME",
    .sizes = SZ_1x2,
    .features = FEATURE_SYSTEM,
    // shares the connection module's colours: quiet time rides under it for customization
    .theme_alias = MOD_SYSTEM_CONNECTION,
    .body = system_quiet_body
};
