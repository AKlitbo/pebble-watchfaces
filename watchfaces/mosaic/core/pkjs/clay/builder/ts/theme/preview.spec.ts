// @vitest-environment jsdom
/**
 * Specs for the live colour preview.
 *
 * The luma flip and the mono fallbacks are the firmware's own rules mirrored
 * in the config page, so a drift here shows the user a preview the watch
 * will not honour.
 */

import { describe, test, expect } from 'vitest';
import { contrastText, channelCss, paintSwatch, buildExampleBox } from './preview';

describe('contrastText', () => {
  /** A wrong flip leaves the header title unreadable on its own accent colour. */
  test('goes black on light accents and white on dark ones', () => {
    expect(contrastText(255)).toBe('#000000'); // White
    expect(contrastText(234)).toBe('#000000'); // Light Gray
    expect(contrastText(213)).toBe('#FFFFFF'); // Dark Gray
    expect(contrastText(192)).toBe('#FFFFFF'); // Black
  });

  /** Mono accent renders white on the watch, so its header title has to go black. */
  test('treats mono as a light accent', () => {
    expect(contrastText(null)).toBe('#000000');
  });
});

describe('channelCss', () => {
  /** An unset channel must preview the way the Custom theme's mono default renders. */
  test('falls back to white, with the caption light gray', () => {
    expect(channelCss('value', null)).toBe('#FFFFFF');
    expect(channelCss('subtitle', null)).toBe('#AAAAAA');
    expect(channelCss('value', 240)).toBe('#FF0000');
  });
});

describe('paintSwatch', () => {
  /** A mono swatch painted a solid colour would read as a chosen colour the watch never got. */
  test('paints mono as the stripe pattern and a byte as its colour', () => {
    const monoSwatch = document.createElement('div');
    const colourSwatch = document.createElement('div');

    paintSwatch(monoSwatch, null);
    paintSwatch(colourSwatch, 240);

    expect(monoSwatch.style.background).toContain('repeating-linear-gradient');
    expect(colourSwatch.style.background).toBe('rgb(255, 0, 0)');
  });
});

describe('buildExampleBox', () => {
  /** A headerless module previewing a header would show an accent the watch never draws. */
  test('drops the header strip when asked and keeps it otherwise', () => {
    const module = { label: 'Battery' };
    const staged = { accent: null, value: null, icon: null, subtitle: null };

    const withHeader = buildExampleBox(module, staged, false, false);
    const headerless = buildExampleBox(module, staged, true, false);

    expect(withHeader.box.querySelector<HTMLElement>('.tb-ex-hdr')).not.toBeNull();
    expect(headerless.box.querySelector<HTMLElement>('.tb-ex-hdr')).toBeNull();
  });

  /** A borderless module previewing an accent outline would oversell what the watch draws. */
  test('paints the border transparent when the border is hidden', () => {
    const module = { label: 'Battery' };
    const staged = { accent: 240, value: null, icon: null, subtitle: null };

    const bordered = buildExampleBox(module, staged, false, false);
    const borderless = buildExampleBox(module, staged, false, true);

    expect(bordered.box.style.border).toContain('rgb(255, 0, 0)');
    expect(borderless.box.style.border).toContain('transparent');
  });

  /** paint() repainting the wrong region would preview an edit on the wrong channel. */
  test('paint updates just the channel it is given', () => {
    const module = { label: 'Battery' };
    const staged = { accent: null, value: null, icon: null, subtitle: null };
    const example = buildExampleBox(module, staged, false, false);

    example.paint('value', 240);

    expect(example.box.querySelector<HTMLElement>('.tb-ex-val').style.color).toBe('rgb(255, 0, 0)');
    expect(example.box.querySelector<HTMLElement>('.tb-ex-icon').style.color).toBe('rgb(255, 255, 255)');
  });
});
