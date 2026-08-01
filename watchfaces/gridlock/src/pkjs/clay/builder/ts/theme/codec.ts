/**
 * The APPEARANCE_CUSTOM_COLORS wire string in all three of its lives: the
 * current sparse ~3 format it writes, plus the ~ positional and the oldest
 * 5 char per module formats it still reads and migrates. This whole string
 * is the interchange format the import/export textarea and the one
 * APPEARANCE_CUSTOM_COLORS key carry, the watch splits it into two persist
 * keys internally.
 *
 * What this writes has to match the C decoder in settings_schema.c exactly, so
 * keep the ~3 encoder in step with it or the watch reads the string wrong.
 *
 * What this reads is deliberately wider than C. The watch only understands the
 * two tagged formats and treats an untagged string as carrying nothing, so a
 * saved 5 char string paints plain on the wrist. Reading it here anyway is the
 * way back: the config page shows the colours the string holds, and saving
 * re-emits them as ~3 for the watch. So the two decoders disagreeing on an
 * untagged string is the migration, not a drift to go and fix.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { ColorMap, FlagMap } from '../types';

/** The colour and flag maps a wire string parses into. */
export interface AppearanceMaps {
  colors: ColorMap;
  headerless: FlagMap;
  borderless: FlagMap;
}

// one url-safe base64 char per 64-colour index. "." leaves the channel mono
// keep in step with the C decoder in settings_schema.c
export const COLOR_ALPHABET =
  'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_';

// a tagged string carries this so it cannot be mistaken for the old
// per-module format. must not be a base64 char or "." so it can never read
// as a record. mirrors the C marker
export const FORMAT_MARKER = '~';

// the four placement sizes in the bit order the packed flags use. mirrors
// ModuleSize in the C catalog so JS and firmware agree on which bit is which size
export const SIZE_ORDER = ['1x2', '2x2', '1x4', '2x4'];

/** A colour byte as its wire char, "." for a channel left mono. */
export function channelToken(byte: number | null): string {
  return byte == null ? '.' : COLOR_ALPHABET.charAt(byte & 63);
}

/**
 * One 6 bit slice of the flags byte as a base64 char, "." for zero so a
 * blank record reads "......" instead of a run of A's.
 */
export function flagToken(sixBits: number): string {
  return sixBits === 0 ? '.' : COLOR_ALPHABET.charAt(sixBits & 63);
}

/** One flag char back to its 0..63 value, 0 for "." or any junk. */
export function flagValue(ch: string): number {
  const idx = COLOR_ALPHABET.indexOf(ch);
  return idx < 0 ? 0 : idx;
}

/** One char per module id (base64). Ids stay well under 64 so one char is plenty. */
export function idToken(id: number): string {
  return COLOR_ALPHABET.charAt(id & 63);
}

/** One colour char back into its argb byte, or null when it names no colour. */
export function decodeChannel(ch: string): number | null {
  const idx = COLOR_ALPHABET.indexOf(ch);
  return idx < 0 ? null : 192 + idx;
}

/** Reads a per size flag without tripping over a missing module entry. */
export function flagOn(map: FlagMap, moduleValue: number, size: string): boolean {
  return Boolean(map[moduleValue] && map[moduleValue][size]);
}

/** Writes one per size flag, creating the module entry when needed. */
export function setFlag(map: FlagMap, moduleValue: number, size: string, on: boolean): void {
  if (!map[moduleValue]) {
    map[moduleValue] = {};
  }
  map[moduleValue][size] = on;
}

/**
 * Packs a module's per size header/border flags into one byte: bit s is
 * headerless at size s, bit 4+s is borderless at size s (s is the SIZE_ORDER
 * index, mirroring C ModuleSize).
 */
export function packFlags(headerless: FlagMap, borderless: FlagMap, id: number): number {
  let byte = 0;
  for (let s = 0; s < SIZE_ORDER.length; s++) {
    const size = SIZE_ORDER[s];
    if (flagOn(headerless, id, size)) {
      byte |= 1 << s;
    }
    if (flagOn(borderless, id, size)) {
      byte |= 1 << (4 + s);
    }
  }

  return byte;
}

/**
 * Spreads a packed byte back across the per size maps. Used for both tagged
 * records and, on migration, the old single flag applied to every size.
 */
export function unpackFlags(headerless: FlagMap, borderless: FlagMap, id: number, byte: number): void {
  for (let s = 0; s < SIZE_ORDER.length; s++) {
    const size = SIZE_ORDER[s];
    if (byte & (1 << s)) {
      setFlag(headerless, id, size, true);
    }
    if (byte & (1 << (4 + s))) {
      setFlag(borderless, id, size, true);
    }
  }
}

/** Reads the four colour chars of a record starting at offset into the colours map. */
export function readColours(colors: ColorMap, text: string, id: number, offset: number): void {
  const accent = decodeChannel(text.charAt(offset));
  const value = decodeChannel(text.charAt(offset + 1));
  const icon = decodeChannel(text.charAt(offset + 2));
  const subtitle = decodeChannel(text.charAt(offset + 3));

  if (accent != null || value != null || icon != null || subtitle != null) {
    colors[id] = { accent: accent, value: value, icon: icon, subtitle: subtitle };
  }
}

