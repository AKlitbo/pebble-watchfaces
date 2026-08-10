// @vitest-environment jsdom
/**
 * Specs for the layout builder's module visuals.
 *
 * The component specs run with no thumbnails, so the screenshot lookup and
 * the block filling rules only get exercised here. thumbFor's themeRows
 * redirection is the mapping that lets one module's shot live under another
 * module's label, exactly the kind of quiet indirection a config.ts edit
 * could break.
 */

import { describe, test, expect } from 'vitest';
import { buildModuleList, modInfo, thumbFor, fillBlockVisual } from './visuals';
import { moduleOptionsFixture } from '../../../../../../../core/pkjs/clay/builder/ts/testing/harness';

const modules = buildModuleList(moduleOptionsFixture);
const thumbs = {
  'Time + Date': { '2x2': 'data:time-date-2x2' },
  'Digital Clock': { '1x4': 'data:digital-clock-1x4' },
  'Battery': { '1x2': 'data:battery-1x2' },
};

describe('buildModuleList', () => {
  /** Empty is not placeable, listing it would put a dead chip in the palette. */
  test('drops module 0 and fills the icon and colour fallbacks', () => {
    const result = buildModuleList([{ label: 'X', value: 9 }]);

    expect(modules.some((module) => module.value === 0)).toBe(false);
    expect(result).toEqual([{ value: 9, label: 'X', icon: '·', color: '#999', sizes: [], themeRows: undefined }]);
  });
});

describe('modInfo', () => {
  /** An unknown id from an imported layout must render a placeholder block, not crash the grid. */
  test('falls back to the Unknown stand in for an id it does not know', () => {
    const result = modInfo(modules, 999);

    expect(result.label).toBe('Unknown');
    expect(result.icon).toBe('?');
  });
});

describe('thumbFor', () => {
  /** The 2x2 digital clock's shot is filed under "Time + Date", losing the redirect blanks that block. */
  test('follows a themeRows entry to the label that owns the size\'s shot', () => {
    const result = thumbFor(thumbs, modules, 1, '2x2');

    expect(result).toBe('data:time-date-2x2');
  });

  /** A size without a themeRows redirect has to keep using the module's own label. */
  test('uses the module\'s own label when the themeRows entry matches the size', () => {
    const result = thumbFor(thumbs, modules, 1, '1x4');

    expect(result).toBe('data:digital-clock-1x4');
  });

  /** A missing shot must come back null so the block falls back to its emoji instead of a broken img. */
  test('returns null when the label has no shot at that size', () => {
    expect(thumbFor(thumbs, modules, 2, '1x2')).toBe('data:battery-1x2');
    expect(thumbFor(thumbs, modules, 2, '2x2')).toBeNull();
    expect(thumbFor({}, modules, 2, '1x2')).toBeNull();
  });
});

describe('fillBlockVisual', () => {
  /** A 1x4 shot left object-fit contain letterboxes inside the square-row cell. */
  test('stretches a full width shot and keeps the rest contained', () => {
    const wide = document.createElement('div');
    const half = document.createElement('div');
    // a real block always carries these, and the thumb path below never reads them
    const display = { icon: '🔋', label: 'Battery', color: '#4caf50' };

    fillBlockVisual(wide, 'data:shot', display, 4);
    fillBlockVisual(half, 'data:shot', display, 2);

    expect(wide.querySelector<HTMLElement>('img').style.objectFit).toBe('fill');
    expect(half.querySelector<HTMLElement>('img').style.objectFit).toBe('');
  });

  /** Without a shot the block must show the emoji and name or the grid reads as empty. */
  test('falls back to the icon and name when there is no shot', () => {
    const block = document.createElement('div');

    fillBlockVisual(block, null, { icon: '🔋', label: 'Battery', color: '#4caf50' }, 2);

    expect(block.querySelector<HTMLElement>('.lb-icon').textContent).toBe('🔋');
    expect(block.querySelector<HTMLElement>('.lb-name').textContent).toBe('Battery');
    expect(block.querySelector<HTMLElement>('img')).toBeNull();
  });
});
