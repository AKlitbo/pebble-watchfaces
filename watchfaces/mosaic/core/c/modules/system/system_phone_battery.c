/**
 * @file system_phone_battery.c
 * @brief The phone battery panel, drawn the same as the watch's battery panel at both sizes.
 *
 * The level only comes from the Pebble app's Phone plugin, so the panel shows -- over an empty gauge
 * whenever plugins are off or the phone is away. The phone store clears its level when the phone
 * drops, since the watch can no longer know it.
 *
 * @ingroup mosaic_mod_system
 */
#include "engine/grid_engine.h"
#include "system_phone_battery.h"
#include "system_battery.h"

#include "io/stores/phone_store.h"

static void system_phone_battery_body(GridCtx *gctx)
{
    battery_body(gctx, phone_store_battery());
}

const ModuleDef mod_system_phone_battery_def = {
    .label = "PHONE",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_PHONE,
    .body = system_phone_battery_body
};
