/**
 * The layout library: the grids you build, and which of them the watch uses when.
 *
 * The grid on screen edits whichever layout the tab strip has selected, and every edit is kept —
 * there is no Save button because there is nothing to lose. Some of them are then assigned a job:
 * one is the day layout, one takes over after dark, one takes over while Quiet Time is on. Only
 * the assigned ones reach the watch.
 *
 * Everything except those two lives in the hidden gl-store input on the page, so Clay saves it to
 * the phone and seeds it back next time. The config webview blocks its own localStorage, which is
 * why the store is an input rather than something sensible.
 *
 * This is a Clay builder piece. esbuild bundles it into the layout component's initialize, which
 * runs in the config webview, so it sticks to browser APIs.
 */

import { EMPTY_LAYOUT } from './wire';

/** How many layouts the library holds. Some get used; the rest are somewhere to keep a design. */
export const LAYOUT_COUNT = 5;

/** What an assignment reads as when the job has no layout. */
export const ROLE_NONE = -1;

/** A job a layout can be assigned to. Day always has one, the others can be left unassigned. */
export type LayoutRole = 'day' | 'night' | 'quiet';

/** The library as it sits in the hidden store. */
export interface LayoutLibrary {
  /** One wire string per layout, always LAYOUT_COUNT long. */
  layouts: string[];
  /** Which layout the watch shows by day. */
  day: number;
  /** And after dark, or ROLE_NONE to stay on the day one all night. */
  night: number;
  /** And while Quiet Time is on, or ROLE_NONE to leave the hour to decide. */
  quiet: number;
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
    night: clampIndex(raw.night, ROLE_NONE),
    quiet: clampIndex(raw.quiet, ROLE_NONE),
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
  return { layouts: layouts, day: 0, night: library.night, quiet: library.quiet };
}

/**
 * Builds the tab strip and the assignment rows into a host element.
 *
 * The Quiet Time row only appears on a page that has somewhere to put the answer. Every mosaic
 * face builds its grid through this same file, so without that test a face with no such store
 * would grow a picker backed by nothing.
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

  // only offered where the page has a store to keep the answer in
  const quietWanted = document.querySelector('.gl-quiet') !== null;

  /** Stash whatever is on the grid into the layout it belongs to. */
  function keep(): void {
    library.layouts[selected] = opts.getCurrent();
    opts.save();
  }

  function label(index: number): string {
    // one mark per job the layout has been given, so the strip says at a glance which are spares
    const marks = (library.day === index ? '☀' : '')
      + (library.night === index ? '☽' : '')
      + (library.quiet === index ? '⊘' : '');
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

    // the rows were built in this order, and the quiet one may not be there at all
    const order: LayoutRole[] = quietWanted ? ['day', 'night', 'quiet'] : ['day', 'night'];
    const selects = assignments.querySelectorAll<HTMLSelectElement>('select');
    for (let i = 0; i < selects.length && i < order.length; i++) {
      selects[i].value = String(library[order[i]]);
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

  /** One assignment row: a caption and a picker over every layout in the library. */
  function assignRow(name: string, role: LayoutRole): void {
    const row = document.createElement('div');
    row.className = 'lb-assign-row';

    const caption = document.createElement('span');
    caption.className = 'lb-assign-lbl';
    caption.textContent = name;

    const select = document.createElement('select');
    select.className = 'lb-assign-sel';

    // day always has a layout, so only the jobs that can be skipped offer None
    if (role !== 'day') {
      const none = document.createElement('option');
      none.value = String(ROLE_NONE);
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
      library[role] = clampIndex(select.value, role === 'day' ? 0 : ROLE_NONE);
      opts.save();
      redraw();
      opts.onAssign();
    });

    row.appendChild(caption);
    row.appendChild(select);
    assignments.appendChild(row);
  }

  assignRow('Day', 'day');
  assignRow('Night', 'night');
  if (quietWanted) {
    assignRow('Quiet Time', 'quiet');
  }

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
