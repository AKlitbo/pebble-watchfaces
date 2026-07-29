/**
 * @file widgets.h
 * @brief The painted chrome: the battery gauge, the bluetooth glyph, and the three little
 * marks that label the stats row.
 *
 * All stateless draws the scene's overlay slot calls. The bluetooth glyph comes from the
 * shared icon cache and the engine owns the layer.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

/**
 * @brief Draw the battery gauge: an outline with five level-coloured segments.
 *
 * @param ctx The graphics context.
 * @param ink The colour to draw it in when the charge is healthy.
 * @param theme The theme setting value, which decides whether the low-charge warning colours apply.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, GColor ink, uint8_t theme, int level);

/**
 * @brief Paint the dark strip the chrome sits on, across the top of the screen.
 *
 * The scene underneath can be anything from white paper to a black night, so the chrome gets
 * its own ground rather than having to read against all of it.
 *
 * @param ctx The graphics context.
 * @param bounds The window's root bounds.
 */
void widgets_draw_top_bar(GContext *ctx, GRect bounds);

/**
 * @brief Draw the bluetooth glyph for the link state (connected vs slashed).
 *
 * @param ctx The graphics context.
 * @param connected True when the phone link is up.
 */
void widgets_draw_bt(GContext *ctx, bool connected);

/**
 * @brief Draw the stats row's marks: a thermometer, a heart, and a pair of tracks.
 *
 * The bundled glyphs are white masters, so each is recoloured to the palette on the way out
 * and reads in the same ink as the number beside it.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void widgets_draw_stat_glyphs(GContext *ctx, const Palette *pal);

/**
 * @brief Draw the AM/PM marker in the clock's top-right corner.
 *
 * Painted rather than given its own zone: the clock is centred, so its right edge moves with
 * the time format, and the marker measures it each paint to stay tucked against the last
 * digit. Draws nothing on a 24-hour or .beats clock.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 */
void widgets_draw_meridiem(GContext *ctx, GColor color);

/**
 * @brief Draw the muted-speaker glyph that marks Quiet Time, left of the bluetooth glyph.
 *
 * The caller decides whether Quiet Time is on and whether the user asked to see it, so this
 * just paints.
 *
 * @param ctx The graphics context.
 * @param color The colour to paint it.
 */
void widgets_draw_qt(GContext *ctx, GColor color);

/** @} */
