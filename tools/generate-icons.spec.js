/**
 * Specs for the icon rasterizer's color transform.
 *
 * whiten is the one pure step in the pipeline (the rest is sharp/fs I/O). It
 * decides whether a glyph comes through visible white or invisible black, and
 * whether a stroke outline stays open or gets flooded solid, so it is tested
 * directly. The rendering itself is covered by eyeballing the PNGs.
 */

import { describe, test, expect } from 'vitest';
import { whiten } from './generate-icons.js';

describe('whiten', () => {
  /** A black fill left untouched renders an invisible glyph on the watch's dark face. */
  test.each([
    ['#000000'],
    ['#000'],
    ['black']
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
