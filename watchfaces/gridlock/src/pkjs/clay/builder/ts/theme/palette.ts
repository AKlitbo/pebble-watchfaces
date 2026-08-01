/**
 * The Pebble 64 colour gamut and the little colour math around it. The CSV
 * here is the official SDK list and the one palette source in the repo, the
 * vibrant generator reads its names from this module too.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { PaletteEntry } from '../types';

// "HEX Name" pairs. every channel is 00/55/AA/FF so two hex digits map
// straight onto a GColor8 argb byte
export const PEBBLE_COLORS_CSV =
  '000000 Black,000055 Oxford Blue,0000AA Duke Blue,0000FF Blue,' +
  '005500 Dark Green,005555 Midnight Green,0055AA Cobalt Blue,0055FF Blue Moon,' +
  '00AA00 Islamic Green,00AA55 Jaeger Green,00AAAA Tiffany Blue,00AAFF Vivid Cerulean,' +
  '00FF00 Green,00FF55 Malachite,00FFAA Medium Spring Green,00FFFF Cyan,' +
  '550000 Bulgarian Rose,550055 Imperial Purple,5500AA Indigo,5500FF Electric Ultramarine,' +
  '555500 Army Green,555555 Dark Gray,5555AA Liberty,5555FF Very Light Blue,' +
  '55AA00 Kelly Green,55AA55 May Green,55AAAA Cadet Blue,55AAFF Picton Blue,' +
  '55FF00 Bright Green,55FF55 Screamin Green,55FFAA Medium Aquamarine,55FFFF Electric Blue,' +
  'AA0000 Dark Candy Apple Red,AA0055 Jazzberry Jam,AA00AA Purple,AA00FF Vivid Violet,' +
  'AA5500 Windsor Tan,AA5555 Rose Vale,AA55AA Purpureus,AA55FF Lavender Indigo,' +
  'AAAA00 Limerick,AAAA55 Brass,AAAAAA Light Gray,AAAAFF Baby Blue Eyes,' +
  'AAFF00 Spring Bud,AAFF55 Inchworm,AAFFAA Mint Green,AAFFFF Celeste,' +
  'FF0000 Red,FF0055 Folly,FF00AA Fashion Magenta,FF00FF Magenta,' +
  'FF5500 Orange,FF5555 Sunset Orange,FF55AA Brilliant Rose,FF55FF Shocking Pink,' +
  'FFAA00 Chrome Yellow,FFAA55 Rajah,FFAAAA Melon,FFAAFF Rich Brilliant Lavender,' +
  'FFFF00 Yellow,FFFF55 Icterine,FFFFAA Pastel Yellow,FFFFFF White';

/**
 * The palette as { argb, css, name } entries, in the CSV's order so an
 * entry's index is its 64 colour index.
 */
export function buildPalette(csv: string): PaletteEntry[] {
  const entries = csv.split(',');
  const palette: PaletteEntry[] = [];
  for (let i = 0; i < entries.length; i++) {
    const entry = entries[i];
    const hex = entry.substr(0, 6);
    const red = parseInt(hex.substr(0, 2), 16);
    const green = parseInt(hex.substr(2, 2), 16);
    const blue = parseInt(hex.substr(4, 2), 16);
    const argb =
      192 +
      Math.round(red / 85) * 16 +
      Math.round(green / 85) * 4 +
      Math.round(blue / 85);
    palette.push({ argb: argb, css: '#' + hex, name: entry.substr(7) });
  }

  return palette;
}

/**
 * Name to argb byte lookup, so a colour named elsewhere (the vibrant table)
 * finds its byte.
 */
export function buildArgbByName(palette: PaletteEntry[]): Record<string, number> {
  const argbByName: Record<string, number> = {};
  for (let i = 0; i < palette.length; i++) {
    argbByName[palette[i].name] = palette[i].argb;
  }

  return argbByName;
}

/** A number (0 to 255) as two uppercase hex digits. */
export function toHexByte(number: number): string {
  const text = number.toString(16).toUpperCase();
  return text.length < 2 ? '0' + text : text;
}

/** Three channel values (0 to 255) as a css hex colour. */
export function rgbToCss(red: number, green: number, blue: number): string {
  return '#' + toHexByte(red) + toHexByte(green) + toHexByte(blue);
}

/** An argb byte (a GColor8 style byte) as its css hex colour. */
export function argbToCss(argb: number): string {
  const red = ((argb >> 4) & 3) * 85;
  const green = ((argb >> 2) & 3) * 85;
  const blue = (argb & 3) * 85;

  return rgbToCss(red, green, blue);
}
