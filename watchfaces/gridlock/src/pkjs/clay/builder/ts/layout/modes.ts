/**
 * The layout library: four grids you build, and which two of them the watch uses.
 *
 * The grid on screen edits whichever layout the tab strip has selected, and every edit is kept —
 * there is no Save button because there is nothing to lose. Two of the four are then assigned:
 * one is the day layout, one is the night layout, and those are the only two that reach the watch.
 *
 * Everything except those two lives in the hidden gl-store input on the page, so Clay saves it to
 * the phone and seeds it back next time. The config webview blocks its own localStorage, which is
 * why the store is an input rather than something sensible.
 *
 * This is a Clay builder piece. esbuild bundles it into the layout component's initialize, which
 * runs in the config webview, so it sticks to browser APIs.
 */

import { EMPTY_LAYOUT } from './codec';

/** How many layouts the library holds. Two get used; the rest are somewhere to keep a design. */
export const LAYOUT_COUNT = 4;

/** Which layout is the night one when none is. */
export const NIGHT_NONE = -1;

/** The library as it sits in the hidden store. */
export interface LayoutLibrary {
  /** One wire string per layout, always LAYOUT_COUNT long. */
  layouts: string[];
  /** Which layout the watch shows by day. */
  day: number;
  /** And after dark, or NIGHT_NONE to stay on the day one all night. */
  night: number;
}

/** How the tab strip reaches the builder around it. */
export interface ModesOpts {
  /** The wire string for whatever is on the grid right now. */
  getCurrent: () => string;
  /** Put a layout on the grid. The index comes too: several layouts are usually identical, so
   *  the string alone cannot say which one was picked. */
  onSelect: (layout: string, index: number) => void;
  /** Called whenever an assignment changes, so the builder can re-publish both wire values. */
  onAssign: () => void;
  /** Persist the library. The builder owns this because it is the only thing that knows whether
   *  the library has been read yet, and writing before then would save four blank grids. */
  save: () => void;
}

/** What the builder gets back: the one place that knows which layout is being edited. */
export interface ModesBar {
  /** Which layout the grid is editing. */
  selected: () => number;
  /** Redraw the tabs and pickers after something changed underneath. */
  refresh: () => void;
}

/** An index that is a real layout, or -1. */
function clampIndex(value: unknown, fallback: number): number {
  const index = typeof value === 'number' ? value : parseInt(String(value), 10);
  if (isNaN(index) || index < 0 || index >= LAYOUT_COUNT) {
    return fallback;
  }
  return index;
}

/** The library's own store input, or null when the page has none. */
function storeInput(): HTMLInputElement | null {
  // by its own class, not .gl-store: the night layout is a hidden store too, and a plain
  // .gl-store lookup would keep finding whichever of the two Clay rendered first
  return document.querySelector('.gl-library');
}

/** Whether the page has a library store yet, which it may not during the build. */
export function storePresent(): boolean {
  return storeInput() !== null;
}

/**
 * Reads the library back, repairing anything missing.
 *
 * A first run, a store the user has never saved, and a corrupt value all land here, so this never
 * throws and always returns something with LAYOUT_COUNT entries.
 */
export function readLibrary(): LayoutLibrary {
  const input = storeInput();
  let raw: Partial<LayoutLibrary> = {};

  if (input) {
    try {
      raw = JSON.parse(input.value || '') || {};
    } catch (error) {
      raw = {};
    }
  }

  const layouts: string[] = [];
  for (let i = 0; i < LAYOUT_COUNT; i++) {
    const value = raw.layouts && raw.layouts[i];
    layouts.push(typeof value === 'string' && value ? value : EMPTY_LAYOUT);
  }

  return {
    layouts: layouts,
    day: clampIndex(raw.day, 0),
    night: clampIndex(raw.night, NIGHT_NONE),
  };
}

/** Writes the library back so Clay saves it, reporting whether there was anywhere to write. */
export function writeLibrary(library: LayoutLibrary): boolean {
  const input = storeInput();
  if (!input) {
    return false;
  }

  input.value = JSON.stringify(library);
  // nudge Clay to pick it up the way a typed field would
  input.dispatchEvent(new Event('change'));
  return true;
}

