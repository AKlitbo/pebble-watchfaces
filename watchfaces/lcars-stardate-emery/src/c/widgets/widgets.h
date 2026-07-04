/**
 * @file widgets.h
 * @brief Overlay painters for the LCARS face, exposed as stateless draws the engine's
 * overlays draw-slot calls. This module owns the label font while the glyphs come from
 * the shared icon cache. The engine owns the layer.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/** @addtogroup watchface-lcars @{ */

/** @brief Load the label font. */
void widgets_load(void);

/** @brief Free the label font. */
void widgets_unload(void);

/**
 * @brief Draw the four LCARS bar labels (STARDATE / SENSORS / VITALS / TRAVERSAL).
 *
 * @param ctx The graphics context.
 */
void widgets_draw_labels(GContext *ctx);

/**
 * @brief Draw the battery gauge on the top-left block.
 *
 * @param ctx The graphics context.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, int level);

/**
 * @brief Draw the weather / thermometer / heart / feet glyphs, plus the bluetooth glyph.
 *
 * The glyphs come from the shared icon cache, so each one loads a single time.
 *
 * @param ctx The graphics context.
 * @param condition The weather condition token (drives the weather glyph).
 * @param bt_show True to draw the bluetooth glyph (the show/hide setting).
 * @param bt_connected True when the phone link is up (connected vs slashed glyph).
 */
void widgets_draw_glyphs(GContext *ctx, const char *condition, bool bt_show, bool bt_connected);

/** @} */
