/**
 * Specs for the icon rasterizer's pure steps.
 *
 * whiten decides whether a glyph comes through visible white or invisible black.
 * resourceName/buildMedia/replaceMediaArray own the package.json sync: which
 * Pebble id a file maps to, how a manifest's icons fold into an existing media
 * array, and how that array is spliced back without disturbing the rest of the
 * file. Everything else in the pipeline is sharp/fs I/O, covered by eyeballing
 * the PNGs.
 */

import { describe, test, expect } from 'vitest';
import { whiten, resourceName, buildMedia, replaceMediaArray } from './generate-icons';
import type { IconManifest } from './generate-icons';

describe('whiten', () => {
  /** A black fill left untouched renders an invisible glyph on the watch's dark face. */
  test.each([
    ['#000000'],
    ['#000'],
    ['black'],
  ])('recolors a hard-coded fill="%s" to white', (color) => {
    const result = whiten(`<svg><path fill="${color}" d="M0 0"/></svg>`);

    expect(result).toContain('fill="#ffffff"');
    expect(result).not.toContain(`fill="${color}"`);
  });

  /** Stroke-styled glyphs set their color on stroke, not fill - a missed stroke comes through black. */
  test('recolors a hard-coded stroke to white', () => {
    const result = whiten('<svg><path stroke="#000" d="M0 0"/></svg>');

    expect(result).toContain('stroke="#ffffff"');
  });

  /** Upstream SVGs are inconsistent about casing, so BLACK/uppercase hex must match too. */
  test('recolors black regardless of case', () => {
    const result = whiten('<svg><path fill="BLACK" stroke="#000000" d="M0 0"/></svg>');

    expect(result).toContain('fill="#ffffff"');
    expect(result).not.toMatch(/fill="BLACK"/i);
  });

  /** A glyph that never declares a root fill defaults to black, so white must be injected on the root. */
  test('injects white fill and stroke on a root svg that declares no fill', () => {
    const result = whiten('<svg viewBox="0 0 24 24"><path d="M0 0"/></svg>');

    expect(result).toContain('<svg fill="#ffffff" stroke="#ffffff" viewBox="0 0 24 24"');
  });

  /** Flooding a stroke icon's fill="none" root with white fills the outline solid, destroying the shape. */
  test('leaves a root that already declares a fill untouched', () => {
    const input = '<svg fill="none" stroke="#000" viewBox="0 0 24 24"><path d="M0 0"/></svg>';

    const result = whiten(input);

    expect(result).toContain('<svg fill="none"');
    expect(result).not.toContain('fill="#ffffff"');
  });
});

describe('resourceName', () => {
  /** The manifest key is the file basename. Its Pebble id is that uppercased with dashes as underscores. */
  test.each([
    ['wi-clear', 'ICON_WI_CLEAR'],
    ['wi-night-partly-cloudy', 'ICON_WI_NIGHT_PARTLY_CLOUDY'],
    ['bluetooth-slash', 'ICON_BLUETOOTH_SLASH'],
    ['hilo-up', 'ICON_HILO_UP'],
    ['uv', 'ICON_UV'],
  ])('maps %s -> %s', (basename, expected) => {
    const result = resourceName(basename);

    expect(result).toBe(expected);
  });
});