/**
 * Seeds layout 0 from a layout the library has never seen.
 *
 * What an upgrade looks like: the watch already has a LAYOUT and the store is empty, so the
 * existing design becomes layout 0 and the day assignment points at it. Nothing else changes, so
 * anyone upgrading opens the page and finds their layout exactly where they left it.
 */
export function seedLibrary(library: LayoutLibrary, existing: string): LayoutLibrary {
  const untouched = library.layouts.every(function (layout) {
    return layout === EMPTY_LAYOUT;
  });

  if (!untouched || !existing || existing === EMPTY_LAYOUT) {
    return library;
  }

  const layouts = library.layouts.slice();
  layouts[0] = existing;
  return { layouts: layouts, day: 0, night: library.night };
}

/**
 * Builds the tab strip and the two assignment rows into a host element.
 *
 * @param host Where to put it, above the grid.
 * @param library The library to drive, mutated in place as the user edits.
 * @param opts How to reach the builder.
 * @return A handle onto the selection, so the builder never has to keep its own copy of it.
 */
export function buildModesBar(host: HTMLElement, library: LayoutLibrary, opts: ModesOpts): ModesBar {
  let selected = library.day;

  const tabs = document.createElement('div');
  tabs.className = 'lb-ltabs';

  const assignments = document.createElement('div');
  assignments.className = 'lb-assign';

  /** Stash whatever is on the grid into the layout it belongs to. */
  function keep(): void {
    library.layouts[selected] = opts.getCurrent();
    opts.save();
  }

  function label(index: number): string {
    const marks = (library.day === index ? '☀' : '') + (library.night === index ? '☽' : '');
    return marks ? index + 1 + ' ' + marks : String(index + 1);
  }

  function redraw(): void {
    const buttons = tabs.querySelectorAll<HTMLElement>('.lb-ltab');
    for (let i = 0; i < buttons.length; i++) {
      buttons[i].textContent = label(i);
      if (i === selected) {
        buttons[i].classList.add('active');
      } else {
        buttons[i].classList.remove('active');
      }
    }

    const selects = assignments.querySelectorAll<HTMLSelectElement>('select');
    if (selects.length === 2) {
      selects[0].value = String(library.day);
      selects[1].value = String(library.night);
    }
  }

  for (let i = 0; i < LAYOUT_COUNT; i++) {
    (function (index) {
      const tab = document.createElement('button');
      tab.type = 'button';
      tab.className = 'lb-ltab';
      tab.addEventListener('click', function () {
        if (index === selected) {
          return;
        }
        keep(); // the grid is about to be replaced, so bank it first
        selected = index;
        opts.onSelect(library.layouts[index], index);
        redraw();
      });
      tabs.appendChild(tab);
    })(i);
  }

  /** One "Day"/"Night" row: a label and a picker over the four layouts. */
  function assignRow(name: string, night: boolean): void {
    const row = document.createElement('div');
    row.className = 'lb-assign-row';

    const caption = document.createElement('span');
    caption.className = 'lb-assign-lbl';
    caption.textContent = name;

    const select = document.createElement('select');
    select.className = 'lb-assign-sel';

    if (night) {
      const none = document.createElement('option');
      none.value = String(NIGHT_NONE);
      none.textContent = 'None';
      select.appendChild(none);
    }

    for (let i = 0; i < LAYOUT_COUNT; i++) {
      const option = document.createElement('option');
      option.value = String(i);
      option.textContent = 'Layout ' + (i + 1);
      select.appendChild(option);
    }

    select.addEventListener('change', function () {
      keep(); // the picker reads the library, so make sure the grid is in it first
      const index = clampIndex(select.value, night ? NIGHT_NONE : 0);
      if (night) {
        library.night = index;
      } else {
        library.day = index;
      }
      opts.save();
      redraw();
      opts.onAssign();
    });

    row.appendChild(caption);
    row.appendChild(select);
    assignments.appendChild(row);
  }

  assignRow('Day', false);
  assignRow('Night', true);

  host.appendChild(tabs);
  host.appendChild(assignments);
  redraw();

  return {
    selected: function () {
      return selected;
    },
    refresh: redraw,
  };
}
