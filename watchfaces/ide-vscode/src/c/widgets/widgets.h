/**
 * @file widgets.h
 * @brief Overlay painters for the VS Code face, exposed as stateless draws the engine's
 * overlays draw-slot calls: the weather condition glyph (swapped per condition and tinted to the
 * theme), and the battery, bluetooth, and Quiet Time status. The glyphs come from the shared icon
 * cache and the engine owns the layer.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-vscode
 * @{
 */

/**
 * @brief Draw the tab strip's status: the battery, then bluetooth, then Quiet Time.
 *
 * They pack leftwards from the battery cell, one gap apart, and whatever is hidden closes up.
 * Battery Display hides the cell or the percentage, Show Connection Icon hides bluetooth, and the
 * Quiet Time glyph only shows while Quiet Time is on. Nothing wakes the face when Quiet Time
 * starts or ends, so the glyph follows it on the next minute's repaint.
 *
 * @param ctx The graphics context.
 * @param level Battery charge level percentage.
 * @param connected True when the phone link is up.
 */
void widgets_draw_status(GContext *ctx, int level, bool connected);

/**
 * @brief Draw the weather condition glyph, tinted to the theme's stats colour.
 *
 * The glyph comes from the shared icon cache, so each condition's picture loads once.
 *
 * @param ctx The graphics context.
 * @param condition The weather condition token.
 */
void widgets_draw_weather(GContext *ctx, const char *condition);

/** @} */
