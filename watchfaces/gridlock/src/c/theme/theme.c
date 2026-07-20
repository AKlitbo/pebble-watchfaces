/**
 * @file theme.c
 * @brief The Gridlock look is white on black. All three themes share this starting
 * palette. VIBRANT adds each panel's own colours on top (sorted out in the grid
 * engine) and CUSTOM will add a colour per cell from the config page once that is
 * built.
 * @ingroup gridlock_settings
 */
#include "theme.h"

Palette palette_for_theme(uint8_t theme)
{
    if (theme == THEME_MONO_INVERSE)
    {
        // the flip side. black on white with a darker grey for the dim caption line
        return (Palette){
            .accent = GColorBlack,
            .value = GColorBlack,
            .subtitle = GColorDarkGray,
            .icon = GColorBlack,
        };
    }

    // every other theme starts out mono. the colour comes from the panel overrides
    // that the engine only paints on under THEME_VIBRANT
    return (Palette){
        .accent = GColorWhite,
        .value = GColorWhite,
        .subtitle = GColorLightGray,
        .icon = GColorWhite,
    };
}

GColor theme_background(uint8_t theme)
{
    return theme == THEME_MONO_INVERSE ? GColorWhite : GColorBlack;
}
