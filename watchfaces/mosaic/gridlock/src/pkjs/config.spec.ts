/**
 * Specs for the config page's message keys and layout panels.
 *
 * A key the page sends but the appinfo never declared is the quietest failure in the whole
 * settings path: Clay maps an unknown key to the literal dict key "undefined0", the watch ignores
 * it, and the setting simply never works. Nothing warns, nothing throws, and it looks like a bug
 * in whatever feature the key belonged to. Comparing the two lists catches it here instead.
 *
 * The layout palette sorts panels by group, and a panel nobody grouped falls to Other at the end
 * where nobody looks for it. Checking every placeable panel against the group list catches a new
 * panel added without one.
 *
 * Picking the Pebble app as the weather or calendar source has to take every row that only feeds the
 * phone's own fetch off the page, and fold the Weather Location card away whole. The marks that do
 * it sit in the face and the family core, and one unmarked row added to the card leaves it on screen
 * doing nothing. Running the framework's page hook over the real page catches that here.
 *
 * The HRV Skip at Start row sends an index, and the watch looks the gap count up in its own table in
 * settings_schema.c. Both compile on their own, so a count added to one list and not the other would
 * have the watch skip a different number of gaps from the one the wearer picked. Reading the C table
 * beside the page row catches that.
 */

import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect } from 'vitest';
import config from './config';
import { pageKeys, layoutModules, sectionHolding } from '../../../core/pkjs/testing/page-keys';
import { PALETTE_GROUPS } from '../../../core/pkjs/clay/builder/ts/layout/palette';
import { pluginSourcePage } from '../../../paf/ts/plugin-sources/page';
import { CALENDAR_SOURCE_SELECT } from '../../../paf/ts/plugin-sources/calendar';
import { CALENDAR_STANDARD, WEATHER_STANDARD } from '../../../paf/ts/plugin-sources/standard';
import { allItems, conditionsOf } from '../../../paf/ts/clay/rows';
import type { ClayConfigItem } from '../../../paf/ts/clay/types';

const appinfo = JSON.parse(
  fs.readFileSync(path.resolve(import.meta.dirname, '../../pebble.appinfo.json'), 'utf8')
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

  /** The night layout keys in particular, since they span the page, the appinfo and the C schema. */
  test('the night layout keys are wired end to end', () => {
    const onPage = new Set(pageKeys(config));

    for (const key of ['LAYOUT', 'LAYOUT_NIGHT', 'LAYOUT_NIGHT_MODE', 'LAYOUT_NIGHT_START', 'LAYOUT_NIGHT_END']) {
      expect(onPage.has(key), `${key} missing from the config page`).toBe(true);
      expect(appinfo.messageKeys, `${key} missing from the appinfo`).toContain(key);
    }
  });
});

describe('layout palette groups', () => {
  /** A panel with no group drops into Other at the end of the palette, where nobody looks for it. */
  test('every placeable panel sits in one of the palette groups', () => {
    const known = new Set<string>(PALETTE_GROUPS.map((group) => group.name));

    const result = layoutModules(config)
      .filter((module) => module.value !== 0 && (module.sizes || []).length > 0 && !known.has(module.group || ''))
      .map((module) => module.label);

    expect(result).toEqual([]);
  });

  /** The filter above passes just as happily on an empty list, which would prove nothing. */
  test('the layout builder carries its panels', () => {
    const result = layoutModules(config);

    expect(result.length).toBeGreaterThan(20);
  });
});

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

/** A phone with the Pebble app's built-in weather and calendar plugins. */
const BUILT_IN_PLUGINS = [
  { uuid: '00000000-0000-0000-0001-000000000002', name: 'Weather', sources: [{ category: 'weather', items: ['location'], properties: {} }] },
  { uuid: '00000000-0000-0000-0001-000000000004', name: 'Calendar', sources: [{ category: 'calendar', items: ['event'], properties: {} }] },
];

