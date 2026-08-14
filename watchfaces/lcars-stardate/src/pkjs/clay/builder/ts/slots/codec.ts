/**
 * Reading and writing the four slot values.
 *
 * A Clay component owns exactly one message key, and this face has four. So the builder's own key
 * carries the upper-left slot and the other three ride hidden stores on the same page, each found
 * by its own class. Every value is the catalog id written as text, which is what the watch's
 * enum settings already expect off the wire.
 */

import { DEFAULT_SLOTS, ID_EMPTY, SLOT_COUNT, SLOT_STORE_CLASS, canPlace } from './geometry';

/** Read a slot id out of a hidden input's text, falling back to empty. */
export function parseId(text: string | null | undefined): number {
  const n = parseInt(text || '', 10);
  return isNaN(n) || n < 0 ? ID_EMPTY : n;
}

/** The four ids as text, in slot order, ready for the inputs they live in. */
export function formatId(id: number): string {
  return String(id);
}

/**
 * Find the hidden input backing a slot.
 *
 * Every hiddenStore carries .gl-store, so each one on the page is given its own extra class and
 * that is what this looks for. Returns null for the upper-left slot, which is the component's own
 * value rather than a store, and for a store that is not on the page.
 *
 * @param root The component element, used only to reach the document.
 * @param slot The slot to find.
 */
export function storeFor(root: HTMLElement, slot: number): HTMLInputElement | null {
  const cls = SLOT_STORE_CLASS[slot];
  if (!cls) {
    return null;
  }

  const doc = root.ownerDocument || document;
  return doc.querySelector('.' + cls) as HTMLInputElement | null;
}

/**
 * The four current slot values.
 *
 * @param root The component element.
 * @param ownValue The component's own value, which is the upper-left slot.
 */
export function readSlots(root: HTMLElement, ownValue: string): number[] {
  const slots = [];

  for (let slot = 0; slot < SLOT_COUNT; slot++) {
    const store = storeFor(root, slot);
    slots.push(parseId(store ? store.value : ownValue));
  }

  return sanitize(slots);
}

/**
 * Push the three store-backed slots out to their inputs.
 *
 * Each gets a change event so Clay notices, the same way the layout builder nudges its night
 * store. The upper-left slot is not written here: it is the component's own value and goes back
 * through the manipulator.
 *
 * @param root The component element.
 * @param slots The four values.
 */
export function writeStores(root: HTMLElement, slots: number[]): void {
  for (let slot = 1; slot < SLOT_COUNT; slot++) {
    const store = storeFor(root, slot);
    const next = formatId(slots[slot]);

    if (store && store.value !== next) {
      store.value = next;
      store.dispatchEvent(new Event('change'));
    }
  }
}

/**
 * Fix up a set of slots that came from somewhere untrusted.
 *
 * A stored blob or a hand-edited store can hold a tall pick in a slot that cannot show one, which
 * would render as a labelled panel with nothing in it. The firmware makes the same correction, so
 * this is only about the builder showing the truth.
 *
 * @param slots The four values.
 * @return The four values, corrected.
 */
export function sanitize(slots: number[]): number[] {
  const out = slots.slice(0, SLOT_COUNT);

  while (out.length < SLOT_COUNT) {
    out.push(ID_EMPTY);
  }

  for (let slot = 0; slot < SLOT_COUNT; slot++) {
    if (!canPlace(out[slot], slot)) {
      out[slot] = ID_EMPTY;
    }
  }

  return out;
}

/** A fresh copy of the shipped arrangement. */
export function defaults(): number[] {
  return DEFAULT_SLOTS.slice();
}
