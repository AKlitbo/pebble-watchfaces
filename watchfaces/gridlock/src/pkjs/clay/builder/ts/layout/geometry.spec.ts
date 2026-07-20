/**
 * Specs for the layout grid math.
 *
 * jsdom cannot exercise real drop coordinates (every rect is zeros), so the
 * placement rules the drag engine leans on are pinned here instead.
 */

import { describe, test, expect } from 'vitest';
import { occupancyGrid, canPlace, snapDrop, sizeKey } from './geometry';

describe('canPlace', () => {
  /** A half width block accepted mid row would overlap its neighbour on the watch. */
  test('allows a 2 wide block only at columns 0 and 2', () => {
    expect(canPlace([], 0, 0, 2, 1)).toBe(true);
    expect(canPlace([], 0, 2, 2, 1)).toBe(true);
    expect(canPlace([], 0, 1, 2, 1)).toBe(false);
    expect(canPlace([], 0, 3, 2, 1)).toBe(false);
  });

  /** A full width block accepted off column 0 would hang past the grid edge. */
  test('allows a 4 wide block only at column 0', () => {
    expect(canPlace([], 0, 0, 4, 1)).toBe(true);
    expect(canPlace([], 0, 2, 4, 1)).toBe(false);
  });

  /** Accepting a spot past the edges would place a block the watch cannot draw. */
  test('rejects spots that fall off the grid', () => {
    expect(canPlace([], -1, 0, 2, 1)).toBe(false);
    expect(canPlace([], 4, 0, 2, 2)).toBe(false);
    expect(canPlace([], 3, 0, 2, 2)).toBe(true);
  });

  /** Two blocks on the same cells would draw on top of each other. */
  test('rejects a spot another block covers', () => {
    const blocks = [{ module: 2, row: 0, col: 0, w: 2, h: 2 }];

    expect(canPlace(blocks, 1, 0, 2, 1)).toBe(false);
    expect(canPlace(blocks, 2, 0, 2, 1)).toBe(true);
  });

  /** Without the self exclusion a block could never drop back onto its own cells. */
  test('ignores the block being moved via ignoreIdx', () => {
    const blocks = [{ module: 2, row: 0, col: 0, w: 2, h: 2 }];

    expect(canPlace(blocks, 0, 0, 2, 2, 0)).toBe(true);
  });
});

describe('occupancyGrid', () => {
  /** A block marked on the wrong cells would let overlapping drops through. */
  test('marks every cell a block spans with its index', () => {
    const blocks = [
      { module: 2, row: 0, col: 2, w: 2, h: 2 },
      { module: 1, row: 4, col: 0, w: 4, h: 1 },
    ];

    const grid = occupancyGrid(blocks);

    expect(grid[0]).toEqual([null, null, 0, 0]);
    expect(grid[1]).toEqual([null, null, 0, 0]);
    expect(grid[4]).toEqual([1, 1, 1, 1]);
  });
});

describe('snapDrop', () => {
  /** A drop snapped to the wrong column would place the block somewhere the user did not aim. */
  test('snaps the pointer cell to the block start column', () => {
    expect(snapDrop({ r: 0, c: 1 }, 2, 1)).toEqual({ row: 0, col: 0 });
    expect(snapDrop({ r: 0, c: 3 }, 2, 1)).toEqual({ row: 0, col: 2 });
    expect(snapDrop({ r: 0, c: 3 }, 4, 1)).toEqual({ row: 0, col: 0 });
  });

  /** A tall block dropped on the bottom row must pull up instead of hanging off. */
  test('pulls the row up so the block stays on the grid', () => {
    const result = snapDrop({ r: 4, c: 0 }, 2, 2);

    expect(result).toEqual({ row: 3, col: 0 });
  });
});

describe('sizeKey', () => {
  /** A wrong size key would fetch the wrong screenshot for a block. */
  test('maps each block shape to its size key', () => {
    expect(sizeKey(2, 1)).toBe('1x2');
    expect(sizeKey(2, 2)).toBe('2x2');
    expect(sizeKey(4, 1)).toBe('1x4');
    expect(sizeKey(4, 2)).toBe('2x4');
    expect(sizeKey(3, 1)).toBeNull();
  });
});
