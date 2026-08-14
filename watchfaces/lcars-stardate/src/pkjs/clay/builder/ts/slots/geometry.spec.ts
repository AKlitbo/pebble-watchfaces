/**
 * Specs for the slot placement rules.
 *
 * There is no grid here, just four fixed panels and one rule: the sensors block
 * fills a whole column, so it only fits the upper left. Everything below is
 * either that rule or one of the three lookup tables the rest of the builder
 * indexes by slot, where a table one entry short fails silently rather than
 * throwing.
 */

import { describe, test, expect } from 'vitest';
import {
  isBottom, isTall, canPlace, swallowedBy,
  SLOT_LT, SLOT_LB, SLOT_RT, SLOT_RB, SLOT_COUNT,
  SLOT_KEYS, SLOT_STORE_CLASS, DEFAULT_SLOTS,
  ID_HEART, ID_EMPTY, ID_SENSORS,
} from './geometry';

describe('isBottom', () => {
  /** The lower panel of each column is the one a tall pick swallows, so mistaking it drops the wrong panel. */
  test('reports the lower panel of each column', () => {
    const result = [isBottom(SLOT_LT), isBottom(SLOT_LB), isBottom(SLOT_RT), isBottom(SLOT_RB)];

    expect(result).toEqual([false, true, false, true]);
  });
});

describe('isTall', () => {
  /** Only the sensors block fills a column, and treating an ordinary readout as tall would blank the panel under it. */
  test('reports only the sensors block as tall', () => {
    const result = [isTall(ID_SENSORS), isTall(ID_HEART), isTall(ID_EMPTY)];

    expect(result).toEqual([true, false, false]);
  });
});

describe('canPlace', () => {
  /** An ordinary readout fits any panel, and refusing one would make the builder reject a drop the watch accepts. */
  test('allows an ordinary readout in every slot', () => {
    const result = [
      canPlace(ID_HEART, SLOT_LT), canPlace(ID_HEART, SLOT_LB),
      canPlace(ID_HEART, SLOT_RT), canPlace(ID_HEART, SLOT_RB),
    ];

    expect(result).toEqual([true, true, true, true]);
  });

  /** The watch draws the sensors block in one column only, so accepting it elsewhere renders a labelled panel with nothing in it. */
  test('allows the tall block only in the upper left', () => {
    const result = [
      canPlace(ID_SENSORS, SLOT_LT), canPlace(ID_SENSORS, SLOT_LB),
      canPlace(ID_SENSORS, SLOT_RT), canPlace(ID_SENSORS, SLOT_RB),
    ];

    expect(result).toEqual([true, false, false, false]);
  });
});

describe('swallowedBy', () => {
  /** A tall pick covers the panel below it, and leaving that panel set would draw a readout underneath the sensors block. */
  test('names the lower left panel when the tall block lands upper left', () => {
    const result = swallowedBy(ID_SENSORS, SLOT_LT);

    expect(result).toBe(SLOT_LB);
  });

  /** An ordinary pick swallows nothing, so returning a slot here would silently clear a panel the user never touched. */
  test('swallows nothing for an ordinary pick', () => {
    const result = [swallowedBy(ID_HEART, SLOT_LT), swallowedBy(ID_SENSORS, SLOT_RT)];

    expect(result).toEqual([-1, -1]);
  });
});

describe('the slot tables', () => {
  /**
   * A table one entry short is the quiet failure here.
   *
   * readSlots walks to SLOT_COUNT and indexes both tables by slot, and a missing class reads as
   * "no store" rather than as an error, so the slot silently falls back to the component's own
   * value and two panels end up mirroring each other.
   */
  test('carry an entry for every slot', () => {
    const result = [SLOT_KEYS.length, SLOT_STORE_CLASS.length, DEFAULT_SLOTS.length];

    expect(result).toEqual([SLOT_COUNT, SLOT_COUNT, SLOT_COUNT]);
  });

  /** The upper left is the component's own value rather than a hidden store, and giving it a class would send it looking for an input that is not on the page. */
  test('leave the upper left without a store class', () => {
    const result = SLOT_STORE_CLASS[SLOT_LT];

    expect(result).toBeNull();
  });

  /** Each store needs its own class to be found by, since every hiddenStore already shares .gl-store. */
  test('give the other three slots distinct store classes', () => {
    const classes = SLOT_STORE_CLASS.slice(1);

    const result = new Set(classes).size;

    expect(result).toBe(SLOT_COUNT - 1);
    expect(classes).not.toContain(null);
  });

  /** The face ships with this arrangement, so a default the rules reject would blank a panel on a watch nobody has configured. */
  test('ship a default arrangement every slot accepts', () => {
    const result = DEFAULT_SLOTS.map((id, slot) => canPlace(id, slot));

    expect(result).toEqual([true, true, true, true]);
  });
});
