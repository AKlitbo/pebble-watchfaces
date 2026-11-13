/**
 * The layout builder's warning about placed panels that read a Pebble app plugin the phone cannot
 * serve right now, so the wearer hears what to turn on before the panel sits empty on the watch.
 *
 * The framework records where each plugin kind stands as the page opens, in the row with the
 * `pf-plugin-kinds` class. A face that reads no plugin has no such row, so this shows nothing there.
 * Phone Battery, Battery Battery, and Now Playing only ever read a plugin. A weather, stock, or
 * calendar panel only reads one while its Data Source is the Pebble app plugin, which the face's
 * page code hands in through `setPicked`.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's initialize, which runs in
 * the config webview, so it sticks to browser APIs.
 */

import { CALLOUT_BADGE, CALLOUT_BOX } from '../../../../../../paf/ts/clay/callout-style';
import { ROLE_NONE } from './modes';
import type { LayoutLibrary } from './modes';
import { EMPTY_LAYOUT } from './wire';

/** A kind of data a panel can read from a plugin, by its name in the face's providers list. */
export type PluginKind = 'weather' | 'stocks' | 'calendar' | 'music' | 'phone';

/** Where a kind's plugins stand as the page opens, as the framework records it. */
export type KindState = 'off' | 'missing' | 'served';

/** Which kinds the wearer picked the plugin for, on the selects that offer a choice. */
export type PickedKinds = Partial<Record<PluginKind, boolean>>;

/**
 * One warning: the panels it names and why they cannot fill. An `off` warning covers every kind at
 * once, since plugins off in the Pebble app stops them all, so it carries no kind of its own.
 */
export interface PluginWarning {
  state: 'off' | 'missing';
  kind: PluginKind | null;
  panels: string[];
}

/** The kind each panel reads, keyed by label like module-groups.ts. A panel left out reads no plugin. */
const PANEL_KINDS: Record<string, PluginKind> = {
  'Phone Battery':     'phone',
  'Battery Battery':   'phone',
  'Now Playing':       'music',
  'Weather':           'weather',
  'Conditions':        'weather',
  'Temperature':       'weather',
  'Feels Like':        'weather',
  'Hi/Low':            'weather',
  'Humidity':          'weather',
  'Precipitation':     'weather',
  'Dew Point':         'weather',
  'Wind Speed':        'weather',
  'Pressure':          'weather',
  'UV':                'weather',
  'Forecast (Hourly)': 'weather',
  'Forecast (4-Day)':  'weather',
  'Sun':               'weather',
  'Sunrise':           'weather',
  'Sunset':            'weather',
  'Daylight':          'weather',
  'Stock':             'stocks',
  'Watchlist':         'stocks',
  'Next Event':        'calendar',
  'Agenda':            'calendar',
  'Timeline':          'calendar',
};

/** The kinds a wearer picks a source for, so their panels only read a plugin while it is picked. */
const CHOSEN_KINDS: PluginKind[] = ['weather', 'stocks', 'calendar'];

/** The order missing warnings show in, and what each kind needs, for the warning's words. */
const KIND_NEEDS: Array<[PluginKind, string]> = [
  ['phone', 'the Phone plugin'],
  ['music', 'a music plugin'],
  ['weather', 'a weather plugin'],
  ['stocks', 'a stock plugin'],
  ['calendar', 'a calendar plugin'],
];

/**
 * Works out which placed panels cannot fill, and why.
 *
 * @param labels The labels of the panels on the grid, in any order, repeats allowed.
 * @param states Where each kind's plugins stand. A kind left out reads no plugin on this face.
 * @param picked Which kinds the wearer picked the plugin for as the Data Source.
 * @return The warnings, plugins off first and then one for each kind with no plugin, each naming its
 *   panels once in the order they were first placed. Empty when every placed panel can fill.
 */
export function pluginWarnings(labels: string[], states: Partial<Record<string, KindState>>, picked: PickedKinds): PluginWarning[] {
  const off: string[] = [];
  const missing: Partial<Record<PluginKind, string[]>> = {};

  labels.forEach(function (label) {
    const kind = PANEL_KINDS[label];

    if (!kind || (CHOSEN_KINDS.indexOf(kind) >= 0 && !picked[kind])) {
      return;
    }

    const state = states[kind];
    let panels: string[] | undefined;

    if (state === 'off') {
      panels = off;
    } else if (state === 'missing') {
      panels = missing[kind] = missing[kind] || [];
    }

    if (panels && panels.indexOf(label) < 0) {
      panels.push(label);
    }
  });

  const warnings: PluginWarning[] = [];

  if (off.length) {
    warnings.push({ state: 'off', kind: null, panels: off });
  }

  KIND_NEEDS.forEach(function (entry) {
    const panels = missing[entry[0]];

    if (panels) {
      warnings.push({ state: 'missing', kind: entry[0], panels: panels });
    }
  });

  return warnings;
}

