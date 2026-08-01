/**
 * The grid math for the layout builder: what sizes exist, which cells a block
 * covers, and whether a block fits where the user wants to drop it.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { Block, SizeKey } from '../types';

export const GRID_ROWS = 5;
export const GRID_COLS = 4;

export const BLOCK_SIZES: Array<{ w: number; h: number; label: string; key: SizeKey }> = [
  { w: 2, h: 1, label: 'Small', key: '1x2' },
  { w: 2, h: 2, label: 'Big', key: '2x2' },
  { w: 4, h: 1, label: 'Wide Small', key: '1x4' },
  { w: 4, h: 2, label: 'Wide Big', key: '2x4' },
];

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
    grid[r] = [null, null, null, null];
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
 * grid, starting mid row, or covering another block. ignoreIdx treats one
 * placed block as empty, which is how a move ignores the block being moved.
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
  if (w === 2 && col !== 0 && col !== 2) {
    return false;
  }
  if (w === 4 && col !== 0) {
    return false;
  }

  const grid = occupancyGrid(blocks);
  for (let r = row; r < row + h; r++) {
    for (let c = col; c < col + w; c++) {
      if (grid[r][c] !== null && grid[r][c] !== ignoreIdx) {
        return false;
      }
    }
  }

  return true;
}

/**
 * Snaps a raw drop cell to where a block of this size would actually land.
 * Half width blocks start at column 0 or 2, full width ones at 0, and the
 * row is pulled up so the block stays on the grid.
 */
export function snapDrop(target: { r: number; c: number }, w: number, h: number): { row: number; col: number } {
  let col = target.c < 2 ? 0 : 2;
  if (w === 4) {
    col = 0;
  }

  let row = target.r;
  if (row + h > GRID_ROWS) {
    row = GRID_ROWS - h;
  }

  return { row: row, col: col };
}
