// per-theme lookups: background resource ids and panel accent colors
#include "theme.h"

// maps the Theme setting to its baked background resource
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
        default:
            return RESOURCE_ID_IMAGE_BG_CLASSIC;
    }
}

// the top-left cap color the battery gauge is painted over, per theme. 
// these mirror each frame's `.sp-top` cap class
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
        default: // african-violet (#AAAAFF)
            return GColorBabyBlueEyes;
    }
}
