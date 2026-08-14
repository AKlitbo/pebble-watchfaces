// @vitest-environment jsdom
/**
 * Specs for reading and writing the four slot values.
 *
 * A Clay component owns one message key and this face needs four, so three of
 * the slots ride hidden inputs found by class. That split is where the silent
 * failures live: a store that is not found reads as the component's own value
 * instead, and a store written without its change event never reaches Clay at
 * all. The ids are catalog numbers off the wire, and one of them is 0.
 */

import { describe, test, expect, beforeEach } from 'vitest';
import { parseId, formatId, storeFor, readSlots, writeStores, sanitize, defaults } from './codec';
import {
  SLOT_LT, SLOT_LB, SLOT_RT, SLOT_RB,
  ID_HEART, ID_STEPS, ID_EMPTY, ID_SENSORS,
} from './geometry';

/** Puts the three hidden stores on the page the way the config template does. */
function mountStores(values: Record<string, string>): HTMLElement {
  document.body.innerHTML = '';

  Object.keys(values).forEach((cls) => {
    const input = document.createElement('input');
    input.className = 'gl-store ' + cls;
    input.value = values[cls];
    document.body.appendChild(input);
  });

  const root = document.createElement('div');
  document.body.appendChild(root);
  return root;
}

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('parseId', () => {
  /**
   * Heart rate is catalog id 0, so a falsy check would swap it for Empty.
   *
   * Every other id is truthy, which is what makes this the one that slips through review and
   * turns a configured heart rate panel blank on the next settings open.
   */
  test('keeps id 0 rather than reading it as nothing', () => {
    const result = parseId('0');

    expect(result).toBe(ID_HEART);
  });

  /** An ordinary id must survive the trip through the input's text. */
  test('reads a plain id', () => {
    const result = parseId('23');

    expect(result).toBe(ID_SENSORS);
  });

  /** A store the page never filled reads as empty text, and NaN would index the catalog off its end. */
  test('falls back to empty for text that is not a number', () => {
    const result = [parseId(''), parseId('banana'), parseId(null), parseId(undefined)];

    expect(result).toEqual([ID_EMPTY, ID_EMPTY, ID_EMPTY, ID_EMPTY]);
  });

  /** A negative id is not in the catalog, so passing it through would draw a panel from nowhere. */
  test('falls back to empty for a negative id', () => {
    const result = parseId('-1');

    expect(result).toBe(ID_EMPTY);
  });
});

describe('formatId', () => {
  /** The store text is what the next read parses, so a format the parser cannot read back loses the panel. */
  test('round trips every id back through parseId', () => {
    const ids = [ID_HEART, ID_STEPS, ID_EMPTY, ID_SENSORS];

    const result = ids.map((id) => parseId(formatId(id)));

    expect(result).toEqual(ids);
  });
});

describe('storeFor', () => {
  /** The upper left is the component's own value, and handing back a store for it would write the panel twice. */
  test('has no store for the upper left slot', () => {
    const root = mountStores({ 'sb-lb': '20' });

    const result = storeFor(root, SLOT_LT);

    expect(result).toBeNull();
  });

  /** Every hiddenStore shares .gl-store, so finding a slot's own class is the only thing keeping them apart. */
  test('finds each of the other slots by its own class', () => {
    const root = mountStores({ 'sb-lb': '1', 'sb-rt': '2', 'sb-rb': '3' });

    const result = [
      storeFor(root, SLOT_LB).value,
      storeFor(root, SLOT_RT).value,
      storeFor(root, SLOT_RB).value,
    ];

    expect(result).toEqual(['1', '2', '3']);
  });

  /** A page built without a store must read as missing rather than throw before the builder draws. */
  test('returns null for a store that is not on the page', () => {
    const root = mountStores({});

    const result = storeFor(root, SLOT_RB);

    expect(result).toBeNull();
  });
});

describe('readSlots', () => {
  /** The component's own value is the upper left and the stores are the rest, and crossing them mirrors two panels. */
  test('takes the upper left from the own value and the rest from the stores', () => {
    const root = mountStores({ 'sb-lb': '20', 'sb-rt': '0', 'sb-rb': '1' });

    const result = readSlots(root, '23');

    expect(result).toEqual([ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS]);
  });

  /** A hand-edited store can name a tall pick where the watch cannot draw one, and showing it would tell the user a lie about their own face. */
  test('clears a tall pick a store put somewhere it cannot go', () => {
    const root = mountStores({ 'sb-lb': '23', 'sb-rt': '0', 'sb-rb': '1' });

    const result = readSlots(root, '20');

    expect(result).toEqual([ID_EMPTY, ID_EMPTY, ID_HEART, ID_STEPS]);
  });
});

describe('writeStores', () => {
  /** Clay only notices a store it was told changed, so a silent write is a setting the watch never receives. */
  test('writes the three stores and fires a change on each', () => {
    const root = mountStores({ 'sb-lb': '20', 'sb-rt': '20', 'sb-rb': '20' });
    const changed: string[] = [];
    document.querySelectorAll('.gl-store').forEach((store) => {
      store.addEventListener('change', () => changed.push((store as HTMLInputElement).value));
    });

    writeStores(root, [ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS]);

    expect(storeFor(root, SLOT_RT).value).toBe('0');
    expect(storeFor(root, SLOT_RB).value).toBe('1');
    expect(changed).toEqual(['0', '1']);
  });

  /** A store rewritten with what it already held would fire a change Clay answers, and the answer writes back here. */
  test('leaves an unchanged store alone rather than firing a change', () => {
    const root = mountStores({ 'sb-lb': '20', 'sb-rt': '0', 'sb-rb': '1' });
    let fired = 0;
    document.querySelectorAll('.gl-store').forEach((store) => {
      store.addEventListener('change', () => { fired += 1; });
    });

    writeStores(root, [ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS]);

    expect(fired).toBe(0);
  });
});

describe('sanitize', () => {
  /** A valid arrangement must come back untouched, or every read would quietly rewrite the user's face. */
  test('leaves a placeable arrangement as it is', () => {
    const result = sanitize([ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS]);

    expect(result).toEqual([ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS]);
  });

  /** A tall pick outside its column renders as a labelled panel with nothing in it, so it is cleared rather than shown. */
  test('empties a tall pick that is not in the upper left', () => {
    const result = sanitize([ID_HEART, ID_SENSORS, ID_STEPS, ID_EMPTY]);

    expect(result).toEqual([ID_HEART, ID_EMPTY, ID_STEPS, ID_EMPTY]);
  });

  /** A short blob would leave the later slots undefined, and the builder would index a panel that is not there. */
  test('pads a short arrangement out to four', () => {
    const result = sanitize([ID_HEART]);

    expect(result).toEqual([ID_HEART, ID_EMPTY, ID_EMPTY, ID_EMPTY]);
  });

  /** A blob from a face with more panels must be cut down, not carried into a loop that only draws four. */
  test('cuts a long arrangement down to four', () => {
    const result = sanitize([ID_HEART, ID_STEPS, ID_HEART, ID_STEPS, ID_HEART, ID_STEPS]);

    expect(result).toEqual([ID_HEART, ID_STEPS, ID_HEART, ID_STEPS]);
  });
});

describe('defaults', () => {
  /** Handing back the shared table would let the first edit after a reset rewrite what reset means for the rest of the session. */
  test('hands back a fresh copy each time', () => {
    const first = defaults();
    first[0] = 99;

    const result = defaults();

    expect(result[0]).toBe(ID_SENSORS);
  });
});
