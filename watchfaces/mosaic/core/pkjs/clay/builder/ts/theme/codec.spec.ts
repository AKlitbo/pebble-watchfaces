/**
 * Specs for the APPEARANCE_CUSTOM_COLORS wire codec.
 *
 * This string is the contract with the C decoder in settings_schema.c, so
 * the goldens here pin the exact chars either side reads. The flag bit
 * positions mirror ModuleSize in the C catalog, if those drift the watch
 * hides the wrong panel's header.
 */

import { describe, test, expect } from 'vitest';
import {
  SIZE_ORDER,
  channelToken,
  flagToken,
  flagValue,
  decodeChannel,
  flagOn,
  setFlag,
  packFlags,
  unpackFlags,
  serializeAppearance,
  parseAppearance,
} from './codec';

describe('wire chars', () => {
  /** A drifted channel char would paint a different colour than the one the user picked. */
  test('channelToken maps a byte to its base64 char and mono to a dot', () => {
    expect(channelToken(192)).toBe('A');
    expect(channelToken(255)).toBe('_');
    expect(channelToken(null)).toBe('.');
  });

  /** decodeChannel is channelToken's inverse, a mismatch corrupts every colour on load. */
  test('decodeChannel maps chars back and junk to null', () => {
    expect(decodeChannel('A')).toBe(192);
    expect(decodeChannel('_')).toBe(255);
    expect(decodeChannel('.')).toBeNull();
    expect(decodeChannel('!')).toBeNull();
  });

  /** A zero flag slice written as "A" would read as a set flag on the way back in. */
  test('flagToken writes zero as a dot and flagValue reads junk as zero', () => {
    expect(flagToken(0)).toBe('.');
    expect(flagToken(15)).toBe('P');
    expect(flagValue('.')).toBe(0);
    expect(flagValue('P')).toBe(15);
    expect(flagValue('!')).toBe(0);
  });
});

describe('flag packing', () => {
  /** A shifted bit would hide the header at the wrong size on the watch. */
  test('packs headerless into bits 0..3 and borderless into bits 4..7 by SIZE_ORDER', () => {
    const headerless = {};
    const borderless = {};
    setFlag(headerless, 5, '1x2', true);
    setFlag(borderless, 5, '2x4', true);

    const result = packFlags(headerless, borderless, 5);

    expect(SIZE_ORDER).toEqual(['1x2', '2x2', '1x4', '2x4']);
    expect(result).toBe((1 << 0) | (1 << 7));
  });

  /** Pack and unpack disagreeing would flip toggles every time settings reopen. */
  test('unpackFlags restores exactly what packFlags wrote', () => {
    const headerless = {};
    const borderless = {};

    unpackFlags(headerless, borderless, 7, (1 << 1) | (1 << 6));

    expect(flagOn(headerless, 7, '2x2')).toBe(true);
    expect(flagOn(headerless, 7, '1x2')).toBe(false);
    expect(flagOn(borderless, 7, '1x4')).toBe(true);
    expect(packFlags(headerless, borderless, 7)).toBe((1 << 1) | (1 << 6));
  });
});

describe('serializeAppearance', () => {
  /** An empty table must stay the "0" sentinel the watch treats as nothing set. */
  test('writes "0" when nothing is set', () => {
    const result = serializeAppearance({}, {}, {});

    expect(result).toBe('0');
  });

  /** An all-null colour entry writing a record would bloat the string with no-ops. */
  test('skips colour entries with every channel mono and flag entries packing to zero', () => {
    const colors = { 2: { accent: null, value: null, icon: null, subtitle: null } };
    const headerless = { 3: { '1x2': false } };

    const result = serializeAppearance(colors, headerless, {});

    expect(result).toBe('0');
  });

  /** The exact record layout is what settings_schema.c walks, char by char. */
  test('writes the ~3 golden for one coloured and one flagged module', () => {
    const colors = { 1: { accent: 192, value: 193, icon: null, subtitle: 194 } };
    const headerless = {};
    SIZE_ORDER.forEach((size) => setFlag(headerless, 1, size, true));

    const result = serializeAppearance(colors, headerless, {});

    expect(result).toBe('~3BAB.C|BP.');
  });

  /** Unsorted records would make the same settings save as different strings. */
  test('writes records sorted by module id', () => {
    const colors = {
      9: { accent: 192, value: null, icon: null, subtitle: null },
      2: { accent: 193, value: null, icon: null, subtitle: null },
    };

    const result = serializeAppearance(colors, {}, {});

    expect(result).toBe('~3CB...JA...|');
  });
});

describe('parseAppearance', () => {
  /** A round trip that mutates would corrupt the saved appearance on every settings open. */
  test('round trips a ~3 string unchanged', () => {
    const text = '~3BAB.C|BP.';

    const parsed = parseAppearance(text);
    const result = serializeAppearance(parsed.colors, parsed.headerless, parsed.borderless);

    expect(result).toBe(text);
  });

  /** A user upgrading from the positional format would lose their colours if this breaks. */
  test('migrates a ~ positional record', () => {
    const parsed = parseAppearance('~......AB.CP.');

    expect(parsed.colors[1]).toEqual({ accent: 192, value: 193, icon: null, subtitle: 194 });
    expect(flagOn(parsed.headerless, 1, '2x4')).toBe(true);
    expect(flagOn(parsed.borderless, 1, '1x2')).toBe(false);
  });

  /** A user upgrading from the oldest release would lose their colours if this breaks. */
  test('migrates a legacy 5 char record, its one flag spreading to every size', () => {
    const parsed = parseAppearance('.....AB.CX');

    expect(parsed.colors[1]).toEqual({ accent: 192, value: 193, icon: null, subtitle: 194 });
    SIZE_ORDER.forEach((size) => {
      expect(flagOn(parsed.headerless, 1, size)).toBe(true);
      expect(flagOn(parsed.borderless, 1, size)).toBe(true);
    });
  });

  /** Module 0 is Empty, a record for it must never come back as real settings. */
  test('skips records for module 0 in every format', () => {
    const sparse = parseAppearance('~3AAAAA|A_.');
    const legacy = parseAppearance('ABCDH');

    expect(Object.keys(sparse.colors)).toEqual([]);
    expect(Object.keys(sparse.headerless)).toEqual([]);
    expect(Object.keys(legacy.colors)).toEqual([]);
  });

  /** Garbage from a bad paste must fall back to defaults instead of half applied colours. */
  test('returns empty maps for null, "0" and short junk', () => {
    // named rather than spread over Object.values, which quietly depended on key order
    const roundTrip = (text: string): string => {
      const { colors, headerless, borderless } = parseAppearance(text);
      return serializeAppearance(colors, headerless, borderless);
    };

    expect(roundTrip(null)).toBe('0');
    expect(roundTrip('0')).toBe('0');
    expect(roundTrip('abc')).toBe('0');
  });
});
