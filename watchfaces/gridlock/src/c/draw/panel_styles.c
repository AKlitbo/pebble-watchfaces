/**
 * @file panel_styles.c
 * @brief The panel style table. The order matches the Panel Style select in config.ts.
 * @ingroup gridlock_draw
 */
#include "draw/panel_styles.h"

// index 0 is Classic, the historic square panel. Rounded keeps the whole header as it was and only
// softens the four corners, so the label block rounds its top outer corner to sit inside the curve
static const uint8_t s_panel_radius[PANEL_STYLE_COUNT] = {
    0, // Classic (default)
    5, // Rounded
};

uint8_t panel_style_radius(uint8_t choice)
{
    if (choice >= PANEL_STYLE_COUNT)
    {
        choice = 0;
    }

    return s_panel_radius[choice];
}
