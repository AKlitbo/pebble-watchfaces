/**
 * Specs for the panels the layout builder hands the framework's plugin warnings.
 *
 * The warning is the wearer's one chance to hear a panel will sit empty before saving. The panels it
 * weighs are the ones on every layout a role is assigned to, and no others.
 */

import { describe, test, expect } from 'vitest';
import { assignedLabels } from './plugin-panels';
import { ROLE_NONE } from './modes';

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
