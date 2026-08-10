/**
 * @file layouts.c
 * @brief Places the user's blocks into worked-out cells.
 *
 * Geometry: the Emery is 200x228. The outer margin is 4 and the column gutter is 4, so the
 * split columns are 94px wide at x=4 and x=102. A full-width cell is 192px at x=4. The layout
 * is a free grid GRIDLOCK_ROWS rows tall: each grid row is ROW_UNIT tall with ROW_GAP between
 * rows. A block h rows tall spans h units plus the gaps it crosses, so a tall block lines up
 * with two short ones next to it. The used block extent is centred up and down. The last pixel
 * tweaks happen in the emulator.
 * @ingroup gridlock_engine
 */
#include "engine/layouts.h"

#include "settings_schema.h"

#define SCREEN_W 200
#define SCREEN_H 228
#define MARGIN 4
#define GUTTER 4

#define COL_W ((SCREEN_W - MARGIN * 2 - GUTTER) / 2) // 94
#define FULL_W (SCREEN_W - MARGIN * 2)               // 192
#define COL_L MARGIN                                 // 4
#define COL_R (MARGIN + COL_W + GUTTER)              // 102

#define ROW_UNIT 40 // one grid row's height
#define ROW_GAP 3   // gap between grid rows. a tall block absorbs the gaps it spans

/**
 * @brief The exact HxW footprint of a block, from its grid width and height.
 *
 * @param w The block width (2 half, 4 full).
 * @param h The block height (1 short, 2 tall).
 * @return The module size.
 */
static ModuleSize block_msize(uint8_t w, uint8_t h)
{
    bool full = (w >= 4);
    bool tall = (h >= 2);

    if (full)
    {
        return tall ? MSIZE_2x4 : MSIZE_1x4;
    }

    return tall ? MSIZE_2x2 : MSIZE_1x2;
}

// whether a block actually reaches the screen. a module placed at a footprint it does not allow
// is dropped, so a broken string can never draw a module at a size it was not built for
static bool block_is_drawable(const GridlockBlock *block)
{
    return module_allows_size(block->module, block_msize(block->w, block->h));
}

uint8_t layouts_build(ResolvedSlot *out, uint8_t max)
{
    uint8_t n = gridlock_block_count();

    // work out the vertical extent of the placed blocks so the whole thing can be centred,
    // keeping any gaps the user left between rows. only blocks that will really draw count,
    // otherwise a dropped one still reserves its rows and shoves everything else off centre
    int top = GRIDLOCK_ROWS;
    int bottom = 0;
    for (uint8_t i = 0; i < n; i++)
    {
        const GridlockBlock *block = gridlock_block(i);
        if (!block_is_drawable(block))
        {
            continue;
        }
        if (block->row < top)
        {
            top = block->row;
        }
        if (block->row + block->h > bottom)
        {
            bottom = block->row + block->h;
        }
    }

    if (bottom <= top)
    {
        return 0; // an empty layout draws nothing
    }

    int used = bottom - top;
    int total = used * ROW_UNIT + (used - 1) * ROW_GAP;
    int base_y = (SCREEN_H - total) / 2;
    if (base_y < MARGIN)
    {
        base_y = MARGIN;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < n && count < max; i++)
    {
        const GridlockBlock *block = gridlock_block(i);
        ModuleSize msize = block_msize(block->w, block->h);

        if (!block_is_drawable(block))
        {
            continue;
        }

        int w = block->w >= 4 ? FULL_W : COL_W;
        // a full-width block spans both columns so it always starts at the left,
        // whatever column a broken string parsed. row.h overflow is clamped in
        // settings_schema, but nothing bounds the column, so a right-column full
        // block would otherwise land its whole panel off the right edge
        bool full_width = w >= FULL_W;
        bool right_col = !full_width && block->col >= 2;
        int x = right_col ? COL_R : COL_L;
        int y = base_y + (block->row - top) * (ROW_UNIT + ROW_GAP);
        int h = block->h * ROW_UNIT + (block->h - 1) * ROW_GAP;

        out[count++] = (ResolvedSlot){
            .frame = GRect(x, y, w, h),
            .msize = msize,
            .module = block->module,
        };
    }

    return count;
}
