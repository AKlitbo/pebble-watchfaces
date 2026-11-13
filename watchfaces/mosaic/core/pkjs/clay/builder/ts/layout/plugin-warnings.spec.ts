/**
 * Specs for which placed panels the layout builder warns about.
 *
 * The warning is the wearer's one chance to hear a panel will sit empty before saving. Worth pinning
 * are the rules a reader cannot check by eye: a panel that only reads a plugin warns whatever else is
 * picked, a weather, stock, or calendar panel only warns while its source is the plugin, a served
 * kind and a face with no plugin kinds never warn, and plugins off folds every kind into one warning.
 * The panels it weighs are the ones on every layout a role is assigned to, and no others.
 */

import { describe, test, expect } from 'vitest';
import { assignedLabels, pluginWarnings } from './plugin-warnings';
import { ROLE_NONE } from './modes';

describe('pluginWarnings', () => {
  /** With plugins off the phone and music panels sit empty, so both are named in the one warning. */
  test('folds every panel into one warning while plugins are off', () => {
    const states = { phone: 'off', music: 'off', weather: 'off' } as const;

    const result = pluginWarnings(['Phone Battery', 'Now Playing', 'Battery Battery'], states, {});

    expect(result).toEqual([{ state: 'off', kind: null, panels: ['Phone Battery', 'Now Playing', 'Battery Battery'] }]);
  });

  /** A weather panel on a fetched source fills with plugins off, so warning there would send the wearer looking for nothing. */
  test('leaves out a weather panel whose source is not the plugin', () => {
    const states = { weather: 'off' } as const;

    const result = pluginWarnings(['Weather', 'Humidity'], states, { weather: false });

    expect(result).toEqual([]);
  });

  /** A stock source left on the plugin with no stock plugin installed shows NO PLUGIN on the watch. */
  test('warns about a stock panel set to the plugin when the phone has none', () => {
    const states = { stocks: 'missing', weather: 'served' } as const;

    const result = pluginWarnings(['Weather', 'Stock', 'Watchlist', 'Stock'], states, { stocks: true, weather: true });

    expect(result).toEqual([{ state: 'missing', kind: 'stocks', panels: ['Stock', 'Watchlist'] }]);
  });

  /** A served kind fills its panels, so naming them would be a false alarm. */
  test('says nothing about a kind a plugin serves', () => {
    const states = { phone: 'served', music: 'served', calendar: 'served' } as const;

    const result = pluginWarnings(['Phone Battery', 'Now Playing', 'Agenda'], states, { calendar: true });

    expect(result).toEqual([]);
  });

  /** A face that reads no plugin has no kinds row on its page, so it must show no warning at all. */
  test('says nothing on a page with no plugin kinds', () => {
    const result = pluginWarnings(['Weather', 'Phone Battery'], {}, { weather: true });

    expect(result).toEqual([]);
  });
});

describe('assignedLabels', () => {
  /** The panels' first fields, as the faces' own readers give them back, so each test reads the ids it placed. */
  function parse(layout: string): Array<{ module: number }> {
    return layout === '0' ? [] : layout.split(';').map((record) => ({ module: Number(record.split(',')[0]) }));
  }

  const NAMES: Record<number, string> = { 2: 'Battery', 3: 'Weather', 20: 'Moon', 75: 'Phone Battery' };

  /**
   * The watch starts a panel's readings on any layout a role is assigned to, and ignores the rest. A
   * panel only on the night layout that went unnamed would sit empty after dark with no warning, and
   * one on a spare layout would warn about a panel the watch never shows.
   */
  test('names the panels on the assigned layouts and leaves out the spare ones', () => {
    const library = { layouts: ['2,0,0,2,1;3,1,0,2,1', '75,0,0,2,1;2,1,0,2,1', '20,0,0,2,2', '0', '0'], day: 0, night: 1, quiet: ROLE_NONE };

    const result = assignedLabels(library, parse, (module) => NAMES[module]);

    expect(result).toEqual(['Battery', 'Weather', 'Phone Battery']);
  });
});
