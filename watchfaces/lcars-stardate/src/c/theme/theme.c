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
        case 7:
            return RESOURCE_ID_IMAGE_BG_VOYAGER;
        case 8:
            return RESOURCE_ID_IMAGE_BG_VOYAGER_MONO;
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
        case 8:  // Voyager Mono
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
        case 7:  // Voyager (command gold #FFAA00)
            return GColorChromeYellow;
        case 8:  // Voyager Mono (white)
            return GColorWhite;
        default: // african-violet (#AAAAFF)
            return GColorBabyBlueEyes;
    }
}

// the two bar colours per theme. they match the frame artwork so a drawn bar sits
// on the same pixels as the chrome around it
// every value is a real Pebble-64 colour because that is all the frame
// stylesheets carry
GColor bar_cap_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Lower Decks
            return GColorRajah;
        case 2:  // PADD
            return GColorCadetBlue;
        case 3:  // Nemesis Blue
            return GColorBlue;
        case 4:  // Classic Mono
            return GColorDarkGray;
        case 5:  // Lower Decks Mono
            return GColorWhite;
        case 6:  // PADD Mono
            return GColorDarkGray;
        case 7:  // Voyager
            return GColorBlueMoon;
        case 8:  // Voyager Mono
            return GColorDarkGray;
        default: // Classic
            return GColorLavenderIndigo;
    }
}

GColor bar_body_for_theme(uint8_t theme)
{
    switch (theme)
    {
        case 1:  // Lower Decks
            return GColorOrange;
        case 2:  // PADD
            return GColorLiberty;
        case 3:  // Nemesis Blue
            return GColorBlueMoon;
        case 4:  // Classic Mono
            return GColorWhite;
        case 5:  // Lower Decks Mono
            return GColorDarkGray;
        case 6:  // PADD Mono
            return GColorDarkGray;
        case 7:  // Voyager
            return GColorLiberty;
        case 8:  // Voyager Mono
            return GColorLightGray;
        default: // Classic
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
        case 8:  // Voyager Mono
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
