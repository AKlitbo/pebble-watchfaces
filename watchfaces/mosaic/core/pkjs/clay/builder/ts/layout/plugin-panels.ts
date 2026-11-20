/**
 * What the framework's plugin warnings need from the layout builder: the kind each mosaic panel
 * reads, and the panels on the layouts the watch shows.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's initialize, which runs in
 * the config webview, so it sticks to browser APIs.
 */

import { ROLE_NONE } from './modes';
import type { LayoutLibrary } from './modes';
import { EMPTY_LAYOUT } from './wire';
import type { PluginKind } from '../../../../../../paf/ts/clay/builder/ts/shared/plugin-warnings';

/** The kind each panel reads, keyed by label like module-groups.ts. A panel left out reads no plugin. */
export const PANEL_KINDS: Record<string, PluginKind> = {
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
