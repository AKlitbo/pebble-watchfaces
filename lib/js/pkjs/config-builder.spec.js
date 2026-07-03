/**
 * Specs for the shared Clay config builder.
 *
 * The builder turns a per-section description into the Clay config array each
 * face ships. The messageKeys it emits are the wire contract with the C settings
 * table and package.json, and whether an optional section appears is driven by
 * the presence of its key. Both are easy to break silently in a refactor.
 */

import { describe, test, expect } from 'vitest';
import buildConfig from './config-builder.js';

/** Collects every messageKey the config publishes, across top-level items and sections. */
function collectMessageKeys(config) {
  const keys = [];

  for (const entry of config) {
    if (entry.messageKey) {
      keys.push(entry.messageKey);
    }
    for (const item of entry.items || []) {
      if (item.messageKey) {
        keys.push(item.messageKey);
      }
    }
  }

  return keys;
}

/** Finds a config item by its messageKey, searching inside sections. */
function findItemByKey(config, messageKey) {
  for (const entry of config) {
    if (entry.messageKey === messageKey) {
      return entry;
    }
    for (const item of entry.items || []) {
      if (item.messageKey === messageKey) {
        return item;
      }
    }
  }

  return undefined;
}

const minimalTheme = { options: [{ label: 'A', value: 0 }] };

describe('buildConfig wire contract', () => {
  /** A renamed or dropped core key silently breaks the settings round-trip with the watch. */
  test('always publishes the core settings keys', () => {
    const config = buildConfig({ theme: minimalTheme });

    const keys = collectMessageKeys(config);

    expect(keys).toEqual(expect.arrayContaining([
      'THEME', 'BLUETOOTH_ICON', 'BLUETOOTH_VIBE_CONNECT', 'BLUETOOTH_VIBE_DISCONNECT',
      'DATE_FORMAT', 'TIME_FORMAT', 'STEPS_MODE'
    ]));
  });
});

describe('buildConfig optional sections', () => {
  /** A face that opts out of weather/location/temperature must not publish those keys,
   *  or the phone offers settings the watch never reads. */
  test('omits the optional keys when their sections are absent', () => {
    const config = buildConfig({ theme: minimalTheme });

    const keys = collectMessageKeys(config);

    expect(keys).not.toContain('USE_GPS');
    expect(keys).not.toContain('GPS_FALLBACK');
    expect(keys).not.toContain('LOCATION_NAME');
    expect(keys).not.toContain('WEATHER_PROVIDER');
    expect(keys).not.toContain('API_KEY');
    expect(keys).not.toContain('TEMPERATURE_UNIT');
  });

  /** If presence inclusion breaks, a full-featured face silently loses its weather,
   *  location, or temperature settings. */
  test('includes the optional keys when their sections are present', () => {
    const config = buildConfig({
      theme: minimalTheme,
      location: {},
      weather: {},
      temperature: {}
    });

    const keys = collectMessageKeys(config);

    expect(keys).toEqual(expect.arrayContaining([
      'USE_GPS', 'GPS_FALLBACK', 'LOCATION_NAME', 'WEATHER_PROVIDER', 'API_KEY', 'TEMPERATURE_UNIT'
    ]));
  });
});

describe('buildConfig per-section values', () => {
  /** If the builder dropped the face's theme list, the user could not pick their watch's themes. */
  test('passes the face theme options through to the THEME select', () => {
    const themeOptions = [
      { label: 'Classic', value: 0 },
      { label: 'Nemesis Blue', value: 3 }
    ];

    const config = buildConfig({ theme: { options: themeOptions } });
    const themeSelect = findItemByKey(config, 'THEME');

    expect(themeSelect.options).toEqual(themeOptions);
  });

  /** A wrong default date format shows the user the wrong date layout on first install. */
  test('applies the face date default to the DATE_FORMAT select', () => {
    const config = buildConfig({ theme: minimalTheme, date: { default: '%a %d %b' } });

    const dateSelect = findItemByKey(config, 'DATE_FORMAT');

    expect(dateSelect.defaultValue).toBe('%a %d %b');
  });

  /** A face that wants GPS on by default would silently ship with it off if the default leaked. */
  test('defaults GPS off when the location section omits gpsDefault', () => {
    const config = buildConfig({ theme: minimalTheme, location: {} });

    const gpsToggle = findItemByKey(config, 'USE_GPS');

    expect(gpsToggle.defaultValue).toBe(false);
  });

  /** A face that opts GPS on must carry that through, or first-run users get no automatic weather. */
  test('honors gpsDefault when the location section sets it', () => {
    const config = buildConfig({ theme: minimalTheme, location: { gpsDefault: true } });

    const gpsToggle = findItemByKey(config, 'USE_GPS');

    expect(gpsToggle.defaultValue).toBe(true);
  });
});
