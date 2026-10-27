// @vitest-environment jsdom
/**
 * Specs for the layout builder's palette.
 *
 * The palette filters by group so each list stays a short drag from the face. That makes the
 * filter the thing to pin: which groups a size shows and in what order, opening on a group rather
 * than everything, the status line that says what is left out, and the placed dimming surviving a
 * redraw, since every group switch rebuilds the tiles.
 */

import { describe, test, expect, beforeEach } from 'vitest';
import { createPalette, groupsForSize } from './palette';
import type { ModuleGroup, ModuleInfo } from '../types';

/** A module at the given sizes and group, with the fields the palette never reads left plain. */
function module(value: number, label: string, sizes: string[], group?: ModuleGroup): ModuleInfo {
  return { value: value, label: label, icon: '·', color: '#999', sizes: sizes, group: group };
}

const modules: ModuleInfo[] = [
  module(2, 'Battery', ['1x2', '2x2'], 'Status'),
  module(3, 'Weather', ['1x2', '2x2'], 'Weather'),
  module(4, 'Temperature', ['1x2'], 'Weather'),
  module(6, 'Steps', ['1x2', '2x2'], 'Health'),
  module(17, 'Date', ['1x2'], 'Date'),
  module(25, 'Forecast (Hourly)', ['1x4'], 'Weather'),
  module(70, 'Mystery', ['1x2']),
];

const SIZES = [
  { key: '1x2', wide: false },
  { key: '1x4', wide: true },
  { key: '2x2', wide: false },
  { key: '2x4', wide: true },
];

/** Builds a palette over the modules above, with no thumbnails. */
function mountPalette() {
  const container = document.createElement('div');

  document.body.appendChild(container);

  const palette = createPalette({
    container: container,
    modules: modules,
    sizes: SIZES,
    thumbFor: function () {
      return null;
    },
    onPick: function () {},
  });

  return { container, palette };
}

/** The module ids of the tiles showing, in order. */
function tileValues(container: HTMLElement): number[] {
  return Array.from(container.querySelectorAll<HTMLElement>('.lb-pal-item')).map((tile) => Number(tile.dataset.value));
}

/** Clicks the group tab with this name. */
function pickGroup(container: HTMLElement, label: string): void {
  const tabs = Array.from(container.querySelectorAll<HTMLElement>('.lb-group'));

  tabs.find((tab) => tab.querySelector('.lb-group-nm').textContent === label).click();
}

/** Clicks the size tab with this key. */
function pickSize(container: HTMLElement, key: string): void {
  container.querySelector<HTMLElement>('.lb-tab[data-size="' + key + '"]').click();
}

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('groupsForSize', () => {
  /** The group tabs follow this order, so a shuffle here reorders every tab row on the page. */
  test('lists the groups with panels at the size in the palette order', () => {
    const result = groupsForSize(modules, '1x2').map((section) => section.group.name);

    expect(result).toEqual(['Date', 'Weather', 'Health', 'Status', 'Other']);
  });

  /** A panel nobody gave a group must still be placeable, not vanish from the palette. */
  test('puts a panel with no group under Other at the end', () => {
    const sections = groupsForSize(modules, '1x2');

    const result = sections[sections.length - 1].modules.map((entry) => entry.label);

    expect(result).toEqual(['Mystery']);
  });
});

describe('createPalette', () => {
  /** Opening on every panel makes a long list, and a panel near its end is a long drag from the face. */
  test('opens a size on its first group rather than on every panel', () => {
    const { container } = mountPalette();

    const result = tileValues(container);

    expect(result).toEqual([17]);
  });

  /** Without the status line a short list reads as everything, and the other panels look missing. */
  test('the status line names the group and how many panels it leaves out', () => {
    const { container } = mountPalette();

    pickGroup(container, 'Weather');
    const result = container.querySelector<HTMLElement>('.lb-filter span').textContent;

    expect(result).toBe('Weather · 2 of 6');
    expect(tileValues(container)).toEqual([3, 4]);
  });

  /** Show all is the way back to every panel, so it must show them all and drop the status line. */
  test('Show all lists every panel at the size and drops the status line', () => {
    const { container } = mountPalette();

    container.querySelector<HTMLElement>('.lb-filter-all').click();
    const result = tileValues(container);

    expect(result).toEqual([17, 3, 4, 6, 2, 70]);
    expect(container.querySelector('.lb-filter')).toBeNull();
  });

  /** Losing the pick on a size switch sends you hunting for the same group again every time. */
  test('keeps the group picked on a size when switching away and back', () => {
    const { container } = mountPalette();

    pickGroup(container, 'Health');
    pickSize(container, '2x2');
    pickSize(container, '1x2');
    const result = tileValues(container);

    expect(result).toEqual([6]);
  });

  /** Every group switch rebuilds the tiles, and losing the dimming lets a panel be placed twice. */
  test('a placed panel stays dimmed after switching group away and back', () => {
    const { container, palette } = mountPalette();

    palette.markPlaced([3]);
    pickGroup(container, 'Weather');
    pickGroup(container, 'Date');
    pickGroup(container, 'Weather');
    const result = container.querySelector<HTMLElement>('.lb-pal-item[data-value="3"]').classList.contains('placed');

    expect(result).toBe(true);
  });

  /** A size with one group has nothing to choose between, so a lone group tab is just clutter. */
  test('a size with a single group shows its panels with no group row', () => {
    const { container } = mountPalette();

    pickSize(container, '1x4');
    const result = container.querySelector<HTMLElement>('.lb-groups').hidden;

    expect(result).toBe(true);
    expect(tileValues(container)).toEqual([25]);
  });

  /** A size no panel comes in would open on an empty list, so it gets no tab at all. */
  test('leaves out the tab for a size no panel comes in', () => {
    const { container } = mountPalette();

    const result = Array.from(container.querySelectorAll<HTMLElement>('.lb-tab')).map((tab) => tab.dataset.size);

    expect(result).toEqual(['1x2', '1x4', '2x2']);
  });

  /** A full width shot in a half width tile is too small to read. */
  test('gives a full width size the wide tile', () => {
    const { container } = mountPalette();

    pickSize(container, '1x4');
    const result = container.querySelector<HTMLElement>('.lb-pal-item').className;

    expect(result).toBe('lb-pal-item wide');
  });
});
