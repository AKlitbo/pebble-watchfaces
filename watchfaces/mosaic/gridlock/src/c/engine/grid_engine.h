/**
 * @file grid_engine.h
 * @brief The drawing engine. It gives every slot of the current layout its own clipped
 * layer so a module can only paint inside its own cell and never spills into the cells
 * next to it. Each layer paints the usual panel (border, header, value, subtitle, icon)
 * or hands the whole frame to the module when it sets a draw override. The hubs own the
 * readings. When a reading changes the engine just repaints.
 *
 * @ingroup gridlock_engine
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup gridlock_engine
 * @{
 */

#include "mosaic/engine/catalog.h"
#include "draw/fonts.h"   // the face's size-named font slots (FONT_TEKO_26, ...) for every module
#include "ui/engine/engine.h"  // the shared slot engine this grid sits on (EngineSlot/EngineBuild)

// shared content inset: how far panel text/icons sit in from the tile's left and right
// edges. one global knob so every standard panel keeps the same margins
#define PANEL_PAD 4

/**
 * @brief The bundle of stuff handed to a module when it draws. It carries the graphics
 * context, the worked-out rects (the whole cell and the body), the footprint size, and
 * the current theme colours.
 */
typedef struct GridCtx {
    GContext *ctx;
    GRect bounds;          ///< The whole area of the cell
    GRect body;            ///< The area under the header where modules draw
    ModuleSize size;       ///< The exact HxW footprint
    GColor color_value;    ///< The big readout colour
    GColor color_subtitle; ///< The caption line colour
    GColor color_accent;   ///< The header block and border colour
    GColor color_icon;     ///< Icon colour. White for mono, its own colour for vibrant
    GColor color_label;    ///< Header label text. Flips black or white to stay readable on the accent block
    bool   borderless;     ///< True when the outer border is skipped, so edge insets drop the border pixel
    bool   headerless;     ///< True when the header strip is dropped, so the module owns the taller tile
} GridCtx;

// pixel-exact content pinned to an edge (icons, gauges) sits one pixel further in than
// PANEL_PAD so it clears the border and lands the same visible gap the text gets (text
// carries its own font left-bearing). border-independent on purpose: toggling the border
// off must not shift the content, so borderless panels keep the same inset
#define EDGE_PAD(gctx) (PANEL_PAD + 1)

/**
 * @brief Works out where to put a box inside the cell's body.
 *
 * @param gctx The grid context. It carries the body size.
 * @param size How big the thing you want to place is.
 * @param anchor Which corner or edge to line it up with (e.g. GAlignBottomRight, GAlignCenter).
 * @param dx Nudge left or right. Positive goes right and negative goes left.
 * @param dy Nudge up or down. Positive goes down and negative goes up.
 * @return The box position it landed on.
 */
GRect grid_anchor(GridCtx *gctx, GSize size, GAlign anchor, int dx, int dy);

/**
 * @brief Builds the Gridlock grid as draw-slots on the shared engine: one slot per layout
 * cell, each painting its module's panel. Handed to engine_init as the EngineBuild callback,
 * so engine_init/deinit/rebuild/mark_dirty all come from the shared engine (ui/engine/engine.h).
 *
 * @param out The slot list to fill.
 * @param max The most slots it may write.
 * @param bounds The window's root bounds (unused: the layout works in absolute cell rects).
 * @return The number of slots written.
 */
uint8_t gridlock_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Frees the engine's cached checker bitmaps (the header dither). Call from the face's
 * deinit for symmetric cleanup. The OS reclaims them on unload regardless.
 */
void gridlock_engine_cleanup(void);

/** @} */
