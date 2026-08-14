// @vitest-environment jsdom
/**
 * Specs for turning a readout into something to look at.
 *
 * A filled panel in the builder is meant to be a picture of what the watch will
 * draw, so the cases worth pinning are the ones where the picture is missing:
 * a readout the thumbnail pass never shot falls back to its emoji and tint, and
 * the fallback is what stops an unphotographed readout rendering as a broken
 * image or an empty box.
 */

import { describe, test, expect, beforeEach } from 'vitest';
import { buildReadoutList, sizeFor, thumbFor, fillVisual, readoutById } from './visuals';
import type { Readout } from './visuals';
import { ID_HEART, ID_EMPTY, ID_SENSORS } from './geometry';

const THUMBS = {
  'Heart Rate': { slot: 'data:heart-slot' },
  'Sensors': { tall: 'data:sensors-tall' },
  'Steps': { slot: '' },
};

const HEART: Readout = { value: ID_HEART, label: 'Heart Rate', icon: '♥', color: '#f00' };
const SENSORS: Readout = { value: ID_SENSORS, label: 'Sensors', icon: '⚙', color: '#0f0' };
const UNSHOT: Readout = { value: 5, label: 'Stocks', icon: '$', color: '#00f' };

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('buildReadoutList', () => {
  /**
   * Clay hands option values back as strings.
   *
   * Leaving them as text makes every later === against a catalog id fail, so the palette would
   * drag readouts that never match the slot they land in.
   */
  test('reads a string option value as a number', () => {
    const result = buildReadoutList([{ value: '1', label: 'Steps' }]);

    expect(result[0].value).toBe(1);
  });

  /** Clearing a panel is what dragging one off does, so an Empty row in the palette would be a second way to do it that looks like a readout. */
  test('drops the Empty row from the palette', () => {
    const result = buildReadoutList([
      { value: 0, label: 'Heart Rate' },
      { value: ID_EMPTY, label: 'Empty' },
      { value: 1, label: 'Steps' },
    ]);

    expect(result.map((readout) => readout.label)).toEqual(['Heart Rate', 'Steps']);
  });

  /** A row with no icon or colour still has to draw something, or its palette tile renders as a blank square. */
  test('fills in an icon and tint for a row that carries neither', () => {
    const result = buildReadoutList([{ value: 1, label: 'Steps' }]);

    expect(result[0].icon).toBe('·');
    expect(result[0].color).toBe('#999');
  });

  /** Clay can hand over nothing at all before the config settles, and mapping that would throw before the palette draws. */
  test('reads a missing option list as an empty palette', () => {
    const result = buildReadoutList(null);

    expect(result).toEqual([]);
  });
});

describe('sizeFor', () => {
  /** The tall block is shot at its own size, so filing it under the panel size fetches a crop of the wrong shape. */
  test('files the tall block apart from the ordinary readouts', () => {
    const result = [sizeFor(ID_SENSORS), sizeFor(ID_HEART)];

    expect(result).toEqual(['tall', 'slot']);
  });
});

describe('thumbFor', () => {
  /** Reading the wrong size hands the panel another shot, so the tall block would render a panel-sized crop. */
  test('returns the shot filed under the label at its own size', () => {
    const result = [thumbFor(THUMBS, HEART), thumbFor(THUMBS, SENSORS)];

    expect(result).toEqual(['data:heart-slot', 'data:sensors-tall']);
  });

  /** A readout the thumbnail pass skipped has no entry, and indexing into it would throw before the panel draws. */
  test('returns null for a readout with no shots at all', () => {
    const result = thumbFor(THUMBS, UNSHOT);

    expect(result).toBeNull();
  });

  /** An empty panel has no readout to photograph, and asking anyway would throw on the label lookup. */
  test('returns null for no readout', () => {
    const result = thumbFor(THUMBS, null);

    expect(result).toBeNull();
  });

  /** An empty data url is falsy but not null, and setting img.src to it reloads the config page into the panel. */
  test('returns null rather than the empty string for a blank entry', () => {
    const result = thumbFor(THUMBS, { value: 1, label: 'Steps', icon: '·', color: '#999' });

    expect(result).toBeNull();
  });
});

describe('readoutById', () => {
  /** A drag carries a catalog id, so a failed lookup is a ghost with nothing drawn in it. */
  test('finds a readout by its catalog id', () => {
    const result = readoutById([HEART, SENSORS], ID_SENSORS);

    expect(result).toBe(SENSORS);
  });

  /** An id the palette does not carry must read as nothing rather than as the first readout in the list. */
  test('returns null for an id that is not in the list', () => {
    const result = readoutById([HEART, SENSORS], 99);

    expect(result).toBeNull();
  });
});

describe('fillVisual', () => {
  /** A panel is repainted every time the slots change, and leaving the old contents stacks a second image on top of the first. */
  test('clears whatever the box held before', () => {
    const box = document.createElement('div');
    fillVisual(box, HEART, THUMBS, false);

    fillVisual(box, SENSORS, THUMBS, false);

    expect(box.querySelectorAll('img')).toHaveLength(1);
    expect(box.querySelector('img').src).toBe('data:sensors-tall');
  });

  /** An empty panel has to say so, or a cleared slot looks like one the builder failed to draw. */
  test('writes an empty marker for no readout', () => {
    const box = document.createElement('div');

    fillVisual(box, null, THUMBS, false);

    expect(box.querySelector('.sb-slot-empty').textContent).toBe('Empty');
  });

  /** The palette and the panels size their images differently, so the wrong class draws a tile at the other one's scale. */
  test('classes the image by whether the box is a palette tile', () => {
    const tile = document.createElement('div');
    tile.className = 'sb-pal';
    const slot = document.createElement('div');

    fillVisual(tile, HEART, THUMBS, false);
    fillVisual(slot, HEART, THUMBS, false);

    expect(tile.querySelector('img').className).toBe('sb-pal-img');
    expect(slot.querySelector('img').className).toBe('sb-slot-img');
  });

  /** Without the fallback an unphotographed readout draws nothing at all, which reads as a builder that failed rather than a readout with no shot. */
  test('falls back to the tint and emoji for a readout with no shot', () => {
    const box = document.createElement('div');

    fillVisual(box, UNSHOT, THUMBS, false);

    expect(box.querySelector('img')).toBeNull();
    expect(box.classList.contains('sb-pal-fallback')).toBe(true);
    expect(box.querySelector('.sb-pal-icon').textContent).toBe('$');
  });

  /** The palette names its readouts and the panels do not, so ignoring the flag either crowds a panel or leaves the palette unreadable. */
  test('adds the label to the fallback only when asked', () => {
    const named = document.createElement('div');
    const bare = document.createElement('div');

    fillVisual(named, UNSHOT, THUMBS, true);
    fillVisual(bare, UNSHOT, THUMBS, false);

    expect(named.textContent).toContain('Stocks');
    expect(bare.textContent).not.toContain('Stocks');
  });

  /** A box that fell back once and is later given a shot would keep the tint behind its image. */
  test('drops the fallback styling when a later readout has a shot', () => {
    const box = document.createElement('div');
    fillVisual(box, UNSHOT, THUMBS, false);

    fillVisual(box, HEART, THUMBS, false);

    expect(box.classList.contains('sb-pal-fallback')).toBe(false);
    expect(box.style.background).toBe('');
  });
});
