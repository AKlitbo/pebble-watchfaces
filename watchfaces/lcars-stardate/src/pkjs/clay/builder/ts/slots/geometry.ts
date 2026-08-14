/**
 * Every fact about the shape of this face's panel layout lives here, so pointing the builder at a
 * different arrangement is a matter of editing this file rather than chasing constants through the
 * drag and render code.
 *
 * There is no grid: four fixed positions, two per column. The only placement rule is that the
 * sensors block fills a whole column and so may only sit in a column's top slot.
 */

/** The four panels, in the order the watch numbers them. */
export const SLOT_LT = 0;
export const SLOT_LB = 1;
export const SLOT_RT = 2;
export const SLOT_RB = 3;
export const SLOT_COUNT = 4;

/** The catalog ids the builder has to reason about by name. Everything else is just a number. */
export const ID_HEART = 0;
export const ID_STEPS = 1;
export const ID_EMPTY = 20;
export const ID_SENSORS = 23;

/** What the face ships as, and what the reset button puts back. */
export const DEFAULT_SLOTS = [ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS];

/** A slot's message key, in slot order. The first is the component's own. */
export const SLOT_KEYS = [
  'APPEARANCE_SLOT_LEFT_TOP',
  'APPEARANCE_SLOT_LEFT_BOTTOM',
  'APPEARANCE_SLOT_RIGHT_TOP',
  'APPEARANCE_SLOT_RIGHT_BOTTOM',
];

/** The hidden-store class each of the other three slots is found by. */
export const SLOT_STORE_CLASS = [null, 'sb-lb', 'sb-rt', 'sb-rb'];

/** Whether a slot is the lower of its column. */
export function isBottom(slot: number): boolean {
  return slot === SLOT_LB || slot === SLOT_RB;
}

/**
 * Whether a readout fills both rows of its column.
 *
 * Only the sensors block does. It is not a flag on the option list because the watch decides the
 * same thing from its own catalog, and two sources would be one too many.
 */
export function isTall(id: number): boolean {
  return id === ID_SENSORS;
}

/**
 * Whether a readout may sit in a slot.
 *
 * A tall one only in the upper left, which is the one column the watch draws it in. The firmware
 * enforces this too, so a hand-edited setting cannot smuggle one in.
 */
export function canPlace(id: number, slot: number): boolean {
  return !isTall(id) || slot === SLOT_LT;
}

/** Which slot a tall pick swallows, or -1 when the pick is a normal one. */
export function swallowedBy(id: number, slot: number): number {
  return isTall(id) && slot === SLOT_LT ? SLOT_LB : -1;
}