/**
 * The names of the panels on every layout the watch shows, day, night, and Quiet Time, each once.
 *
 * The watch starts the phone and music readings for a panel on any of them, so a panel placed only on
 * the night layout still needs its plugin. A layout no role is assigned to never reaches the watch, so
 * its panels are left out.
 *
 * @param library The face's layouts and the roles assigned to them.
 * @param parse The face's own reader for a layout string, since each face lays out its grid its own way.
 * @param labelOf A panel's name, from its id.
 * @return The names, in the order each was first placed, day first.
 */
export function assignedLabels(library: LayoutLibrary, parse: (layout: string) => Array<{ module: number }>, labelOf: (module: number) => string): string[] {
  const labels: string[] = [];

  [library.day, library.night, library.quiet].forEach(function (index) {
    if (index === ROLE_NONE) {
      return;
    }

    parse(library.layouts[index] || EMPTY_LAYOUT).forEach(function (block) {
      const label = labelOf(block.module);

      if (labels.indexOf(label) < 0) {
        labels.push(label);
      }
    });
  });

  return labels;
}

/** The panels' names as a sentence's subject, such as "Stock and Watchlist". */
function nameList(panels: string[]): string {
  if (panels.length < 3) {
    return panels.join(' and ');
  }

  return panels.slice(0, -1).join(', ') + ', and ' + panels[panels.length - 1];
}

/**
 * The words a warning shows.
 *
 * @param warning The warning.
 * @return One or two sentences saying what to do in the Pebble app.
 */
export function warningText(warning: PluginWarning): string {
  const subject = nameList(warning.panels) + (warning.panels.length > 1 ? ' need ' : ' needs ');

  if (warning.state === 'off') {
    return subject + 'plugins turned on in the Pebble app, under Settings, Plugins.';
  }

  let needs = 'a plugin';

  KIND_NEEDS.forEach(function (entry) {
    if (entry[0] === warning.kind) {
      needs = entry[1];
    }
  });

  return subject + needs + ' in the Pebble app, and your phone has none yet.';
}

/** Reads the framework's plugin kinds row back, as nothing for a page without one or a value that is not an object. */
function readStates(): Partial<Record<string, KindState>> {
  const store = document.querySelector('.pf-plugin-kinds') as HTMLInputElement | null;

  if (!store || !store.value) {
    return {};
  }

  try {
    const parsed: unknown = JSON.parse(store.value);

    return parsed && typeof parsed === 'object' && !Array.isArray(parsed) ? parsed as Partial<Record<string, KindState>> : {};
  } catch {
    return {};
  }
}

/** The warnings under the watch, kept in step with the grid and the sources the wearer picked. */
export interface PluginWarningsView {
  /** Redraws the warnings for the panels now on the grid. */
  update(labels: string[]): void;
  /** Redraws the warnings for the sources the wearer has picked. */
  setPicked(picked: PickedKinds): void;
}

/**
 * Puts the warning box on the page, straight after the given element, and returns the handle that
 * keeps it current. The box stays empty and hidden while every placed panel can fill.
 *
 * @param after The element the box goes after, normally the watch and its buttons.
 * @return The handle to redraw it with.
 */
export function createPluginWarnings(after: HTMLElement): PluginWarningsView {
  const states = readStates();
  const box = document.createElement('div');
  let labels: string[] = [];
  let picked: PickedKinds = {};

  box.className = 'lb-plugin-warnings';
  // the side padding lines the callouts up with the palette below
  box.setAttribute('style', 'padding: 0 12px;');

  if (after.parentNode) {
    after.parentNode.insertBefore(box, after.nextSibling);
  }

  function draw(): void {
    box.innerHTML = '';

    pluginWarnings(labels, states, picked).forEach(function (warning) {
      // the look of the framework's own plugin notes on the same page. the badge is a span here, since
      // an inline style cannot reach a :before, and its content line is ignored on a span
      const note = document.createElement('div');
      const badge = document.createElement('span');
      const words = document.createElement('span');

      note.className = 'lb-plugin-warning';
      note.setAttribute('style', CALLOUT_BOX);
      badge.setAttribute('style', CALLOUT_BADGE);
      badge.textContent = '!';
      words.textContent = warningText(warning);
      note.appendChild(badge);
      note.appendChild(words);
      box.appendChild(note);
    });

    box.hidden = !box.firstChild;
  }

  draw();

  return {
    update: function (next) {
      labels = next;
      draw();
    },
    setPicked: function (next) {
      picked = next;
      draw();
    },
  };
}
