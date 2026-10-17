/**
 * @file battery.c
 * @brief The battery gauge's warning colours.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/theme/battery.h"

#include "sketchbook/theme/palette.h"

/**
 * @addtogroup family-sketchbook
 * @{
 */

GColor sketchbook_battery_fill(uint8_t theme, GColor ink, int level)
{
    // Mono keeps the gauge greyscale. the lit-segment count already shows the charge, so a
    // red or amber warning would only break the look
    if (theme == THEME_MONO)
    {
        return ink;
    }

    if (level <= 20)
    {
        return GColorRed;
    }

    if (level <= 40)
    {
        return GColorChromeYellow;
    }

    return ink;
}

/** @} */
