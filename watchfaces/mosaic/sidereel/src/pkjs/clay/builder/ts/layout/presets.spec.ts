/**
 * Specs for the preset layouts.
 *
 * The presets are hand written wire strings, so nothing else guarantees they
 * are actually placeable. These specs prove each one survives the parser
 * untouched and lays out without overlaps.
 */

import { describe, test, expect } from 'vitest';
import { LAYOUT_PRESETS } from './presets';
import { serializeLayout, parseLayoutString } from './codec';
import { canPlace } from './geometry';

describe('LAYOUT_PRESETS', () => {
  /** An empty table would leave the preset buttons doing nothing at all. */
  test('ships a preset for every button, including the default', () => {
    const result = Object.keys(LAYOUT_PRESETS);

    expect(result).toContain('default');
    expect(result.length).toBeGreaterThan(1);
  });

  Object.keys(LAYOUT_PRESETS).forEach((presetId) => {
    /** A preset the parser has to fix up would load differently than its author intended. */
    test(`preset "${presetId}" parses back without corrections`, () => {
      const blocks = parseLayoutString(LAYOUT_PRESETS[presetId]);

      const result = serializeLayout(blocks);

      const normalized = LAYOUT_PRESETS[presetId].split(';').sort().join(';');
      expect(result.split(';').sort().join(';')).toBe(normalized);
    });

    /** An overlapping preset would stack two panels on the same cells with one tap. */
    test(`preset "${presetId}" places every block without overlap`, () => {
      const blocks = parseLayoutString(LAYOUT_PRESETS[presetId]);

      blocks.forEach((block, index) => {
        const others = blocks.filter((candidate, otherIndex) => otherIndex !== index);

        const result = canPlace(others, block.row, block.col, block.w, block.h);

        expect(result).toBe(true);
      });
    });
  });
});
