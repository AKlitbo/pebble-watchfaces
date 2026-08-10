/**
 * @file grid_engine.h
 * @brief The context a panel body draws into.
 *
 * Everything a module body touches matches gridlock's context field for field, so a panel copied
 * across needs no edit. What that face's layout builder fills in has no counterpart here: there
 * is no grid, just four fixed cells.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

#include "mosaic/engine/catalog.h"
#include "draw/fonts.h"   // the face's size-named font slots (FONT_TEKO_26, ...) for every module
#include "ui/engine/engine.h"  // the shared slot engine this sits on (EngineSlot/EngineBuild)

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
    GColor color_icon;     ///< Icon colour
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

/** @} */
