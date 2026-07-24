/**
 * @file system_store.c
 * @brief The active system store: holds the battery + bluetooth state and subscribes to the
 * battery and connection services itself.
 */
#include "io/stores/system_store.h"

static struct
{
    int  battery_level;
    bool charging;
    bool bluetooth_connected;
} s_state;

static bool s_bt_initialized;   // the first bluetooth reading only seeds. later changes can buzz
static void (*s_cb)(void);
static BtVibePolicy s_vibe;
static void (*s_reconnect)(void); // fired on a real disconnected -> connected transition

/**
 * @brief Save the battery reading and notify.
 */
static void set_battery(int level, bool charging)
{
    s_state.battery_level = level;
    s_state.charging = charging;
    if (s_cb) s_cb();
}

/**
 * @brief Save the bluetooth state, buzzing the face's policy on a real transition, and notify.
 */
static void set_bluetooth(bool connected)
{
    bool was_connected = s_state.bluetooth_connected;

    // hand a real transition to the face's policy. the first reading only seeds the state
    if (s_bt_initialized && was_connected != connected && s_vibe)
    {
        s_vibe(connected);
    }

    // the phone app just came back. let the face catch up its phone-backed readouts now instead
    // of waiting out each store's poll. the first reading only seeds, so this skips at launch
    if (s_bt_initialized && !was_connected && connected && s_reconnect)
    {
        s_reconnect();
    }

    s_bt_initialized = true;
    s_state.bluetooth_connected = connected;
    if (s_cb) s_cb();
}

// --- service handlers ---

static void on_battery(BatteryChargeState state)
{
    set_battery(state.charge_percent, state.is_charging);
}

static void on_connection(bool connected)
{
    set_bluetooth(connected);
}

// --- public API ---

void system_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void system_store_on_reconnect(void (*cb)(void))
{
    s_reconnect = cb;
}

void system_store_init(SystemConfig cfg, const SystemSeed *seed)
{
    s_state.battery_level = 0;
    s_state.charging = false;
    s_state.bluetooth_connected = false;
    s_bt_initialized = false;
    s_vibe = cfg.vibe;

    if (seed)
    {
        // write straight to state so a pinned bluetooth value never trips the vibe guard
        s_state.battery_level = seed->battery;
        s_state.charging = seed->charging;
        s_state.bluetooth_connected = seed->bluetooth;
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        // both services fire their current reading via the peeks below so the store holds
        // real state immediately. the first bluetooth reading is swallowed so no startup buzz
        battery_state_service_subscribe(on_battery);
        on_battery(battery_state_service_peek());

        connection_service_subscribe((ConnectionHandlers){
            .pebble_app_connection_handler = on_connection,
        });
        on_connection(connection_service_peek_pebble_app_connection());
    }
}

void system_store_deinit(void)
{
    battery_state_service_unsubscribe();
    connection_service_unsubscribe();
}

int  system_store_battery(void)   { return s_state.battery_level; }
bool system_store_charging(void)  { return s_state.charging; }
bool system_store_bluetooth(void) { return s_state.bluetooth_connected; }
