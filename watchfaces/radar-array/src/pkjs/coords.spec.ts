/**
 * Specs for the coordinates Radar Array sends the watch.
 *
 * The watch's location store saves a fix for the next launch only when both keys hold a number. A
 * longitude key left empty means nothing is ever saved, and the coordinates row shows dashes on
 * every launch until the weather arrives. A "NO LOCK" reply must stay unsaved, so it never
 * replaces a good fix.
 */
import { describe, test, expect } from 'vitest';
import type { WeatherResult } from '../../paf/ts/weather/util';
import { coordFields } from './coords';

const KEYS = { LOCATION_LATITUDE: 10, LOCATION_LONGITUDE: 11 };

function reply(lat?: number, lon?: number): WeatherResult {
  return { temperature: 20, condition: 'clear', location: 'Los Angeles', ok: true, lat, lon };
}

describe('coordFields', () => {
  /** The latitude key is what the watch prints, so the whole readout has to be in it. */
  test('puts the whole readout in the latitude key', () => {
    const result = coordFields(KEYS, reply(34.0522, -118.2437));

    expect(result[10]).toBe('34.05N 118.24W');
  });

  /** An empty longitude key means the watch never saves the fix, and every launch shows dashes. */
  test('puts the longitude alone in the longitude key', () => {
    const result = coordFields(KEYS, reply(34.0522, -118.2437));

    expect(result[11]).toBe('118.24W');
  });

  /** A reply missing either half carrying a number would be saved over the last good location. */
  test.each([
    ['no coordinates', undefined, undefined],
    ['a latitude only', 34.0522, undefined],
    ['a longitude only', undefined, -118.2437],
    ['a latitude that is not a number', Number.NaN, -118.2437],
  ])('sends no number in either key for %s', (_name, lat, lon) => {
    const result = coordFields(KEYS, reply(lat, lon));

    expect(result).toEqual({ 10: 'NO LOCK', 11: '' });
  });
});
