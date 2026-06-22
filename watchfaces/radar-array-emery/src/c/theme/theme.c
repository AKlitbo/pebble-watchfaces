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
        default: // Default
            return GColorGreen;
    }
}