/** The page as the phone opens it on a plugin build, with both page hooks run over a copy. */
function pluginPage(): ClayConfigItem[] {
  const page = JSON.parse(JSON.stringify(config)) as ClayConfigItem[];
  const messageKeys = Object.fromEntries(appinfo.messageKeys.map((key, index) => [key, index]));
  const state = { pluginUuid: null, lastError: null, lastEmpty: false };

  pluginSourcePage(page, {
    selectKey: 'WEATHER_PROVIDER',
    category: 'weather',
    item: 'location',
    plugins: BUILT_IN_PLUGINS,
    messageKeys,
    state,
    standard: WEATHER_STANDARD,
    texts: {},
    movesBelow: 'LOCATION_USE_GPS',
  });
  pluginSourcePage(page, {
    selectKey: 'CALENDAR_SOURCE',
    category: 'calendar',
    item: 'event',
    plugins: BUILT_IN_PLUGINS,
    messageKeys,
    state,
    standard: CALENDAR_STANDARD,
    texts: {},
    insert: { select: CALENDAR_SOURCE_SELECT, anchorKey: 'CALENDAR_ICS_URL' },
  });
  return page;
}

/**
 * The message keys of the rows hidden under the given picks, read the way the page's
 * dependentVisibility reads them. A control not picked holds its default.
 */
function hiddenKeys(page: ClayConfigItem[], picks: Record<string, unknown>): string[] {
  const rows = allItems(page);
  const values: Record<string, unknown> = {};

  rows.forEach((row) => {
    if (row.messageKey) {
      values[row.messageKey] = row.defaultValue;
    }
  });
  Object.assign(values, picks);

  return rows
    .filter((row) => row.messageKey && !conditionsOf(row).every((condition) => condition.in.some((wanted) => String(wanted) === String(values[condition.key]))))
    .map((row) => row.messageKey as string);
}

describe('config page with the Pebble app plugins', () => {
  /** A row left showing under the plugin does nothing, and reads as a setting the watch still uses. */
  test('picking Preferred Source hides the key, the location rows, and the iCal rows', () => {
    const page = pluginPage();

    const result = hiddenKeys(page, { WEATHER_PROVIDER: 'plugin', CALENDAR_SOURCE: 'plugin' });

    expect(result).toEqual(expect.arrayContaining(['WEATHER_API_KEY', 'LOCATION_USE_GPS', 'LOCATION_GPS_FALLBACK', 'LOCATION_NAME', 'CALENDAR_ICS_URL', 'CALENDAR_POLL']));
  });

  /** The clock reads the time zone whatever the weather source, so the plugin must never take it off the page. */
  test('picking Preferred Source keeps the alternate time zone', () => {
    const page = pluginPage();

    const result = hiddenKeys(page, { WEATHER_PROVIDER: 'plugin', CALENDAR_SOURCE: 'plugin' });

    expect(result).not.toContain('CLOCK_TIMEZONE_1');
  });

  /** Switching back has to bring every row the plugin hid, or an Open-Meteo wearer loses their city. */
  test('switching back to Open-Meteo and the iCal link shows the rows again', () => {
    const page = pluginPage();

    const result = hiddenKeys(page, { WEATHER_PROVIDER: 'openmeteo', CALENDAR_SOURCE: 'ical' });

    expect(result).not.toContain('LOCATION_USE_GPS');
    expect(result).not.toContain('LOCATION_NAME');
    expect(result).not.toContain('CALENDAR_ICS_URL');
    expect(result).not.toContain('CALENDAR_POLL');
  });

  /**
   * Clay draws a section as a card and only the hook's hideWhenEmpty folds one away. A heading or a
   * row in the card without the mark leaves an empty card behind once the plugin is picked.
   */
  test('the Weather Location card folds away whole', () => {
    const page = pluginPage();

    const result = sectionHolding(page, 'LOCATION_USE_GPS');

    expect(result?.hideWhenEmpty).toBe(true);
    expect((result?.items || []).every((row) => conditionsOf(row).some((condition) => condition.key === 'WEATHER_PROVIDER'))).toBe(true);
  });
});

