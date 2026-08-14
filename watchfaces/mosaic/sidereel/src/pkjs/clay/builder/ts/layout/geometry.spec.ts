/**
 * Specs for the layout grid math.
 *
 * jsdom cannot exercise real drop coordinates (every rect is zeros), so the
 * placement rules the drag engine leans on are pinned here instead.
 *
 * What makes this face's grid its own is what it cannot use. The reel owns the
 * right half and the hour pointer owns the middle row, so the panel field is a
 * single column of four cells split either side of the pointer. Every case
 * below is really about one of those two edges.
 */

import { describe, test, expect } from 'vitest';
import { occupancyGrid, canPlace, snapDrop, sizeKey, isBlocked, spansPointer } from './geometry';

describe('isBlocked', () => {
  /** The reel is drawn by the face, so a panel allowed onto its columns would be painted straight over. */
  test('blocks every column the reel covers', () => {
    const result = [isBlocked(0, 0), isBlocked(0, 1), isBlocked(0, 2), isBlocked(0, 3)];

    expect(result).toEqual([false, false, true, true]);
  });

  /** The hour pointer runs across the middle row, and a panel there hides the one thing the face is for. */
  test('blocks the pointer row across the panel field', () => {
    const result = [isBlocked(1, 0), isBlocked(2, 0), isBlocked(3, 0)];

    expect(result).toEqual([false, true, false]);
  });
});

describe('spansPointer', () => {
  /** A tall block that reads as clear of the pointer would be drawn through it. */
  test('reports the rows a block of each height would touch', () => {
    const short = [spansPointer(1, 1), spansPointer(2, 1), spansPointer(3, 1)];
    const tall = [spansPointer(0, 2), spansPointer(1, 2), spansPointer(2, 2), spansPointer(3, 2)];

    expect(short).toEqual([false, true, false]);
    expect(tall).toEqual([false, true, true, false]);
  });
});

describe('canPlace', () => {
  /** This face has one column, so a block accepted anywhere else would sit under the minute reel. */
  test('allows a block only at column 0', () => {
    expect(canPlace([], 0, 0, 2, 1)).toBe(true);
    expect(canPlace([], 0, 1, 2, 1)).toBe(false);
    expect(canPlace([], 0, 2, 2, 1)).toBe(false);
  });

  /** A small block on the pointer row would cover the hour hand with a panel. */
  test('rejects a 1 high block on the pointer row', () => {
    expect(canPlace([], 1, 0, 2, 1)).toBe(true);
    expect(canPlace([], 2, 0, 2, 1)).toBe(false);
    expect(canPlace([], 3, 0, 2, 1)).toBe(true);
  });

  /** A big block straddling the pointer is the same bug one row earlier, where the top edge still looks clear. */
  test('rejects a 2 high block that straddles the pointer row', () => {
    expect(canPlace([], 0, 0, 2, 2)).toBe(true);
    expect(canPlace([], 1, 0, 2, 2)).toBe(false);
    expect(canPlace([], 2, 0, 2, 2)).toBe(false);
    expect(canPlace([], 3, 0, 2, 2)).toBe(true);
  });

  /** Accepting a spot past the edges would place a block the watch cannot draw. */
  test('rejects spots that fall off the grid', () => {
    expect(canPlace([], -1, 0, 2, 1)).toBe(false);
    expect(canPlace([], 4, 0, 2, 2)).toBe(false);
    expect(canPlace([], 4, 0, 2, 1)).toBe(true);
  });

  /** Two blocks on the same cells would draw on top of each other. */
  test('rejects a spot another block covers', () => {
    const blocks = [{ module: 2, row: 0, col: 0, w: 2, h: 2 }];

    expect(canPlace(blocks, 1, 0, 2, 1)).toBe(false);
    expect(canPlace(blocks, 3, 0, 2, 1)).toBe(true);
  });

  /** Without the self exclusion a block could never drop back onto its own cells. */
  test('ignores the block being moved via ignoreIdx', () => {
    const blocks = [{ module: 2, row: 0, col: 0, w: 2, h: 2 }];

    const result = canPlace(blocks, 0, 0, 2, 2, 0);

    expect(result).toBe(true);
  });
});

describe('occupancyGrid', () => {
  /** A block marked on the wrong cells would let overlapping drops through. */
  test('marks every cell a block spans with its index', () => {
    const blocks = [
      { module: 2, row: 0, col: 0, w: 2, h: 2 },
      { module: 1, row: 4, col: 0, w: 2, h: 1 },
    ];

    const result = occupancyGrid(blocks);

    expect(result[0]).toEqual([0, 0, null, null]);
    expect(result[1]).toEqual([0, 0, null, null]);
    expect(result[3]).toEqual([null, null, null, null]);
    expect(result[4]).toEqual([1, 1, null, null]);
  });
});

describe('snapDrop', () => {
  /** There is one column, so a drop that kept the pointer's column would place the block over the reel. */
  test('snaps every drop back to the one column', () => {
    const result = snapDrop({ r: 0, c: 3 }, 2, 1);

    expect(result).toEqual({ row: 0, col: 0 });
  });

  /** A big block has only two homes, and a drop aimed at the top half must take the upper one. */
  test('sends a 2 high block dropped above the pointer to the top half', () => {
    expect(snapDrop({ r: 0, c: 0 }, 2, 2)).toEqual({ row: 0, col: 0 });
    expect(snapDrop({ r: 1, c: 0 }, 2, 2)).toEqual({ row: 0, col: 0 });
  });

  /** And one aimed at or below the pointer must take the lower one rather than jumping the pointer. */
  test('sends a 2 high block dropped on or below the pointer to the bottom half', () => {
    expect(snapDrop({ r: 2, c: 0 }, 2, 2)).toEqual({ row: 3, col: 0 });
    expect(snapDrop({ r: 3, c: 0 }, 2, 2)).toEqual({ row: 3, col: 0 });
  });

  /** A small block dropped on the pointer has to go somewhere, and silently landing on it would draw over the hour hand. */
  test('walks a 1 high block off the pointer row', () => {
    const result = snapDrop({ r: 2, c: 0 }, 2, 1);

    expect(result).toEqual({ row: 3, col: 0 });
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
  });

  /** The reel owns the right half, so a full width shape has no key here and must not borrow a half width one. */
  test('has no key for a shape this face cannot place', () => {
    expect(sizeKey(4, 1)).toBeNull();
    expect(sizeKey(4, 2)).toBeNull();
    expect(sizeKey(3, 1)).toBeNull();
  });
});
