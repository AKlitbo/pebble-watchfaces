/**
 * @file layout_string.h
 * @brief The LAYOUT wire string the drag builder writes, parsed into placed blocks.
 *
 * One block per entry joined with semicolons, each "module,row,col,w,h". It is the same format
 * gridlock uses, so a layout means the same thing on both faces, but this grid is narrower: one
 * column four rows tall, with the hour pointer across the middle. col and w are therefore read
 * and thrown away, and a tall block is pulled back to one side of the pointer rather than
 * allowed to straddle it.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "layout.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/** @brief One placed panel. */
typedef struct
{
    uint8_t module;  /**< A gridlock ModuleType */
    uint8_t row;     /**< Its top row, 0 to GRID_ROWS-1 */
    uint8_t height;  /**< 1 for a short panel, 2 for a tall one */
} SidereelBlock;

/**
 * @brief How many blocks the current layout string places.
 *
 * @return 0 to CELL_COUNT.
 */
uint8_t sidereel_block_count(void);

/**
 * @brief One placed block.
 *
 * @param index Which block, 0 to sidereel_block_count()-1.
 * @return The block, or an empty one when the index is past the end.
 */
SidereelBlock sidereel_block(uint8_t index);

/** @} */
