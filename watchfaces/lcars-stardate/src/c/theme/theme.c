/**
 * @file theme.c
 * @brief Per-theme lookups: background resource ids and panel accent colours.
 */
#include "theme.h"

uint32_t bg_resource_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:
            return RESOURCE_ID_IMAGE_BG_LOWER_DECKS;
        case 2:
            return RESOURCE_ID_IMAGE_BG_LOWER_DECKS_PADD;
        case 3:
            return RESOURCE_ID_IMAGE_BG_NEMESIS_BLUE;
        case 4:
            return RESOURCE_ID_IMAGE_BG_MONO;
        case 5:
            return RESOURCE_ID_IMAGE_BG_LOWER_DECKS_MONO;
        case 6:
            return RESOURCE_ID_IMAGE_BG_LOWER_DECKS_PADD_MONO;
        default:
            return RESOURCE_ID_IMAGE_BG_CLASSIC;
    }
}

GColor label_color_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 4:  // Classic Mono
        case 5:  // Lower Decks Mono
        case 6:  // PADD Mono
            return GColorLightGray;
        default:
            return GColorChromeYellow;
    }
}

GColor panel_accent_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // harvestgold (#FFAA55)
            return GColorRajah;
        case 2:  // night-rain (#5555AA)
            return GColorLiberty;
        case 3:  // tangerine (#FFAA55)
            return GColorRajah;
        case 4:  // Classic Mono (white)
            return GColorWhite;
        case 5:  // Lower Decks Mono (white)
            return GColorWhite;
        case 6:  // PADD Mono (white)
            return GColorWhite;
        default: // african-violet (#AAAAFF)
            return GColorBabyBlueEyes;
    }
}

GColor battery_fill_for_theme(uint8_t theme, int level)
{
    GColor healthy = panel_accent_for_theme(theme);

    // mono themes keep the gauge grayscale. the lit-segment count already shows the
    // charge so the red/amber low-battery warnings would only break the monochrome look
    switch (theme)
    {
        case 4:  // Classic Mono
        case 5:  // Lower Decks Mono
        case 6:  // PADD Mono
            return healthy;
        default:
            break;
    }

    if (level <= 20)
    {
        return GColorRed;
    }

    if (level <= 40)
    {
        return GColorChromeYellow;
    }

    return healthy;
}