/**
 * Builds the sparse ~3 wire string: a colour section then "|" then a flag
 * section. Colour records (5 chars: id + four channels) are keyed by the
 * group's primary id, so a grouped member emits nothing and inherits at draw
 * time. Flag records (3 chars: id + two packed flag chars) are keyed by each
 * module's own id, so every panel toggles independently. An empty table
 * serialises to "0".
 */
export function serializeAppearance(colors: ColorMap, headerless: FlagMap, borderless: FlagMap): string {
  const colorIds = Object.keys(colors).sort(function (a, b) { return parseInt(a, 10) - parseInt(b, 10); });
  const colorRecs = [];
  for (let i = 0; i < colorIds.length; i++) {
    const id = parseInt(colorIds[i], 10);
    const c = colors[id];
    if (!c || (c.accent == null && c.value == null && c.icon == null && c.subtitle == null)) {
      continue;
    }
    colorRecs.push(idToken(id) + channelToken(c.accent) + channelToken(c.value) +
                   channelToken(c.icon) + channelToken(c.subtitle));
  }

  const flagSeen: Record<string, boolean> = {};
  let key;
  for (key in headerless) { flagSeen[key] = true; }
  for (key in borderless) { flagSeen[key] = true; }
  const flagIds = Object.keys(flagSeen).sort(function (a, b) { return parseInt(a, 10) - parseInt(b, 10); });
  const flagRecs = [];
  for (let j = 0; j < flagIds.length; j++) {
    const fid = parseInt(flagIds[j], 10);
    const byte = packFlags(headerless, borderless, fid);
    if (byte === 0) {
      continue;
    }
    flagRecs.push(idToken(fid) + flagToken(byte & 63) + flagToken((byte >> 6) & 63));
  }

  if (colorRecs.length === 0 && flagRecs.length === 0) {
    return '0';
  }

  return FORMAT_MARKER + '3' + colorRecs.join('') + '|' + flagRecs.join('');
}

/**
 * Reads a wire string back into fresh colour and flag maps. A ~3 string is
 * the current sparse format, a bare ~ one is the positional format, and an
 * untagged one is the oldest per module format. The older two migrate in
 * place, the single legacy flag applying to every size.
 */
export function parseAppearance(text: string): AppearanceMaps {
  const colors: ColorMap = {};
  const headerless: FlagMap = {};
  const borderless: FlagMap = {};
  const result: AppearanceMaps = { colors: colors, headerless: headerless, borderless: borderless };

  if (!text || text === '0') {
    return result;
  }

  // ~3 sparse: "~3" + 5-char colour records + "|" + 3-char flag records
  if (text.charAt(0) === FORMAT_MARKER && text.charAt(1) === '3') {
    const sparse = text.substring(2);
    const bar = sparse.indexOf('|');
    const colorPart = bar < 0 ? sparse : sparse.substring(0, bar);
    const flagPart = bar < 0 ? '' : sparse.substring(bar + 1);
    let o;
    for (o = 0; o + 5 <= colorPart.length; o += 5) {
      const cid = COLOR_ALPHABET.indexOf(colorPart.charAt(o));
      if (cid > 0) {
        readColours(colors, colorPart, cid, o + 1);
      }
    }
    for (o = 0; o + 3 <= flagPart.length; o += 3) {
      const fid = COLOR_ALPHABET.indexOf(flagPart.charAt(o));
      if (fid > 0) {
        const fbyte = flagValue(flagPart.charAt(o + 1)) | (flagValue(flagPart.charAt(o + 2)) << 6);
        unpackFlags(headerless, borderless, fid, fbyte);
      }
    }
    return result;
  }

  // ~ (positional): a fixed 6-char record per id. migrated on the next save to ~3
  if (text.charAt(0) === FORMAT_MARKER) {
    const body = text.substring(1);
    for (let offset = 0; offset + 6 <= body.length; offset += 6) {
      const id = offset / 6;
      if (id === 0) {
        continue; // module 0 is Empty
      }
      readColours(colors, body, id, offset);
      const byte = flagValue(body.charAt(offset + 4)) | (flagValue(body.charAt(offset + 5)) << 6);
      unpackFlags(headerless, borderless, id, byte);
    }
    return result;
  }

  // oldest: fixed 5-char slots with one readable flags char per module. migrate it
  for (let oldOffset = 0; oldOffset + 5 <= text.length; oldOffset += 5) {
    const oldId = oldOffset / 5;
    if (oldId === 0) {
      continue;
    }
    readColours(colors, text, oldId, oldOffset);
    const flag = text.charAt(oldOffset + 4);
    const hideHeader = flag === 'H' || flag === 'X';
    const hideBorder = flag === 'B' || flag === 'X';
    for (let s = 0; s < SIZE_ORDER.length; s++) {
      if (hideHeader) {
        setFlag(headerless, oldId, SIZE_ORDER[s], true);
      }
      if (hideBorder) {
        setFlag(borderless, oldId, SIZE_ORDER[s], true);
      }
    }
  }
  return result;
}
