/**
 * @file vibe.c
 * @brief Haptic feedback wrapper implementation
 */
#include "system/vibe/vibe.h"

#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

void vibe_pulse(VibePulse pulse)
{
    // respect do-not-disturb: stay silent so a flurry of events can't buzz through it
    if (quiet_time_is_active())
    {
        return;
    }

    switch (pulse)
    {
        case VibePulseShort:
            vibes_short_pulse();
            break;
        case VibePulseLong:
            vibes_long_pulse();
            break;
        case VibePulseDouble:
            vibes_double_pulse();
            break;
    }
}

void vibe_custom(const uint32_t *durations, uint32_t num_segments)
{
    // stay silent during do-not-disturb, same as vibe_pulse
    if (quiet_time_is_active())
    {
        return;
    }

    vibes_enqueue_custom_pattern((VibePattern){ .durations = durations, .num_segments = num_segments });
}

void vibe_choice(uint8_t choice)
{
    switch (choice)
    {
        case VIBE_SHORT:  vibe_pulse(VibePulseShort);  break;
        case VIBE_LONG:   vibe_pulse(VibePulseLong);   break;
        case VIBE_DOUBLE: vibe_pulse(VibePulseDouble); break;
        default: break;  // VIBE_NONE
    }
}

void vibe_bt_transition(bool connected)
{
    vibe_choice(connected ? settings_u8(SETTING_BLUETOOTH_VIBE_CONNECT)
                          : settings_u8(SETTING_BLUETOOTH_VIBE_DISCONNECT));
}
