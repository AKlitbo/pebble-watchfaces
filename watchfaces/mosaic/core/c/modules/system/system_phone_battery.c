/**
 * @file system_phone_battery.c
 * @brief The phone battery panel, drawn the same as the watch's battery panel at both sizes.
 *
 * The level only comes from the Pebble app's Phone plugin, so the panel says PLUGINS OFF while
 * plugins are off, and shows -- over an empty gauge while the phone is away. The phone store clears its
 * level when the phone drops, since the watch can no longer know it.
 *
 * @ingroup mosaic_mod_system
 */
#include "engine/grid_engine.h"
#include "system_phone_battery.h"
#include "system_battery.h"

#include "io/stores/phone_store.h"

static void system_phone_battery_body(GridCtx *gctx)
{
    // the phone says when nothing on it serves the battery, which is plugins off in the Pebble app
    if (phone_store_no_plugin())
    {
        battery_body_plugins_off(gctx);
        return;
    }

    battery_body(gctx, phone_store_battery());
}

const ModuleDef mod_system_phone_battery_def = {
    .label = "PHONE",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_PHONE,
    .body = system_phone_battery_body
};
