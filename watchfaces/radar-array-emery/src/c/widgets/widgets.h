/**
 * @file widgets.h
 * @brief Painted overlays for the radar face, exposed as stateless draws the engine's
 * overlays draw-slot calls. The bluetooth glyph comes from the shared icon cache and the
 * engine owns the layer.
 *
 * @ingroup watchface-radar
 */
#pragma once
#include <pebble.h>

/** @addtogroup watchface-radar @{ */

/**
 * @brief Draw the static TEMP / PULSE / RANGE captions in the panel accent.
 *
 * @param ctx The graphics context.
 */
void widgets_draw_labels(GContext *ctx);

/**
 * @brief Draw the battery gauge: an accent outline with five level-coloured segments.
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

/** @} */
