/**
 * Specs for the VIBRANT colour generator.
 *
 * resolveColors follows a module's alias to whichever module owns the family colour, which
 * is exactly where the Dew Point bug lived. validate is the guard that turns a palette typo
 * into a failed build instead of a box on the watch. The stale check asserts the committed
 * vibrant_table.g.h and vibrant.g.js still match what the generator produces, so the
 * firmware table and the config preview can never drift apart again.
 */

import { describe, test, expect } from 'vitest';
import fs from 'fs';

// the C table is shared, the JS map is written per face. gridlock is the family's reference
const FACE = 'gridlock';
import {
  CATALOG_H,
  SOURCE_JSON,
  OUT_C,
  outJs,
  parseModuleOrder,
  paletteNameSet,
  resolveColors,
  validate,
  buildHeader,
  buildJsMap,
} from './generate-vibrant';

const source = JSON.parse(fs.readFileSync(SOURCE_JSON, 'utf8'));
const order = parseModuleOrder(fs.readFileSync(CATALOG_H, 'utf8'));
const paletteNames = paletteNameSet();

describe('resolveColors', () => {
  /** A family member that kept its own colour would light the wrong swatch in the theme editor. */
  test('follows an alias to the family primary colour', () => {
    const aliased = { 'MOD_A': { color: 'Red' }, 'MOD_B': { alias: 'MOD_A' } };

    const result = resolveColors(aliased, 'MOD_B');

    expect(result).toEqual({ accent: 'Red', value: 'Red', icon: 'Red', subtitle: null });
  });

  /** Dew Point shared Precipitation's family: a stray own-colour once made the preview disagree with the watch. */
  test('resolves Dew Point to Precipitation Vivid Cerulean', () => {
    const result = resolveColors(source, 'MOD_WEATHER_DEW_POINT');

    expect(result.accent).toBe('Vivid Cerulean');
  });

  /** The shorthand and per-channel forms must land the same four channels the C table reads. */
  test('spreads the color shorthand across accent value and icon and leaves subtitle mono', () => {
    const result = resolveColors({ 'MOD_A': { color: 'Orange' } }, 'MOD_A');

    expect(result).toEqual({ accent: 'Orange', value: 'Orange', icon: 'Orange', subtitle: null });
  });
});

describe('validate', () => {
  /** A palette typo would compile in C as an unknown GColor or render a box in the editor. */
  test('rejects a colour that is not a Pebble-64 name', () => {
    const bad = { 'MOD_X': { color: 'Not A Colour' } };

    const call = () => validate(['MOD_X'], bad, paletteNames);

    expect(call).toThrow(/unknown palette colour/);
  });

  /** A module added to the enum but forgotten in the source would silently render mono. */
  test('rejects a module missing from the source', () => {
    const call = () => validate(['MOD_X'], {}, paletteNames);

    expect(call).toThrow(/missing from module-vibrant.json/);
  });

  /** A leftover source key after an enum rename hides a real mismatch, so it must fail loudly. */
  test('rejects a source key that is not a module', () => {
    const call = () => validate([], { 'MOD_GHOST': {} }, paletteNames);

    // anchored so the message cannot go back to leading with a stringified entry
    expect(call).toThrow(/^unknown module MOD_GHOST in module-vibrant\.json$/);
  });
});

describe('generated files are current', () => {
  /** A stale C table means the watch paints an old colour that the source no longer describes. */
  test('vibrant_table.g.h matches the generator output', () => {
    const result = buildHeader(order, source);

    expect(result).toBe(fs.readFileSync(OUT_C, 'utf8'));
  });

  /** A stale JS map means the config editor previews a colour the watch will not paint. */
  test('vibrant.g.js matches the generator output', () => {
    const result = buildJsMap(order, source);

    expect(result).toBe(fs.readFileSync(outJs(FACE), 'utf8'));
  });
});
