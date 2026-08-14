/**
 * Specs for what a panel drag is allowed to do.
 *
 * The pointer handling lives in lib and jsdom cannot exercise it, since every
 * rect is zeros and no drop would ever hit a panel. What is this face's own is
 * tryPlace, which decides the whole arrangement rather than just the panel
 * under the pointer, because a swap can push the tall block somewhere it does
 * not fit. Every case below is one of those knock-on moves.
 */

import { describe, test, expect } from 'vitest';
import { tryPlace } from './drag';
import {
  SLOT_LT, SLOT_LB, SLOT_RT, SLOT_RB,
  ID_HEART, ID_STEPS, ID_EMPTY, ID_SENSORS,
} from './geometry';

/** Dragged straight off the palette rather than out of a panel. */
const FROM_PALETTE = -1;

// what the face ships as: the tall block over the left column, heart and steps down the right
const SHIPPED = [ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS];

describe('tryPlace', () => {
  /** A palette drop is the ordinary case, and refusing it would make the builder look broken on first use. */
  test('drops a palette pick into the panel under the pointer', () => {
    const result = tryPlace(SHIPPED, ID_STEPS, FROM_PALETTE, SLOT_RT);

    expect(result).toEqual([ID_SENSORS, ID_EMPTY, ID_STEPS, ID_STEPS]);
  });

  /** Dragging between panels trades them, and writing only the target would leave the dragged readout in both places. */
  test('swaps the two panels when a pick is dragged from one to another', () => {
    const result = tryPlace(SHIPPED, ID_HEART, SLOT_RT, SLOT_RB);

    expect(result).toEqual([ID_SENSORS, ID_EMPTY, ID_STEPS, ID_HEART]);
  });

  /** The tall block fills its column, so whatever sat below it has to go rather than be pushed aside. */
  test('clears the panel below when the tall block lands upper left', () => {
    const slots = [ID_EMPTY, ID_HEART, ID_STEPS, ID_EMPTY];

    const result = tryPlace(slots, ID_SENSORS, FROM_PALETTE, SLOT_LT);

    expect(result).toEqual([ID_SENSORS, ID_EMPTY, ID_STEPS, ID_EMPTY]);
  });

  /** The watch draws the tall block in one column, so a drop anywhere else has to be refused rather than shown and then corrected. */
  test('refuses the tall block anywhere but the upper left', () => {
    const slots = [ID_EMPTY, ID_HEART, ID_STEPS, ID_EMPTY];

    const result = [
      tryPlace(slots, ID_SENSORS, FROM_PALETTE, SLOT_LB),
      tryPlace(slots, ID_SENSORS, FROM_PALETTE, SLOT_RT),
      tryPlace(slots, ID_SENSORS, FROM_PALETTE, SLOT_RB),
    ];

    expect(result).toEqual([null, null, null]);
  });

  /**
   * A swap into the upper left would push the tall block out of the one column it fits.
   *
   * This is why the whole arrangement is checked rather than the target panel alone. Looking only
   * at where the heart lands says yes, and the move quietly strands the sensors block on the right.
   */
  test('refuses a swap that would push the tall block out of its column', () => {
    const result = tryPlace(SHIPPED, ID_HEART, SLOT_RT, SLOT_LT);

    expect(result).toBeNull();
  });

  /** Dragging the tall block down its own column is the same bug from the other side. */
  test('refuses moving the tall block into the panel below it', () => {
    const result = tryPlace(SHIPPED, ID_SENSORS, SLOT_LT, SLOT_LB);

    expect(result).toBeNull();
  });

  /** A pick let go on the panel it came from must land back as it was, not swap the panel with itself into something else. */
  test('leaves the arrangement alone when a pick is dropped on its own panel', () => {
    const result = tryPlace(SHIPPED, ID_HEART, SLOT_RT, SLOT_RT);

    expect(result).toEqual(SHIPPED);
  });

  /** The arrangement handed in is the live one, so mutating it would move panels the user only dragged over. */
  test('does not touch the arrangement it was given', () => {
    const slots = SHIPPED.slice();

    tryPlace(slots, ID_STEPS, FROM_PALETTE, SLOT_RT);

    expect(slots).toEqual(SHIPPED);
  });
});
