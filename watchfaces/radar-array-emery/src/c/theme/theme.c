/**
 * @file theme.c
 * @brief Per-theme lookups: background resource ids and chrome accent colors.
 */
#include "theme.h"

uint32_t bg_resource_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:
            return RESOURCE_ID_IMAGE_BG_RESCUE;
        case 2:
            return RESOURCE_ID_IMAGE_BG_NEON;
        case 3:
            return RESOURCE_ID_IMAGE_BG_STEALTH;
        case 4:
            return RESOURCE_ID_IMAGE_BG_PHOSPHOR;
        case 5:
            return RESOURCE_ID_IMAGE_BG_CRIMSON;
        case 6:
            return RESOURCE_ID_IMAGE_BG_MONO;
        default:
            return RESOURCE_ID_IMAGE_BG_DEFAULT;
    }
}

GColor panel_accent_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Rescue
            return GColorLightGray;
        case 2:  // Neon
            return GColorCyan;
        case 3:  // Stealth
            return GColorElectricBlue;
        case 4:  // Phosphor
            return GColorRajah;
        case 5:  // Crimson
            return GColorDarkCandyAppleRed;
        case 6:  // Mono
            return GColorLightGray;
        default: // Default
            return GColorChromeYellow;
    }
}

GColor primary_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Rescue
            return GColorRed;
        case 2:  // Neon
            return GColorMagenta;
        case 3:  // Stealth
            return GColorLightGray;
        case 4:  // Phosphor
            return GColorChromeYellow;
        case 5:  // Crimson
            return GColorRed;
        case 6:  // Mono
            return GColorWhite;
        default: // Default
            return GColorGreen;
    }
}

GColor battery_fill_for_theme(uint8_t theme, int level)
{
    // the Mono theme keeps the gauge grayscale - the lit-segment count already shows
    // the charge, so the red/amber low-battery warnings would only break the look
    if (theme == 6)  // Mono
    {
        return primary_color_for_theme(theme);
    }

    if (level <= 20)
    {
        return GColorRed;
    }

    if (level <= 40)
    {
        return GColorChromeYellow;
    }

    return primary_color_for_theme(theme);
}
