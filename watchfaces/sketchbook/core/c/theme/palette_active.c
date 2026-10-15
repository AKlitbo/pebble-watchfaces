/**
 * @file palette_active.c
 * @brief Picking the live palette from the mode and the day/night and Quiet Time readings.
 *
 * The two real decisions, the rotation and which of day/night/quiet wins, are the framework's
 * `rotate_index` and `role_pick`, both host-tested. This is the thin bit that joins them to the
 * face's own settings.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/theme/palette_active.h"

#include <stddef.h>

#include "sketchbook/theme/palette.h"
#include "clock/rotate.h"
#include "role/role.h"

/** How many minutes a Shuffle choice holds a palette for. */
static int interval_minutes(uint8_t choice)
{
    switch (choice)
    {
        case SHUFFLE_EVERY_15M: return 15;
        case SHUFFLE_EVERY_30M: return 30;
        case SHUFFLE_HOURLY:    return 60;
        case SHUFFLE_EVERY_3H:  return 180;
        case SHUFFLE_EVERY_6H:  return 360;
        case SHUFFLE_DAILY:     return 1440;
        default:                return 60;
    }
}

uint8_t sketchbook_active_theme(const PalettePick *pick, int year, int yday, int hour, int minute)
{
    if (pick == NULL)
    {
        return 0;
    }

    if (pick->mode == PALETTE_MODE_SHUFFLE)
    {
        return (uint8_t)rotate_index(year, yday, hour, minute, interval_minutes(pick->shuffle_interval), THEME_COUNT);
    }

    if (pick->mode == PALETTE_MODE_DAYNIGHT)
    {
        Role role = role_pick(pick->quiet_on, pick->use_quiet, pick->night_on, pick->use_night);

        if (role == ROLE_QUIET)
        {
            return pick->quiet_theme;
        }

        if (role == ROLE_NIGHT)
        {
            return pick->night_theme;
        }

        return pick->day_theme;
    }

    // PALETTE_MODE_FIXED, and anything out of range falls back to the one chosen palette
    return pick->day_theme;
}
