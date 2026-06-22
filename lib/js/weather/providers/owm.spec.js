/**
 * Deterministic specs for the OpenWeatherMap provider.
 *
 * The provider takes resolved coordinates only, so this is a single weather
 * call. The injected `request` is stubbed and every call is recorded so the
 * URL and error mapping can be asserted.
 */

import { describe, test, expect } from 'vitest';
import owm from './owm.js';
import { fetchRequest } from '../../../../testing/fetch-request.js';

const WX = '/data/2.5/weather';

/**
 * Stub `request` that routes by URL substring and records calls in order.
 * 
 * @param {!Object<string, {err?: string, body?: string}>} byPath substring -> response
 * @param {!Array<string>} calls sink for requested URLs, in order
 * @return {Function}
 */
function routing(byPath, calls) {
  return (url, callback) => {
    calls.push(url);

    const key = Object.keys(byPath).find((path) => url.includes(path));
    if (!key) {
      throw new Error('unexpected request url: ' + url);
    }

    const response = byPath[key];
    callback(response.err || null, response.body);
  };
}

/** Runs the provider synchronously and returns the result object. */
function run(opts, request) {
  let result;
  owm.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { key: 'k', coords: { lat: 40, lon: -73 }, fahrenheit: false };
const WX_OK = JSON.stringify({ cod: 200, name: 'WeatherCity', main: { temp: 13.4 }, weather: [{ main: 'Rain' }] });

describe('owm provider', () => {
  describe('guard clauses', () => {
    /** A missing key must short-circuit before any network call. */
    test('reports a missing key without making a request', () => {
      const calls = [];

      const result = run({ ...BASE, key: '' }, routing({}, calls));

      expect(result.condition).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });

    /** With no coordinates there is nothing to look up. */
    test('reports No Location when coords are missing', () => {
      const calls = [];

      const result = run({ ...BASE, coords: null }, routing({}, calls));

      expect(result.condition).toBe('NO LOCATION');
      expect(calls).toHaveLength(0);
    });
  });

  describe('weather request', () => {
    /** The weather call must use the supplied coords - a wrong URL reads the wrong place. */
    test('queries weather at the supplied coordinates', () => {
      const calls = [];

      const result = run(BASE, routing({ [WX]: { body: WX_OK } }, calls));

      expect(calls).toHaveLength(1);
      expect(calls[0]).toContain(WX);
      expect(calls[0]).toMatch(/lat=40&lon=-73/);
      expect(result.ok).toBe(true);
    });

    /** A wrong units param shows Fahrenheit numbers labelled as Celsius (or vice-versa). */
    test.each([
      [false, 'units=metric'],
      [true, 'units=imperial']
    ])('selects units in the weather URL (fahrenheit=%s)', (fahrenheit, expected) => {
      const calls = [];

      run({ ...BASE, fahrenheit }, routing({ [WX]: { body: WX_OK } }, calls));

      expect(calls[0]).toContain(expected);
    });
  });

  describe('successful reads', () => {
    /** Temp and condition must come from the documented fields, rounded for the watch. */
    test('returns the rounded temp and primary condition', () => {
      const result = run(BASE, routing({ [WX]: { body: WX_OK } }, []));

      expect(result.temperature).toBe(13);
      expect(result.condition).toBe('RAIN');
    });

    /** A clear sky with a night icon code must report Clear Night so the watch shows a moon. */
    test('promotes a clear sky to Clear Night when the icon is a night code', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear', icon: '01n' }] });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR_NIGHT');
    });

    /** A clear sky with a day icon code must stay Clear so the watch shows the sun. */
    test('keeps a clear sky as Clear when the icon is a day code', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear', icon: '01d' }] });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** A clear sky with no icon must default to day and stay Clear, never showing a moon. */
    test('keeps a clear sky as Clear when the icon is absent', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear' }] });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** With no label, OWM's own city name (json.name) is used. */
    test('labels the reading with the response name when no label is given', () => {
      const result = run(BASE, routing({ [WX]: { body: WX_OK } }, []));

      expect(result.location).toBe('WeatherCity');
    });

    /** A supplied label (the geocoded place name) must win over the response name. */
    test('prefers the supplied label for the reading location', () => {
      const result = run({ ...BASE, label: 'Phoenix' }, routing({ [WX]: { body: WX_OK } }, []));

      expect(result.location).toBe('Phoenix');
    });
  });

  describe('weather-step errors', () => {
    /** A weather-call failure must not be silently swallowed. */
    test('maps a weather network error to Net Error', () => {
      const result = run(BASE, routing({ [WX]: { err: 'HTTP 500' } }, []));

      expect(result.condition).toBe('NET ERROR');
    });

    /** A 401 on the weather call must surface as Invalid Key. */
    test('maps a 401 from the weather call to Invalid Key', () => {
      const body = JSON.stringify({ cod: 401 });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('INVALID KEY');
    });

    /** Any non-200 code other than 401 is a generic API error. */
    test('maps a non-401 error code to API Error', () => {
      const body = JSON.stringify({ cod: 429, message: 'rate' });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('API ERROR');
    });

    /** A response missing main/weather must report No Wx Data, not crash on undefined. */
    test('maps a weather response missing fields to No Wx Data', () => {
      const body = JSON.stringify({ cod: 200 });

      const result = run(BASE, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('NO WX DATA');
    });
  });

  // live integration against the real OpenWeatherMap API, opt-in via RUN_LIVE=1
  // plus OWM_KEY - catches the upstream changing response shape
  describe.skipIf(process.env.RUN_LIVE !== '1' || !process.env.OWM_KEY)('live', () => {
    const KEY = process.env.OWM_KEY;
    const live = (opts) => new Promise((resolve) => owm.fetch(opts, fetchRequest, resolve));

    /** The real coords path must still parse into a usable reading. */
    test('returns a usable reading for coordinates', async () => {
      const result = await live({ key: KEY, coords: { lat: 51.5074, lon: -0.1278 }, fahrenheit: false });

      expect(result.ok).toBe(true);
      expect(typeof result.temperature).toBe('number');
      expect(result.condition).toBeTruthy();
    });
  });
});
