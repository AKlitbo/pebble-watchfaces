/**
 * Deterministic specs for the Open-Meteo provider.
 *
 * The provider takes resolved coordinates only, so this is a single forecast
 * call whose parsing depends on util.wmoToCondition. The injected `request` is
 * stubbed and calls are recorded so the URL and sequencing can be asserted.
 */

import { describe, test, expect } from 'vitest';
import openmeteo from './openmeteo.js';
import { fetchRequest } from '../../../../testing/fetch-request.js';

const FC = '/v1/forecast';

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
  openmeteo.fetch(opts, request, (received) => { result = received; });
  return result;
}

const COORDS = { key: '', coords: { lat: 40, lon: -73 }, fahrenheit: false };
const FC_OK = JSON.stringify({ current: { temperature_2m: 13.4, weather_code: 61 } });

describe('openmeteo provider', () => {
  describe('guard clauses', () => {
    /** With no coordinates there is nothing to look up. */
    test('reports No Location when coords are missing', () => {
      const calls = [];

      const result = run({ key: '', coords: null, fahrenheit: false }, routing({}, calls));

      expect(result.condition).toBe('NO LOCATION');
      expect(calls).toHaveLength(0);
    });
  });

  describe('forecast request', () => {
    /** The forecast must query the supplied coords - a wrong URL reads the wrong place. */
    test('queries the forecast at the supplied coordinates', () => {
      const calls = [];

      run(COORDS, routing({ [FC]: { body: FC_OK } }, calls));

      expect(calls).toHaveLength(1);
      expect(calls[0]).toContain(FC);
      expect(calls[0]).toMatch(/latitude=40&longitude=-73/);
    });

    /** The forecast request must ask for is_day, else night can never be detected. */
    test('requests is_day in the forecast URL', () => {
      const calls = [];

      run(COORDS, routing({ [FC]: { body: FC_OK } }, calls));

      expect(calls[0]).toContain('is_day');
    });

    /** A wrong unit param shows Fahrenheit numbers labelled as Celsius (or vice-versa). */
    test.each([
      [false, 'temperature_unit=celsius'],
      [true, 'temperature_unit=fahrenheit']
    ])('selects the temperature unit (fahrenheit=%s)', (fahrenheit, expected) => {
      const calls = [];

      run({ ...COORDS, fahrenheit }, routing({ [FC]: { body: FC_OK } }, calls));

      expect(calls[0]).toContain(expected);
    });
  });

  describe('successful reads', () => {
    /** The temperature must be read from the documented field and rounded. */
    test('returns the rounded forecast temperature', () => {
      const result = run(COORDS, routing({ [FC]: { body: FC_OK } }, []));

      expect(result.temperature).toBe(13);
    });

    /** Proves the provider reads `weather_code` and routes it through wmoToCondition.
     * The full code->word table is exercised exhaustively in util.spec.js. */
    test('derives the condition from the forecast weather_code', () => {
      const body = JSON.stringify({ current: { temperature_2m: 5, weather_code: 61 } });

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('RAIN');
    });

    /** A clear sky at night (is_day=0) must report Clear Night so the watch shows a moon. */
    test('promotes a clear sky to Clear Night when is_day is 0', () => {
      const body = JSON.stringify({ current: { temperature_2m: 5, weather_code: 0, is_day: 0 } });

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('CLEAR_NIGHT');
    });

    /** A clear sky by day (is_day=1) must stay Clear so the watch shows the sun. */
    test('keeps a clear sky as Clear when is_day is 1', () => {
      const body = JSON.stringify({ current: { temperature_2m: 5, weather_code: 0, is_day: 1 } });

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** A clear sky with no is_day field must default to day and stay Clear, never showing a moon. */
    test('keeps a clear sky as Clear when is_day is absent', () => {
      const body = JSON.stringify({ current: { temperature_2m: 5, weather_code: 0 } });

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** A coords reading with no label falls back to "My Location". */
    test('labels a reading My Location when no label is given', () => {
      const result = run(COORDS, routing({ [FC]: { body: FC_OK } }, []));

      expect(result.location).toBe('My Location');
    });

    /** A label (the geocoded place name) must be carried through to the watch. */
    test('uses the supplied label for the reading location', () => {
      const result = run({ ...COORDS, label: 'Phoenix' }, routing({ [FC]: { body: FC_OK } }, []));

      expect(result.location).toBe('Phoenix');
    });

    /** The resolved coordinates must ride along so the watch can show lat/lon. */
    test('carries the coordinates into the result', () => {
      const result = run(COORDS, routing({ [FC]: { body: FC_OK } }, []));

      expect(result.lat).toBe(40);
      expect(result.lon).toBe(-73);
    });
  });

  describe('forecast-step errors', () => {
    /** A forecast network failure must surface, not be mistaken for a reading. */
    test('maps a forecast network error to Net Error', () => {
      const result = run(COORDS, routing({ [FC]: { err: 'x' } }, []));

      expect(result.condition).toBe('NET ERROR');
    });

    /** A malformed forecast body must show a data error, not crash on undefined. */
    test('maps unparseable forecast data to Bad Wx Data', () => {
      const result = run(COORDS, routing({ [FC]: { body: 'nope' } }, []));

      expect(result.condition).toBe('BAD WX DATA');
    });

    /** Open-Meteo signals failures with an error object - must read as API Error. */
    test('maps a forecast API error object to API Error', () => {
      const body = JSON.stringify({ error: true, reason: 'bad' });

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('API ERROR');
    });

    /** A forecast missing the current block must report No Wx Data, not crash. */
    test('maps a forecast missing current to No Wx Data', () => {
      const body = JSON.stringify({});

      const result = run(COORDS, routing({ [FC]: { body } }, []));

      expect(result.condition).toBe('NO WX DATA');
    });
  });

  // live integration against the real Open-Meteo API, opt-in via RUN_LIVE=1
  // catches the upstream changing response shape
  describe.skipIf(process.env.RUN_LIVE !== '1')('live', () => {
    const live = (opts) => new Promise((resolve) => openmeteo.fetch(opts, fetchRequest, resolve));

    /** The real coords path must still parse into a usable reading. */
    test('returns a usable reading for coordinates', async () => {
      const result = await live({ key: '', coords: { lat: 51.5074, lon: -0.1278 }, fahrenheit: false });

      expect(result.ok).toBe(true);
      expect(typeof result.temperature).toBe('number');
      expect(result.condition).toBeTruthy();
    });
  });
});
