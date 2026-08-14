/**
 * Specs for the LAYOUT wire string codec.
 *
 * This string is the contract between the builder and the watch's layout
 * parser, so the goldens here pin the exact text either side reads.
 *
 * The parser is also the repair step. A wire string can arrive hand edited, or
 * carried over from a face with a wider grid, so the cases that matter most are
 * the ones where it has to correct a block rather than trust it.
 */

import { describe, test, expect } from 'vitest';
import { serializeLayout, parseLayoutString, EMPTY_LAYOUT } from './codec';

describe('serializeLayout', () => {
  /** An unsorted wire string would make the same layout save as different texts. */
  test('writes blocks sorted by row', () => {
    const blocks = [
      { module: 3, row: 3, col: 0, w: 2, h: 2 },
      { module: 2, row: 0, col: 0, w: 2, h: 1 },
      { module: 17, row: 1, col: 0, w: 2, h: 1 },
    ];

    const result = serializeLayout(blocks);

    expect(result).toBe('2,0,0,2,1;17,1,0,2,1;3,3,0,2,2');
  });

  /**
   * An empty grid has to write something, not nothing.
   *
   * settings_apply_inbox on the watch skips an empty cstring rather than storing it, so a cleared
   * grid that serialised to '' would leave the old layout in place forever. The sentinel is the
   * only way "I cleared this" survives the trip.
   */
  test('writes the sentinel for no blocks, not an empty string', () => {
    const result = serializeLayout([]);

    expect(result).toBe(EMPTY_LAYOUT);
    expect(result).not.toBe('');
  });

  /** And it has to read back as nothing, or clearing would place a phantom block. */
  test('the sentinel parses back to no blocks', () => {
    const result = parseLayoutString(EMPTY_LAYOUT);

    expect(result).toEqual([]);
  });
});

describe('parseLayoutString', () => {
  /** A round trip that mutates would corrupt the saved layout on every settings open. */
  test('round trips what serializeLayout wrote', () => {
    const text = '2,0,0,2,1;17,1,0,2,1;3,3,0,2,2';
    const blocks = parseLayoutString(text);

    const result = serializeLayout(blocks);

    expect(result).toBe(text);
  });

  /** Junk from a hand edited import must not take the good blocks down with it. */
  test('skips empty segments, short segments and module 0', () => {
    const result = parseLayoutString('banana;;0,0,0,2,1;3,1;2,0,0,2,1');

    expect(result).toEqual([{ module: 2, row: 0, col: 0, w: 2, h: 1 }]);
  });

  /** The panel field is one column wide, so a width carried over from a wider face would draw across the reel. */
  test('forces every block to the one width this face draws', () => {
    const result = parseLayoutString('2,0,0,4,1;3,1,0,9,1');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 2, h: 1 },
      { module: 3, row: 1, col: 0, w: 2, h: 1 },
    ]);
  });

  /** An oversized height from an import would draw past the watch screen edge. */
  test('snaps heights to the two placeable ones', () => {
    const result = parseLayoutString('2,0,0,2,9;3,1,0,2,0');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 2, h: 2 },
      { module: 3, row: 1, col: 0, w: 2, h: 1 },
    ]);
  });

  /** A column from a wider face would put the panel under the minute reel. */
  test('forces every block back to the one column', () => {
    const result = parseLayoutString('2,0,3,2,1;3,1,2,2,1');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 2, h: 1 },
      { module: 3, row: 1, col: 0, w: 2, h: 1 },
    ]);
  });

  /** A block hanging off the bottom row would render half a panel on the watch. */
  test('clamps rows so the block stays on the grid', () => {
    const result = parseLayoutString('2,9,0,2,2;3,-1,0,2,1');

    expect(result).toEqual([
      { module: 2, row: 3, col: 0, w: 2, h: 2 },
      { module: 3, row: 0, col: 0, w: 2, h: 1 },
    ]);
  });

  /** A string naming a block on the pointer row is a shape the face cannot draw, and dropping it would lose the panel silently. */
  test('pulls a 1 high block off the pointer row rather than dropping it', () => {
    const result = parseLayoutString('2,2,0,2,1');

    expect(result).toEqual([{ module: 2, row: 3, col: 0, w: 2, h: 1 }]);
  });

  /** A big block straddling the pointer goes to the half it was aimed at, so the repair does not swap top for bottom. */
  test('pulls a 2 high block to the side of the pointer it came from', () => {
    const result = parseLayoutString('2,1,0,2,2;3,2,0,2,2');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 2, h: 2 },
      { module: 3, row: 3, col: 0, w: 2, h: 2 },
    ]);
  });
});
