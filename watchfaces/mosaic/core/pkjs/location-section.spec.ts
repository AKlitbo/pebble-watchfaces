/**
 * Specs for the Weather Location section and the alternate time zone row.
 *
 * The section folds away whole while the weather comes from a Pebble app plugin, and the framework
 * only folds a card when every row in it, heading included, carries the mark. The time zone row has
 * to stay a time zone search, since the watch reads an offset from it rather than a place.
 */

import { describe, test, expect } from 'vitest';
import { locationSection, timeZoneItem } from './location-section';

describe('locationSection', () => {
  /** One unmarked row keeps the card on screen once the plugin is picked, holding a heading over nothing. */
  test('marks every row for the weather source, heading included', () => {
    const result = (locationSection().items || []).filter((row) => row.pluginSourceHides !== 'WEATHER_PROVIDER');

    expect(result).toEqual([]);
  });
});

describe('timeZoneItem', () => {
  /** Without the time zone flag the search saves a place, and the watch wants an offset from this key. */
  test('is a time zone search on the CLOCK_TIMEZONE_1 key', () => {
    const result = timeZoneItem();

    expect(result).toMatchObject({ type: 'locationsearch', messageKey: 'CLOCK_TIMEZONE_1', timeZone: true });
  });
});
