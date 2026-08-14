/**
 * Specs for the preset arrangements.
 *
 * The presets are hand written slot lists in a JSON file, so nothing else
 * guarantees they are actually placeable. A preset the rules have to correct
 * would hand the user a different face than the button promised, and the
 * correction is silent.
 */

import { describe, test, expect } from 'vitest';
import { SLOT_PRESETS } from './presets';
import { sanitize } from './codec';
import { canPlace, SLOT_COUNT } from './geometry';

describe('SLOT_PRESETS', () => {
  /** esbuild inlines the JSON at bundle time, so a table that arrived empty leaves the preset buttons doing nothing. */
  test('ships the default arrangement the reset button puts back', () => {
    const result = Object.keys(SLOT_PRESETS);

    expect(result).toContain('default');
  });

  Object.keys(SLOT_PRESETS).forEach((presetId) => {
    /** A preset with the wrong number of slots would leave a panel undefined for the builder to draw. */
    test(`preset "${presetId}" names every slot`, () => {
      const result = SLOT_PRESETS[presetId].length;

      expect(result).toBe(SLOT_COUNT);
    });

    /** A preset the rules reject would blank a panel the moment it loads, so the button would not do what it says. */
    test(`preset "${presetId}" places every readout where the watch can draw it`, () => {
      const result = SLOT_PRESETS[presetId].map((id, slot) => canPlace(id, slot));

      expect(result).toEqual(new Array(SLOT_COUNT).fill(true));
    });

    /** And it has to survive the read path untouched, or the arrangement shown is not the one stored. */
    test(`preset "${presetId}" comes back from sanitize unchanged`, () => {
      const result = sanitize(SLOT_PRESETS[presetId]);

      expect(result).toEqual(SLOT_PRESETS[presetId]);
    });
  });
});
