/**
 * @file vibe.c
 * @brief haptic feedback wrapper implementation
 */
#include "vibe.h"

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
