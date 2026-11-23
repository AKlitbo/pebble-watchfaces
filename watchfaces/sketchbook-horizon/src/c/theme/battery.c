/**
 * @file battery.c
 * @brief The battery gauge's warning colours.
 *
 * @ingroup watchface-sketchbook
 */
#include "theme/battery.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

GColor sketchbook_battery_fill(const CommonPalette *pal, int level)
{
    if (level <= 20)
    {
        return pal->batt_critical;
    }

    if (level <= 40)
    {
        return pal->batt_low;
    }

    return pal->bar_ink;
}

/** @} */
