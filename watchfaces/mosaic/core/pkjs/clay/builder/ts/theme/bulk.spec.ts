/**
 * Specs for the every panel colour sweep.
 *
 * The sweep writes one pick into dozens of records at once, so a slip here
 * repaints every panel wrong in one tap. Worth pinning are a channel left
 * alone keeping each panel's own colour, Vibrant resolving per panel, and
 * mono clearing rather than leaving the old colour behind.
 */

import { describe, test, expect } from 'vitest';
import { colourAll } from './bulk';
import { buildThemeModules } from './model';
import { buildArgbByName, buildPalette, PEBBLE_COLORS_CSV } from './palette';
import { moduleOptionsFixture } from '../testing/fixtures';
import type { ColorMap } from '../types';

const modules = buildThemeModules(moduleOptionsFixture);
const argbByName = buildArgbByName(buildPalette(PEBBLE_COLORS_CSV));
const RED = 0xf0;
const WHITE = 0xff;
const GREEN = 0xcc;

describe('colourAll', () => {
  /** A sweep that skipped a listed panel would leave one box in the old colour. */
  test('writes the picked channel into every listed module', () => {
    const colors: ColorMap = {};

    colourAll(colors, modules, { accent: RED }, argbByName);

    const result = Object.keys(colors).map((id) => colors[Number(id)].accent);

    expect(Object.keys(colors)).toEqual(['1', '2', '3', '6', '25', '49']);
    expect(result).toEqual([RED, RED, RED, RED, RED, RED]);
  });

  /** Setting Value everywhere must not flatten the accents someone already chose per panel. */
  test("keeps each module's own colour on a channel left out of the picks", () => {
    const colors: ColorMap = {
      2: { accent: GREEN, value: null, icon: RED, subtitle: null },
    };

    colourAll(colors, modules, { value: WHITE }, argbByName);

    const result = colors[2];

    expect(result).toEqual({ accent: GREEN, value: WHITE, icon: RED, subtitle: null });
  });

  /** Vibrant means each panel's own colour, so one colour across the board is the bug. */
  test('vibrant gives each module its own signature colour and mono when it has none', () => {
    const colors: ColorMap = {
      3: { accent: RED, value: null, icon: null, subtitle: null },
    };

    colourAll(colors, modules, { accent: 'vibrant' }, argbByName);

    const result = [colors[2].accent, colors[3].accent];

    expect(result).toEqual([GREEN, null]);
  });

  /** Mono has to clear a colour already set, or the "back to plain" sweep does nothing. */
  test('mono clears a colour already set', () => {
    const colors: ColorMap = {
      6: { accent: RED, value: RED, icon: RED, subtitle: RED },
    };

    colourAll(colors, modules, { accent: null, value: null }, argbByName);

    const result = colors[6];

    expect(result).toEqual({ accent: null, value: null, icon: RED, subtitle: RED });
  });

  /** A themeHidden panel draws with its shared row's colour, so a record of its own would fight it. */
  test('leaves the themeHidden modules out', () => {
    const colors: ColorMap = {};

    colourAll(colors, modules, { icon: RED }, argbByName);

    const result = colors[26];

    expect(result).toBeUndefined();
  });
});
