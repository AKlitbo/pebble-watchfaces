/**
 * Specs for the layout library.
 *
 * The cases worth pinning are the ones that lose someone's work without saying anything: a store
 * that fails to read leaving four blank grids, an upgrade that does not find the layout already on
 * the watch, and an assignment pointing at a layout that is not there.
 *
 * @vitest-environment jsdom
 */

import { describe, test, expect, beforeEach } from 'vitest';
import { readLibrary, writeLibrary, seedLibrary, buildModesBar, LAYOUT_COUNT, NIGHT_NONE } from './modes';
import { EMPTY_LAYOUT } from './codec';

/** The hidden store the library lives in, the way the config page renders it. */
function mountStore(value = ''): HTMLInputElement {
  const input = document.createElement('input');
  input.type = 'hidden';
  input.className = 'gl-store gl-library';
  input.value = value;
  document.body.appendChild(input);
  return input;
}

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('readLibrary', () => {
  /** With no store at all the page still has to build, so this cannot throw. */
  test('gives a full library when the page has no store', () => {
    const result = readLibrary();

    expect(result.layouts).toHaveLength(LAYOUT_COUNT);
    expect(result.day).toBe(0);
    expect(result.night).toBe(NIGHT_NONE);
  });

  /** A half-written or corrupt value must not strand the user on a blank page. */
  test('repairs junk rather than throwing', () => {
    mountStore('{not json');

    const result = readLibrary();

    expect(result.layouts).toHaveLength(LAYOUT_COUNT);
    expect(result.layouts.every((layout) => layout === EMPTY_LAYOUT)).toBe(true);
  });

  /** A short library from an older shape still has to come back the right length. */
  test('pads a library that is short', () => {
    mountStore(JSON.stringify({ layouts: ['2,0,0,2,2'], day: 0, night: NIGHT_NONE }));

    const result = readLibrary();

    expect(result.layouts).toHaveLength(LAYOUT_COUNT);
    expect(result.layouts[0]).toBe('2,0,0,2,2');
    expect(result.layouts[3]).toBe(EMPTY_LAYOUT);
  });

  /** An assignment past the end would publish undefined as a layout. */
  test('clamps an assignment that names no layout', () => {
    mountStore(JSON.stringify({ layouts: [], day: 99, night: 42 }));

    const result = readLibrary();

    expect(result.day).toBe(0);
    expect(result.night).toBe(NIGHT_NONE);
  });
});

describe('writeLibrary', () => {
  /** Clay only saves what it is told changed, so the write has to fire the event too. */
  test('writes the store and nudges Clay', () => {
    const input = mountStore();
    let changes = 0;
    input.addEventListener('change', () => { changes++; });

    const result = writeLibrary({ layouts: ['a', 'b', 'c', 'd'], day: 1, night: 2 });

    expect(result).toBe(true);
    expect(JSON.parse(input.value).day).toBe(1);
    expect(changes).toBe(1);
  });

  /** And it says so rather than throwing when there is nowhere to write. */
  test('reports failure when the page has no store', () => {
    const result = writeLibrary({ layouts: ['a', 'b', 'c', 'd'], day: 0, night: NIGHT_NONE });

    expect(result).toBe(false);
  });
});

describe('seedLibrary', () => {
  /**
   * The upgrade case, and the one that would be noticed loudest.
   *
   * Someone on 0.14.0 has a layout on their watch and no library at all. If it did not land in
   * layout 1 they would open the settings page and find their design gone.
   */
  test('puts an existing watch layout into layout 1', () => {
    const empty = { layouts: [EMPTY_LAYOUT, EMPTY_LAYOUT, EMPTY_LAYOUT, EMPTY_LAYOUT], day: 0, night: NIGHT_NONE };

    const result = seedLibrary(empty, '2,0,0,2,2;3,0,2,2,1');

    expect(result.layouts[0]).toBe('2,0,0,2,2;3,0,2,2,1');
    expect(result.day).toBe(0);
  });

  /** But it must never overwrite a library the user has already built. */
  test('leaves a library that already has something in it', () => {
    const used = { layouts: ['9,0,0,2,2', EMPTY_LAYOUT, EMPTY_LAYOUT, EMPTY_LAYOUT], day: 0, night: NIGHT_NONE };

    const result = seedLibrary(used, '2,0,0,2,2');

    expect(result.layouts[0]).toBe('9,0,0,2,2');
  });

  /** A fresh install has nothing to seed from, which is not an error. */
  test('does nothing when there is no existing layout', () => {
    const empty = { layouts: [EMPTY_LAYOUT, EMPTY_LAYOUT, EMPTY_LAYOUT, EMPTY_LAYOUT], day: 0, night: NIGHT_NONE };

    const result = seedLibrary(empty, EMPTY_LAYOUT);

    expect(result.layouts[0]).toBe(EMPTY_LAYOUT);
  });
});

describe('buildModesBar', () => {
  /** One tab per layout, or a layout becomes unreachable. */
  test('builds a tab for every layout and two assignment pickers', () => {
    mountStore();
    const host = document.createElement('div');
    const library = { layouts: ['a', 'b', 'c', 'd'], day: 0, night: NIGHT_NONE };

    buildModesBar(host, library, { getCurrent: () => 'a', onSelect: () => {}, onAssign: () => {}, save: () => {} });

    expect(host.querySelectorAll('.lb-ltab')).toHaveLength(LAYOUT_COUNT);
    expect(host.querySelectorAll('select')).toHaveLength(2);
  });

  /**
   * Switching tab has to bank the grid first.
   *
   * Without this every edit is lost the moment you look at another layout, which is the one bug
   * that would make the whole feature untrustworthy.
   */
  test('keeps the current grid before switching away from it', () => {
    mountStore();
    const host = document.createElement('div');
    const library = { layouts: ['a', 'b', 'c', 'd'], day: 0, night: NIGHT_NONE };
    let loaded = '';

    buildModesBar(host, library, {
      getCurrent: () => 'edited',
      onSelect: (layout) => { loaded = layout; },
      onAssign: () => {},
      save: () => {},
    });
    host.querySelectorAll<HTMLElement>('.lb-ltab')[2].click();

    expect(library.layouts[0]).toBe('edited');
    expect(loaded).toBe('c');
  });

  /** The night picker offers None, since turning the feature off has to be reachable. */
  test('the night picker can be set back to none', () => {
    mountStore();
    const host = document.createElement('div');
    const library = { layouts: ['a', 'b', 'c', 'd'], day: 0, night: 2 };
    let assigned = 0;

    buildModesBar(host, library, { getCurrent: () => 'a', onSelect: () => {}, onAssign: () => { assigned++; }, save: () => {} });
    const night = host.querySelectorAll<HTMLSelectElement>('select')[1];
    night.value = String(NIGHT_NONE);
    night.dispatchEvent(new Event('change'));

    expect(library.night).toBe(NIGHT_NONE);
    expect(assigned).toBe(1);
  });
});
