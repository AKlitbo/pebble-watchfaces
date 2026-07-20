/**
 * @file theme.h
 * @brief A set of colours for each theme. Gridlock paints everything itself so a theme
 * is really just a handful of colours with no built in background.
 *
 * @ingroup gridlock_settings
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup gridlock_settings
 * @{
 */

/**
 * @brief The three themes. The order is the number we save to settings so only ever
 * add to the end.
 *
 * MONO is white on black, the classic Gridlock look. MONO_INVERSE flips that to black on
 * white. VIBRANT starts from mono and lets each panel pick its own colours from code (the
 * icons get tinted too). CUSTOM starts from the plain mono look and lets the config page
 * set each module's accent value and icon from its own colour key. A channel the user
 * leaves alone stays mono.
 */
typedef enum
{
    THEME_MONO = 0,
    THEME_VIBRANT,
    THEME_CUSTOM,
    THEME_MONO_INVERSE,
    THEME_COUNT,
} GridlockTheme;

/**
 * @brief The colours one theme paints with.
 */
typedef struct
{
    GColor accent;   ///< The panel chrome colour
    GColor value;    ///< The big number readout
    GColor subtitle; ///< The small line under a value (also the dim AM/PM label)
    GColor icon;     ///< The icon colour
} Palette;

/**
 * @brief The starting palette every theme builds on (the mono look). VIBRANT stacks
 * each panel's own colours on top of this back in the engine.
 *
 * @param theme The theme number from settings.
 * @return The starting palette.
 */
Palette palette_for_theme(uint8_t theme);

/**
 * @brief The window background colour for a theme.
 *
 * Every theme paints on black except MONO_INVERSE which paints on white.
 *
 * @param theme The theme number from settings.
 * @return The background colour.
 */
GColor theme_background(uint8_t theme);

/** @} */