describe('config page order', () => {
  /**
   * Every location row only feeds the weather, so a wearer picks the source before the place. On a
   * build without the plugin nothing moves the card, so the face's own order is the one they see.
   */
  test('the Weather card comes before the Weather Location card', () => {
    const page = config as ClayConfigItem[];
    const weather = sectionHolding(page, 'WEATHER_PROVIDER');
    const location = sectionHolding(page, 'LOCATION_USE_GPS');

    const result = page.indexOf(weather as ClayConfigItem);

    expect(weather).toBeDefined();
    expect(location).toBeDefined();
    expect(result).toBeGreaterThanOrEqual(0);
    expect(result).toBeLessThan(page.indexOf(location as ClayConfigItem));
  });

  /** The clock reads the time zone whatever the weather source, so the row belongs with the other clock settings. */
  test('the Clock card holds the Alternate Time Zone', () => {
    const page = config as ClayConfigItem[];

    const result = sectionHolding(page, 'CLOCK_TIMEZONE_1');

    expect(result).toBeDefined();
    expect(result).toBe(sectionHolding(page, 'CLOCK_TIME_FORMAT'));
  });
});

describe('config page weather key', () => {
  /** Open-Meteo needs no key, so a key row showing for it asks the wearer for something that does nothing. */
  test.each([
    ['openmeteo', true],
    ['owm', false],
    ['weatherapi', false],
  ])('the API Key row for %s is hidden: %s', (provider, hidden) => {
    const result = hiddenKeys(config as ClayConfigItem[], { WEATHER_PROVIDER: provider }).includes('WEATHER_API_KEY');

    expect(result).toBe(hidden);
  });
});

describe('config page HRV settle choice', () => {
  const schema = fs.readFileSync(path.resolve(import.meta.dirname, '../c/settings_schema.c'), 'utf8');

  /** The gap counts the watch looks the saved index up in, read out of the C table. */
  function watchSettleGaps(): number[] {
    const table = /s_hrv_settle_gaps\[\]\s*=\s*\{([^}]*)\}/.exec(schema);

    expect(table).not.toBeNull();

    return (table as RegExpExecArray)[1].split(',').map((entry) => Number(entry.trim()));
  }

  /** The index the watch starts on before any save, read out of the field's FACE_ENUM. */
  function watchSettleDefault(): number {
    const field = /FACE_ENUM\(GridlockHealth,\s*hrv_settle,\s*HEALTH_HRV_SETTLE,[^,]*,\s*(\d+)\)/.exec(schema);

    expect(field).not.toBeNull();

    return Number((field as RegExpExecArray)[1]);
  }

  /** The settle row on the page. */
  function settleRow(): ClayConfigItem {
    const row = allItems(config as ClayConfigItem[]).find((item) => item.messageKey === 'HEALTH_HRV_SETTLE');

    expect(row).toBeDefined();

    return row as ClayConfigItem;
  }

  /** A page offering a count the watch table does not hold, or in another order, skips the wrong number of gaps. */
  test('each choice on the page names the count the watch keeps at its index', () => {
    const expected = watchSettleGaps().map((gaps, index) => [index, gaps]);

    const result = (settleRow().options || []).map((option) => [option.value, parseInt(option.label, 10)]);

    expect(result).toEqual(expected);
  });

  /** A page and a watch starting on different counts would send back a choice the wearer never made after a reset. */
  test('the page and the watch both start on 10 gaps', () => {
    const watchDefault = watchSettleDefault();

    const result = settleRow().defaultValue;

    expect(result).toBe(watchDefault);
    expect(watchSettleGaps()[watchDefault]).toBe(10);
  });
});
