/**
 * Specs for the Pebble 64 palette.
 *
 * The CSV is the one palette source in the repo (the vibrant generator reads
 * its names from here too), so its shape and the argb math are pinned.
 */

import { describe, test, expect } from 'vitest';
import { PEBBLE_COLORS_CSV, buildPalette, buildArgbByName, argbToCss } from './palette';

const palette = buildPalette(PEBBLE_COLORS_CSV);

describe('buildPalette', () => {
  /** A missing or doubled entry would shift every palette index the wire string stores. */
  test('has all 64 colours with unique names', () => {
    const names = new Set(palette.map((entry) => entry.name));

    expect(palette.length).toBe(64);
    expect(names.size).toBe(64);
  });

  /** Wrong argb math would make the config preview disagree with what the watch paints. */
  test('maps the corner colours to their GColor8 bytes', () => {
    const byName = buildArgbByName(palette);

    expect(byName.Black).toBe(192);
    expect(byName.White).toBe(255);
    expect(byName.Red).toBe(240);
    expect(byName.Green).toBe(204);
    expect(byName.Blue).toBe(195);
  });

  /** An index that is not the argb offset would desync the picker grid from the wire chars. */
  test('lists entries in argb order so index plus 192 is the byte', () => {
    palette.forEach((entry, index) => {
      expect(entry.argb).toBe(192 + index);
    });
  });
});

describe('argbToCss', () => {
  /** A byte that does not paint back to its own hex means a swatch lies about its colour. */
  test('reconstructs every palette entry css from its byte', () => {
    palette.forEach((entry) => {
      expect(argbToCss(entry.argb)).toBe(entry.css);
    });
  });
});
