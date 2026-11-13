/**
 * @file phone_gate.c
 * @brief The phone store's live switch, following the panels placed on any layout.
 *
 * @ingroup mosaic_engine
 */
#include "phone_gate.h"

#include <pebble.h>

#include "io/stores/phone_store.h"
#include "mosaic/engine/catalog.h"
#include "settings_schema.h"

static bool s_started; ///< whether the face started the store live at launch, which a dev build's seed skips
static bool s_live;    ///< whether the store was last started live

/**
 * @brief Whether a panel showing the phone's battery is on any layout.
 *
 * @return True when Phone Battery or Battery Battery is placed in the day, night, or Quiet Time layout.
 */
static bool phone_shown(void)
{
    return mosaic_places_module(MOD_SYSTEM_PHONE_BATTERY) || mosaic_places_module(MOD_SYSTEM_BATTERY_PAIR);
}

/** @brief Starts the store live or not, by what the layouts place now. */
static void start_store(void)
{
    s_live = phone_shown();
    phone_store_init((PhoneConfig){.live = s_live}, NULL);
}

void phone_gate_start(void)
{
    s_started = true;
    start_store();
}

void phone_gate_settings_changed(void)
{
    if (!s_started || phone_shown() == s_live)
    {
        return;
    }

    start_store();
}
