/**
 * Specs for the module screenshot lookup.
 *
 * Two levels of index with nothing guarding either one, called for every cell
 * both builders draw. A module the thumbnail pass skipped has no entry at all,
 * so the miss path is the one that decides between an emoji fallback and a
 * builder that throws before it renders anything.
 */

import { describe, test, expect } from 'vitest';
import { thumbByLabel } from './thumbs';

const thumbs = {
  'Battery': { '1x2': 'data:battery-1x2', '2x2': 'data:battery-2x2' },
  'Weather': { '1x2': '' },
};

describe('thumbByLabel', () => {
  /** Reading the wrong size hands the cell another shot, so a 1x2 block renders the 2x2 art. */
  test('returns the shot filed under the label at that size', () => {
    const result = thumbByLabel(thumbs, 'Battery', '1x2');

    expect(result).toBe('data:battery-1x2');
  });

  /** A module the thumbnail pass skipped has no entry, and indexing into it would throw before the builder draws a single cell. */
  test('returns null for a label with no shots at all', () => {
    const result = thumbByLabel(thumbs, 'Stocks', '1x2');

    expect(result).toBeNull();
  });

  /** A module shot only at some sizes has to fall back per size, or the missing ones render a broken img. */
  test('returns null when the label carries no shot at that size', () => {
    const result = thumbByLabel(thumbs, 'Battery', '1x4');

    expect(result).toBeNull();
  });

  /** An empty data url is falsy but not null, and setting img.src to it reloads the config page into the cell. */
  test('returns null rather than the empty string for a blank entry', () => {
    const result = thumbByLabel(thumbs, 'Weather', '1x2');

    expect(result).toBeNull();
  });
});
