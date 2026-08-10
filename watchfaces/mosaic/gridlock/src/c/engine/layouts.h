/**
 * @file layouts.h
 * @brief The layout is just a stack of rows from top to bottom that you put together
 * however you like. You pick each row's size and shape and the module or modules in it.
 * The engine stacks the rows and turns them into per-cell slots with real pixel
 * positions.
 *
 * @ingroup gridlock_engine
 */
#pragma once
#include <pebble.h>

#include "mosaic/engine/catalog.h"

/**
 * @addtogroup gridlock_engine
 * @{
 */

/// How many rows the stack supports
#define GRIDLOCK_ROWS 5

/// Stacked-pair rows emit 3 or 4 cells each so a row is not limited to two
/// but the grid only fits 10 half width cells across 5 rows so that caps the total
#define GRIDLOCK_MAX_CELLS (GRIDLOCK_ROWS * 2)

/**
 * @brief One placed panel: which module and where it sits on the grid.
 *
 * The layout is a free grid, GRIDLOCK_ROWS rows tall by two half-width columns. A block picks
 * a top-left cell (row, col) and a size (w by h), so any edge-touching arrangement works and
 * nothing is locked to a fixed row shape. col is 0 (left) or 2 (right). w is 2 (half width)
 * or 4 (full width). h is 1 (short) or 2 (tall).
 */
typedef struct
{
    uint8_t module; ///< Which module sits here (a ModuleType)
    uint8_t row;    ///< Top-left grid row, 0 to GRIDLOCK_ROWS-1
    uint8_t col;    ///< 0 (left) or 2 (right)
    uint8_t w;      ///< 2 half width, 4 full width
    uint8_t h;      ///< 1 short, 2 tall
} GridlockBlock;

/**
 * @brief How tall a cell is. This sets the value font size and how the body is laid out.
 */
typedef enum
{
    CELL_SMALL, ///< Short panel: header and one value line
    CELL_BIG    ///< Tall panel: header, big value, subtitle, and icon
} CellSize;

/**
 * @brief One worked-out cell. Where it sits, how it draws, and what it shows.
 */
typedef struct
{
    GRect      frame;  ///< Where the cell sits in pixels
    ModuleSize msize;  ///< Exact HxW footprint. Used for size rules and pixel tweaks
    ModuleType module; ///< Which module the cell shows
} ResolvedSlot;

/**
 * @brief Builds the list of worked-out cells from the user's row settings.
 *
 * It stacks the rows that are not empty (centred up and down) and gives one
 * ResolvedSlot per cell. Every row uses whatever module it was set to.
 *
 * @param out The array to fill in.
 * @param max How many cells out can hold.
 * @return How many cells got written.
 */
uint8_t layouts_build(ResolvedSlot *out, uint8_t max);

/** @} */
