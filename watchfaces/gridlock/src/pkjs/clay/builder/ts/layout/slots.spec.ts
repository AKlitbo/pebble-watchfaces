// @vitest-environment jsdom
/**
 * Specs for the two saved layout slots.
 *
 * The panel runs inside Clay's config webview, so its job is pure DOM: stash the
 * current layout in the hidden gl-store input, read it back, and guard the two
 * destructive taps (overwrite a saved slot, load over the layout on screen)
 * behind a confirming second tap. A regression here either silently loses a
 * saved layout or wipes the grid on a stray tap.
 *
 * The webview globals come from jsdom. Time is faked so the saved stamp is fixed
 * and the arming timeout can be driven rather than waited on.
 */

import { describe, test, expect, beforeEach, afterEach, vi } from 'vitest';
import { buildSlotsPanel } from './slots';

/** The handles a spec drives one slot row through. */
interface SlotRow {
  status: HTMLElement;
  save: HTMLButtonElement;
  load: HTMLButtonElement;
}

/** Pulls the status line and Save/Load buttons out of the nth slot row. */
function rowOf(panel: HTMLElement, index: number): SlotRow {
  const row = panel.querySelectorAll('.lb-slot-row')[index];
  const buttons = row.querySelectorAll('.lb-io-btn');
  return {
    status: row.querySelector('.lb-slot-status') as HTMLElement,
    save: buttons[0] as HTMLButtonElement,
    load: buttons[1] as HTMLButtonElement,
  };
}

/**
 * Builds the panel into a fresh page next to a hidden store input.
 *
 * current is what a Save reads as the layout on screen. initialStore seeds the
 * store the way Clay would after restoring saved settings. Returns the store,
 * the layouts Load handed back, and accessors for both slot rows.
 */
function mount(current: string, initialStore?: string) {
  document.body.innerHTML = '';

  const store = document.createElement('input');
  store.type = 'hidden';
  store.className = 'gl-store';
  if (initialStore) {
    store.value = initialStore;
  }
  document.body.appendChild(store);

  const panel = document.createElement('div');
  document.body.appendChild(panel);

  const loaded: string[] = [];
  let currentLayout = current;

  buildSlotsPanel(panel, {
    getCurrent: function () {
      return currentLayout;
    },
    onLoad: function (text) {
      loaded.push(text);
    },
  });

  return {
    store,
    loaded,
    setCurrent: function (value: string) {
      currentLayout = value;
    },
    slotA: function () {
      return rowOf(panel, 0);
    },
    slotB: function () {
      return rowOf(panel, 1);
    },
  };
}

beforeEach(() => {
  vi.useFakeTimers();
});

afterEach(() => {
  vi.useRealTimers();
});

describe('buildSlotsPanel', () => {
  /** A save that never reached the shared store would look saved but vanish when the page reopens. */
  test('saves the current layout into the shared store on a single tap of an empty slot', () => {
    vi.setSystemTime(new Date(2026, 6, 17, 14, 2));
    const harness = mount('12,0,0,2,1');

    harness.slotA().save.click();

    const stored = JSON.parse(harness.store.value);
    expect(stored.a.layout).toBe('12,0,0,2,1');
    expect(harness.slotA().status.textContent).toBe('saved 07/17 14:02');
  });

  /** Without the arming step a stray tap overwrites a saved layout the user wanted to keep. */
  test('arms on the first tap and only overwrites a filled slot on the second', () => {
    const harness = mount('1,0,0,2,1');
    harness.slotA().save.click();

    harness.setCurrent('2,0,0,2,1');
    harness.slotA().save.click();

    expect(JSON.parse(harness.store.value).a.layout).toBe('1,0,0,2,1');
    expect(harness.slotA().save.textContent).toBe('Overwrite?');

    harness.slotA().save.click();

    expect(JSON.parse(harness.store.value).a.layout).toBe('2,0,0,2,1');
  });

  /** A stray tap on Load would discard the layout the user is editing. */
  test('applies a stored layout only on the confirming second tap of Load', () => {
    const harness = mount('9,0,0,2,1');
    harness.slotA().save.click();

    harness.slotA().load.click();

    expect(harness.loaded).toEqual([]);
    expect(harness.slotA().load.textContent).toBe('Load?');

    harness.slotA().load.click();

    expect(harness.loaded).toEqual(['9,0,0,2,1']);
  });

  /** Loading an empty slot would blank the grid, so Load stays disabled until something is saved. */
  test('keeps Load disabled while the slot is empty', () => {
    const harness = mount('9,0,0,2,1');

    const result = harness.slotB().load;

    expect(result.disabled).toBe(true);
  });

  /** If the arming never expired, a forgotten slot would overwrite on the next unrelated tap. */
  test('disarms a primed Save after the reset delay so a late tap re-arms instead of overwriting', () => {
    const harness = mount('2,0,0,2,1', JSON.stringify({ a: { layout: '1,0,0,2,1', ts: 0 } }));
    harness.slotA().save.click();

    // the arming window is 2500ms, so this walks well past it
    vi.advanceTimersByTime(3000);
    harness.slotA().save.click();

    expect(JSON.parse(harness.store.value).a.layout).toBe('1,0,0,2,1');
    expect(harness.slotA().save.textContent).toBe('Overwrite?');
  });
});
