/**
 * @file widgets.h
 * @brief Overlay painters for the VS Code face, exposed as stateless draws the engine's
 * overlays draw-slot calls: the battery percentage/icon, the weather condition glyph
 * (swapped per condition and tinted to the theme), and the bluetooth glyph. The glyphs
 * come from the shared icon cache and the engine owns the layer.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

/** @addtogroup watchface-vscode @{ */

/**
 * @brief Draw the battery icon and/or percentage, per the Battery Display setting.
 *
 * @param ctx The graphics context.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, int level);

/**
 * @brief Draw the bluetooth glyph for the link state (connected vs slashed).
 *
 * @param ctx The graphics context.
 * @param connected True when the phone link is up.
 */
void widgets_draw_bt(GContext *ctx, bool connected);

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
