/**
 * @file theme.h
 * @brief The face's colours, in two halves.
 *
 * Panels use gridlock's Palette, because the panel bodies are that face's files and read it
 * through GridCtx. Everything sidereel draws itself (the reel, the pointer, the day track) uses
 * Chrome, which is picked per theme rather than per setting: a theme names the whole look, so a
 * red pointer and a colourful panel row belong to the same choice. The reel and pointer colours
 * are the exception: four settings of their own can paint over whichever theme is running.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief The four themes. The order is the number saved to settings, so only ever append.
 *
 * MONO is white on black. MONO_INVERSE flips that. VIBRANT starts from mono and lets each panel
 * take its own colours from the generated table, and gives the face its red pointer. CUSTOM
 * starts from mono and lets the config page set each panel's channels itself, so a channel left
 * alone stays mono.
 */
typedef enum
{
    THEME_MONO = 0,
    THEME_VIBRANT,
    THEME_CUSTOM,
    THEME_MONO_INVERSE,
    THEME_COUNT,
} SidereelTheme;

/** @brief The colours one panel paints with. Matches gridlock's, field for field. */
typedef struct
{
    GColor accent;   ///< The panel chrome colour
    GColor value;    ///< The big number readout
    GColor subtitle; ///< The small line under a value
    GColor icon;     ///< The icon colour
} Palette;

/** @brief The colours the face itself paints with, outside the panels. */
typedef struct
{
    GColor background;   ///< The left field, and the dark end of the day track
    GColor panel;        ///< The reel panel and the perforated strip
    GColor reel_ink;     ///< The minute digits, on the panel
    GColor highlight;    ///< The band behind the centred minute
    GColor pointer;      ///< The pennant fill
    GColor pointer_ink;  ///< The hour digits, on the pennant
    GColor band_day;     ///< The stretch of the day track the sun is up for
    GColor band_night;   ///< The rest of it
    GColor band_now;     ///< The marker riding it at the current time
} Chrome;

/**
 * @brief The starting palette every theme builds on.
 *
 * VIBRANT and CUSTOM stack each panel's own colours on top of this back in panel.c.
 *
 * @param theme The theme number from settings.
 * @return The starting palette.
 */
Palette palette_for_theme(uint8_t theme);

/**
 * @brief The window background colour for a theme.
 *
 * @param theme The theme number from settings.
 * @return The background colour.
 */
GColor theme_background(uint8_t theme);

/**
 * @brief Re-pull the live theme from settings.
 *
 * Call at setup and again whenever a settings message lands, before the rebuild.
 */
void theme_refresh(void);

/**
 * @brief The live face colours.
 *
 * @return The chrome for the current theme, never NULL.
 */
const Chrome *theme_chrome(void);

/** @} */
