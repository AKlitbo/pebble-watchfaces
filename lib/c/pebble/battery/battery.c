/**
 * @file battery.c
 * @brief battery service: subscribes to charge changes and exposes the current state
 */
#include "battery.h"

static BatteryChangedHandler s_handler;

/**
 * @brief Forward a battery change to the face's handler.
 *
 * @param state The new battery state.
 */
static void battery_handler(BatteryChargeState state)
{
    if (s_handler)
    {
        s_handler(state);
    }
}

void battery_init(BatteryChangedHandler handler)
{
    s_handler = handler;
    battery_state_service_subscribe(battery_handler);

    if (s_handler)
    {
        s_handler(battery_peek());
    }
}

void battery_deinit(void)
{
    battery_state_service_unsubscribe();
    s_handler = NULL;
}

BatteryChargeState battery_peek(void)
{
    return battery_state_service_peek();
}
