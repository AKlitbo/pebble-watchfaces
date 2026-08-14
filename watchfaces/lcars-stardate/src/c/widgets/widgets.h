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

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief What the four ops slots resolved to this paint, in slot order (left top, left bottom,
 * right top, right bottom).
 *
 * Layout works this out from the catalog and hands it over, so the painters below stay painters
 * that are told what to draw rather than reaching for settings themselves.
 */
typedef struct
{
    const char *label[4];   ///< Holder-box word per slot, "" for a slot that draws none
    uint32_t    icon[4];    ///< Glyph resource per slot, 0 for none
    bool        left_composite; ///< The left column is the one tall weather block, not two slots
} OpsChrome;

/** @brief Load the label font. */
void widgets_load(void);

/** @brief Free the label font. */
void widgets_unload(void);

/**
 * @brief Draw the LCARS header bars in the current theme's colours.
 *
 * The frame artwork leaves their space empty, so these are painted before anything that sits on
 * top of them (the label holder boxes). A column showing the tall weather block gets one bar
 * across its top row instead of two.
 *
 * @param ctx The graphics context.
 * @param chrome What the four slots resolved to.
 */
void widgets_draw_bars(GContext *ctx, const OpsChrome *chrome);

/**
 * @brief Draw the LCARS bar labels.
 *
 * STARDATE is fixed. The rest come from whatever their slots are set to show, and an empty word
 * leaves its holder box unpainted.
 *
 * @param ctx The graphics context.
 * @param chrome What the four slots resolved to.
 */
void widgets_draw_labels(GContext *ctx, const OpsChrome *chrome);

/**
 * @brief Draw the battery gauge on the top-left block.
 *
 * @param ctx The graphics context.
 * @param level Battery charge level percentage.
 */
void widgets_draw_battery(GContext *ctx, int level);

/**
 * @brief Draw the slot glyphs plus the bluetooth and quiet-time glyphs.
 *
 * A left column showing the tall weather block draws the big condition glyph and the thermometer
 * instead of two slot glyphs. The glyphs come from the shared icon cache, so each one loads a
 * single time.
 *
 * @param ctx The graphics context.
 * @param chrome What the four slots resolved to.
 * @param condition The weather condition token (drives the big weather glyph).
 * @param bt_show True to draw the bluetooth glyph (the show/hide setting).
 * @param bt_connected True when the phone link is up (connected vs slashed glyph).
 * @param qt_show True to allow the quiet-time glyph (the show/hide setting).
 * @param qt_active True while Quiet Time holds (draws the muted speaker; nothing otherwise).
 */
void widgets_draw_glyphs(GContext *ctx, const OpsChrome *chrome, const char *condition,
                         bool bt_show, bool bt_connected, bool qt_show, bool qt_active);

/** @} */
