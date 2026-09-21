/**
 * Specs for the config page's message keys and date formats.
 *
 * A key the page sends but the appinfo never declared is the quietest failure in the whole
 * settings path: Clay maps an unknown key to the literal dict key "undefined0", the watch ignores
 * it, and the setting simply never works. Nothing warns, nothing throws, and it looks like a bug
 * in whatever feature the key belonged to. Comparing the two lists catches it here instead.
 *
 * This face draws its date with plain strftime, so it has no way to fill in the .beats token the
 * shared date list can carry. A {B} format reaching this page would render on the watch as the
 * literal braces, which reads as a corrupted date rather than an option that does not apply.
 */

import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect } from 'vitest';
import config from './config';
import { pageKeys } from '../../../core/pkjs/testing/page-keys';

const appinfo = JSON.parse(
  fs.readFileSync(path.resolve(import.meta.dirname, '../../config/pebble.appinfo.json'), 'utf8')
) as { messageKeys: string[] };

/** The values the date format select offers, wherever on the page it sits. */
function dateFormatValues(items: unknown): string[] {
  const found: string[] = [];

  function walk(node: unknown): void {
    if (Array.isArray(node)) {
      node.forEach(walk);
      return;
    }
    if (!node || typeof node !== 'object') {
      return;
    }
    const item = node as { messageKey?: unknown; options?: unknown; items?: unknown };
    if (item.messageKey === 'CLOCK_DATE_FORMAT' && Array.isArray(item.options)) {
      for (const option of item.options as Array<{ value?: unknown }>) {
        found.push(String(option.value));
      }
    }
    if (item.items) {
      walk(item.items);
    }
  }

  walk(items);
  return found;
}

describe('config page date formats', () => {
  /** This face draws its date with plain strftime, so a {B} token would reach the watch as text. */
  test('offers no date format carrying the .beats token', () => {
    const result = dateFormatValues(config).filter((value) => value.includes('{B}'));

    expect(result).toEqual([]);
  });

  /** The filter above passes just as happily on an empty list, which would prove nothing. */
  test('offers date formats at all', () => {
    const result = dateFormatValues(config);

    expect(result.length).toBeGreaterThan(10);
  });
});

describe('config page message keys', () => {
  /** The page has to actually carry some, or the walk above is silently finding nothing. */
  test('the page declares message keys', () => {
    const result = pageKeys(config);

    expect(result.length).toBeGreaterThan(20);
  });

  /** Every key the page sends must be one the watch was built to receive. */
  test('every key on the page is declared in the appinfo', () => {
    const declared = new Set(appinfo.messageKeys);

    const result = pageKeys(config).filter((key) => !declared.has(key));

    expect(result).toEqual([]);
  });

  /**
   * The rows the family builds from one shared source. A count alone would pass just as happily
   * with a whole section gone, and a missing section is a settings page quietly short of Health,
   * Location or Weather.
   */
  test('the shared family sections are on the page', () => {
    const onPage = new Set(pageKeys(config));

    for (const key of ['HEALTH_GOAL_STEPS', 'HEALTH_GOAL_VIBE_CUSTOM', 'LOCATION_USE_GPS', 'CLOCK_TIMEZONE_1', 'WEATHER_WIND_UNIT', 'WEATHER_PROVIDER']) {
      expect(onPage.has(key), `${key} missing from the config page`).toBe(true);
    }
  });

  /** The night and Quiet Time layout keys, since they span the page, the appinfo and the C schema. */
  test('the night layout keys are wired end to end', () => {
    const onPage = new Set(pageKeys(config));

    for (const key of ['LAYOUT', 'LAYOUT_NIGHT', 'LAYOUT_QUIET', 'LAYOUT_NIGHT_MODE', 'LAYOUT_NIGHT_START', 'LAYOUT_NIGHT_END']) {
      expect(onPage.has(key), `${key} missing from the config page`).toBe(true);
      expect(appinfo.messageKeys, `${key} missing from the appinfo`).toContain(key);
    }
  });
});
