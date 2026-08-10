/**
 * The grid math for the layout builder: what sizes exist, which cells a block
 * covers, and whether a block fits where the user wants to drop it.
 *
 * Every fact about the shape of THIS face's grid lives here, so pointing the
 * builder at a different face is a matter of editing this file rather than
 * chasing constants through the drag and render code.
 *
 * The grid is the whole watch, the same four by five gridlock uses, because a
 * builder that only showed the usable corner would not look like the face it
 * builds. Most of it is off limits: the reel owns the right half and the hour
 * pointer owns the middle row. BLOCKED says which cells those are, and one
 * check against it covers both.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { Block, SizeKey } from '../../../../../../../core/pkjs/clay/builder/ts/types';

export const GRID_ROWS = 5;
export const GRID_COLS = 4;

/** The only column a block can start at: the panel field is the left half. */
export const BLOCK_COL = 0;

/** The row the hour pointer runs across, so nothing can be placed on it. */
export const POINTER_ROW = 2;

/** The first column the reel covers. Everything from here right is its own. */
export const REEL_COL = 2;

export const BLOCK_SIZES: Array<{ w: number; h: number; label: string; key: SizeKey }> = [
  { w: 2, h: 1, label: 'Small', key: '1x2' },
  { w: 2, h: 2, label: 'Big', key: '2x2' },
];

/**
 * Whether a cell belongs to something other than the panel field.
 *
 * The right half is the minute reel and the middle row is the hour pointer.
 * Both are drawn by the face itself, so a panel can never sit on either.
 */
export function isBlocked(row: number, col: number): boolean {
  return col >= REEL_COL || row === POINTER_ROW;
}

/** The size key ("1x2" and friends) for a block's size, or null when none matches. */
export function sizeKey(w: number, h: number): SizeKey | null {
  for (let i = 0; i < BLOCK_SIZES.length; i++) {
    if (BLOCK_SIZES[i].w === w && BLOCK_SIZES[i].h === h) {
      return BLOCK_SIZES[i].key;
    }
  }

  return null;
}

/** A row by column map of which block index covers each cell, null for empty. */
export function occupancyGrid(blocks: Block[]): Array<Array<number | null>> {
  const grid: Array<Array<number | null>> = [];
  for (let r = 0; r < GRID_ROWS; r++) {
    grid[r] = [];
    for (let c = 0; c < GRID_COLS; c++) {
      grid[r][c] = null;
    }
  }

  for (let i = 0; i < blocks.length; i++) {
    const block = blocks[i];
    for (let row = block.row; row < block.row + block.h; row++) {
      for (let col = block.col; col < block.col + block.w; col++) {
        if (row < GRID_ROWS && col < GRID_COLS) {
          grid[row][col] = i;
        }
      }
    }
  }

  return grid;
}

/**
 * Whether a block of this size can sit at row/col without falling off the
 * grid, landing on the reel or the pointer, or covering another block.
 * ignoreIdx treats one placed block as empty, which is how a move ignores the
 * block being moved.
 */
export function canPlace(
  blocks: Block[],
  row: number,
  col: number,
  w: number,
  h: number,
  ignoreIdx?: number
): boolean {
  if (row < 0 || col < 0 || row + h > GRID_ROWS || col + w > GRID_COLS) {
    return false;
  }
  if (col !== BLOCK_COL) {
    return false;
  }

  const grid = occupancyGrid(blocks);
  for (let r = row; r < row + h; r++) {
    for (let c = col; c < col + w; c++) {
      if (isBlocked(r, c)) {
        return false;
      }
      if (grid[r][c] !== null && grid[r][c] !== ignoreIdx) {
        return false;
      }
    }
  }

  return true;
}

/**
 * Snaps a raw drop cell to where a block of this size would actually land.
 * There is one column so that is fixed. The row is pulled onto the grid, then
 * off the pointer: a tall block dropped astride it goes to the half it covered
 * more of, and a short one dropped on it goes to the nearer side.
 */
export function snapDrop(target: { r: number; c: number }, w: number, h: number): { row: number; col: number } {
  let row = target.r;

  if (row + h > GRID_ROWS) {
    row = GRID_ROWS - h;
  }

  // walk off the pointer row, upward for the top half and downward for the bottom
  while (row >= 0 && row + h <= GRID_ROWS && spansPointer(row, h)) {
    row = target.r < POINTER_ROW ? row - 1 : row + 1;
  }

  if (row < 0) {
    row = 0;
  }
  if (row + h > GRID_ROWS) {
    row = GRID_ROWS - h;
  }

  return { row: row, col: BLOCK_COL };
}

/** Whether a block covering rows [row, row + h) would touch the pointer row. */
export function spansPointer(row: number, h: number): boolean {
  return row <= POINTER_ROW && row + h > POINTER_ROW;
}
