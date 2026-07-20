/**
 * Specs for the LAYOUT wire string codec.
 *
 * This string is the contract between the builder and the watch's layout
 * parser, so the goldens here pin the exact text either side reads.
 */

import { describe, test, expect } from 'vitest';
import { serializeLayout, parseLayoutString } from './codec';

describe('serializeLayout', () => {
  /** An unsorted wire string would make the same layout save as different texts. */
  test('writes blocks sorted by row then column', () => {
    const blocks = [
      { module: 1, row: 2, col: 0, w: 4, h: 1 },
      { module: 3, row: 0, col: 2, w: 2, h: 1 },
      { module: 2, row: 0, col: 0, w: 2, h: 2 },
    ];

    const result = serializeLayout(blocks);

    expect(result).toBe('2,0,0,2,2;3,0,2,2,1;1,2,0,4,1');
  });

  /** An empty grid has to write the empty string the watch treats as no layout. */
  test('writes an empty string for no blocks', () => {
    const result = serializeLayout([]);

    expect(result).toBe('');
  });
});

describe('parseLayoutString', () => {
  /** A round trip that mutates would corrupt the saved layout on every settings open. */
  test('round trips what serializeLayout wrote', () => {
    const text = '2,0,0,2,2;3,0,2,2,1;1,2,0,4,1';

    const result = serializeLayout(parseLayoutString(text));

    expect(result).toBe(text);
  });

  /** Junk from a hand edited import must not take the good blocks down with it. */
  test('skips empty segments, short segments and module 0', () => {
    const result = parseLayoutString('banana;;0,0,0,2,1;3,1;2,0,0,2,1');

    expect(result).toEqual([{ module: 2, row: 0, col: 0, w: 2, h: 1 }]);
  });

  /** An oversized block from an import would draw past the watch screen edge. */
  test('snaps sizes to the placeable ones', () => {
    const result = parseLayoutString('2,0,0,9,9;3,1,0,1,0');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 4, h: 2 },
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

  /** A half width block starting mid row would overlap its neighbour. */
  test('clamps columns to the valid start columns', () => {
    const result = parseLayoutString('2,0,1,2,1;3,1,3,2,1;1,2,2,4,1');

    expect(result).toEqual([
      { module: 2, row: 0, col: 0, w: 2, h: 1 },
      { module: 3, row: 1, col: 2, w: 2, h: 1 },
      { module: 1, row: 2, col: 0, w: 4, h: 1 },
    ]);
  });
});