describe('buildMedia', () => {
  const manifest: IconManifest = {
    'wi-clear': { svg: 'wi/wi-day-sunny', size: [24, 24] },
    'bluetooth': { svg: 'sr/bluetooth-on', size: [14, 14] },
  };

  /** Icons render into the face, so the media file paths must point at its own resources/icons dir. */
  test('emits a bitmap entry per manifest key pointing at the local resources dir', () => {
    const result = buildMedia([], manifest);

    expect(result).toEqual([
      { type: 'bitmap', name: 'ICON_WI_CLEAR', file: 'icons/wi-clear.png' },
      { type: 'bitmap', name: 'ICON_BLUETOOTH', file: 'icons/bluetooth.png' },
    ]);
  });

  /** A re-gen must replace the old icon block wholesale, not stack a second copy beside it. */
  test('replaces existing icon entries instead of appending', () => {
    const media = [
      { type: 'bitmap', name: 'ICON_WI_CLEAR', file: '../../../lib/resources/icons/wi-clear.png' },
      { type: 'bitmap', name: 'ICON_DROPPED', file: 'icons/dropped.png' },
    ];

    const result = buildMedia(media, manifest);

    expect(result.map((entry) => entry.name)).toEqual(['ICON_WI_CLEAR', 'ICON_BLUETOOTH']);
  });

  /** Backgrounds and fonts are not the generator's to touch, so they keep their place around the icon block. */
  test('keeps non-icon entries in order with icons landing where the first icon sat', () => {
    const media = [
      { type: 'bitmap', name: 'IMAGE_BG', file: 'images/bg.png' },
      { type: 'bitmap', name: 'ICON_OLD', file: 'icons/old.png' },
      { type: 'font', name: 'FONT_STM', file: 'fonts/stm.ttf' },
    ];

    const result = buildMedia(media, manifest);

    expect(result.map((entry) => entry.name)).toEqual([
      'IMAGE_BG', 'ICON_WI_CLEAR', 'ICON_BLUETOOTH', 'FONT_STM',
    ]);
  });

  /** A face gaining its first icons should slot them ahead of the fonts, not after them. */
  test('inserts icons before the fonts when the face had none', () => {
    const media = [
      { type: 'bitmap', name: 'IMAGE_BG', file: 'images/bg.png' },
      { type: 'font', name: 'FONT_STM', file: 'fonts/stm.ttf' },
    ];

    const result = buildMedia(media, manifest);

    expect(result.map((entry) => entry.name)).toEqual([
      'IMAGE_BG', 'ICON_WI_CLEAR', 'ICON_BLUETOOTH', 'FONT_STM',
    ]);
  });
});

describe('replaceMediaArray', () => {
  /** Bitmaps ride on one line for a scannable list. Fonts keep their multi-line block so extra fields survive. */
  test('renders bitmaps inline and fonts as blocks', () => {
    const raw = [
      '{',
      '  "resources": {',
      '    "media": [',
      '      { "type": "bitmap", "name": "ICON_OLD", "file": "resources/icons/old.png" }',
      '    ]',
      '  }',
      '}',
    ].join('\n');
    const media = [
      { type: 'bitmap', name: 'ICON_WI_CLEAR', file: 'icons/wi-clear.png' },
      { type: 'font', name: 'FONT_STM', file: 'fonts/stm.ttf', characterRegex: '[0-9]' },
    ];

    const result = replaceMediaArray(raw, media);

    expect(result).toContain('      { "type": "bitmap", "name": "ICON_WI_CLEAR", "file": "icons/wi-clear.png" },');
    expect(result).toContain('        "characterRegex": "[0-9]"');
    expect(JSON.parse(result)).toHaveProperty('resources.media');
  });

  /** The splice must survive an array-valued field inside an entry rather than closing the media array early. */
  test('does not stop at a nested array inside an entry', () => {
    const raw = '{ "media": [ { "type": "font", "name": "F", "file": "f.ttf", "targetPlatforms": ["emery"] } ] }';
    const media = [{ type: 'bitmap', name: 'ICON_A', file: 'icons/a.png' }];

    const result = replaceMediaArray(raw, media);

    expect(JSON.parse(result).media).toEqual(media);
  });

  /** Only the media array is the generator's to rewrite. Text on either side must come through byte-for-byte. */
  test('leaves surrounding text untouched', () => {
    const raw = '{\n  "uuid": "keep-me",\n  "media": [\n    { "type": "bitmap", "name": "ICON_OLD", "file": "resources/icons/old.png" }\n  ],\n  "trailing": true\n}';

    const result = replaceMediaArray(raw, [{ type: 'bitmap', name: 'ICON_A', file: 'icons/a.png' }]);

    expect(result).toContain('"uuid": "keep-me"');
    expect(result).toContain('"trailing": true');
  });
});
