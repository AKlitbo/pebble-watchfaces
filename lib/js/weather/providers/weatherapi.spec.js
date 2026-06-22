/**
 * Deterministic specs for the WeatherAPI.com provider.
 *
 * The provider resolves a place name or "lat,lon" in a single call. The
 * injected `request` is stubbed and every call is recorded so the URL and
 * error mapping can be asserted.
 */

import { describe, test, expect } from 'vitest';
import weatherapi from './weatherapi.js';
import { fetchRequest } from '../../../../testing/fetch-request.js';

const WX = '/v1/current.json';

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
  weatherapi.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE_OPTS = { key: 'test-key', place: 'London', coords: null, fahrenheit: false };

const OK_BODY = JSON.stringify({
  location: { name: 'London' },
  current: { temp_c: 13.4, temp_f: 56.1, condition: { text: 'Light rain' } }
});

describe('weatherapi provider', () => {
  describe('guard clauses', () => {
    /** Without a key the fetch must report a config error, not silently hang. */
    test('reports a missing key without making a request', () => {
      const calls = [];

      const result = run({ ...BASE_OPTS, key: '' }, routing({}, calls));

      expect(result.condition).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });

    /** With neither coords nor a place there is nothing to query. */
    test('reports No Location when neither place nor coords is given', () => {
      const calls = [];

      const result = run({ ...BASE_OPTS, place: '', coords: null }, routing({}, calls));

      expect(result.condition).toBe('NO LOCATION');
      expect(calls).toHaveLength(0);
    });
  });

  describe('transport / parse failures', () => {
    /** A genuine transport failure (no response body) must surface as Net Error. */
    test('maps a request error to Net Error', () => {
      const result = run(BASE_OPTS, routing({ [WX]: { err: 'HTTP 401' } }, []));

      expect(result.ok).toBe(false);
      expect(result.condition).toBe('NET ERROR');
    });

    /** A malformed body must show a data error, not crash on undefined fields. */
    test('maps unparseable JSON to Bad Wx Data', () => {
      const result = run(BASE_OPTS, routing({ [WX]: { body: 'not json' } }, []));

      expect(result.condition).toBe('BAD WX DATA');
    });
  });

  describe('upstream api error codes', () => {
    /** Searching a non-existent place must tell the user, not show a stale reading. */
    test('maps error code 1006 to Loc Not Found', () => {
      const body = JSON.stringify({ error: { code: 1006, message: 'no match' } });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('LOC NOT FOUND');
    });

    /** Each documented key-failure code must map to one clear message. */
    test.each([1002, 2006, 2008])('maps key-error code %i to Invalid Key', (code) => {
      const body = JSON.stringify({ error: { code, message: 'bad key' } });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('INVALID KEY');
    });

    /** An unrecognised error code must still fail safe with a generic message. */
    test('maps an unknown error code to API Error', () => {
      const body = JSON.stringify({ error: { code: 9999, message: 'boom' } });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('API ERROR');
    });

    /** A 200 response missing the current block must not crash reading condition.text. */
    test('maps a response missing current to No Wx Data', () => {
      const body = JSON.stringify({ location: { name: 'London' } });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('NO WX DATA');
    });
  });

  describe('successful reads', () => {
    /** Temperature, condition and label must come from the documented fields. */
    test('returns a rounded celsius reading with an aliased condition by default', () => {
      const result = run(BASE_OPTS, routing({ [WX]: { body: OK_BODY } }, []));

      expect(result).toEqual({ temperature: 13, condition: 'RAIN', location: 'London', ok: true });
    });

    /** A clear sky at night (is_day=0) must report Clear Night so the watch shows a moon. */
    test('promotes a clear sky to Clear Night when is_day is 0', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, is_day: 0, condition: { text: 'Clear' } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR_NIGHT');
    });

    /** A clear sky by day (is_day=1) must stay Clear so the watch shows the sun. */
    test('keeps a clear sky as Clear when is_day is 1', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, is_day: 1, condition: { text: 'Clear' } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** A clear sky with no is_day field must default to day and stay Clear, never showing a moon. */
    test('keeps a clear sky as Clear when is_day is absent', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'Clear' } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** WeatherAPI says "Sunny" for daytime clear - it must alias to Clear so the watch shows the sun, not the NA icon. */
    test('aliases daytime "Sunny" to Clear', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 20, temp_f: 68, is_day: 1, condition: { text: 'Sunny' } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** Picking the wrong unit field shows Fahrenheit numbers labelled as Celsius. */
    test('reads temp_f when fahrenheit is requested', () => {
      const result = run({ ...BASE_OPTS, fahrenheit: true }, routing({ [WX]: { body: OK_BODY } }, []));

      expect(result.temperature).toBe(56);
    });

    /** The place name must be sent as the q parameter or the wrong location is fetched. */
    test('sends the place name as the q parameter', () => {
      const calls = [];

      run(BASE_OPTS, routing({ [WX]: { body: OK_BODY } }, calls));

      expect(calls[0]).toMatch(/[?&]q=London(?:&|$)/);
    });

    /** Coords must be sent as an encoded "lat,lon" q value, not the empty place name. */
    test('sends "lat,lon" as the q parameter when coords are present', () => {
      const calls = [];

      run({ ...BASE_OPTS, place: '', coords: { lat: 51.5, lon: -0.12 } }, routing({ [WX]: { body: OK_BODY } }, calls));

      expect(calls[0]).toMatch(/[?&]q=51\.5%2C-0\.12(?:&|$)/);
    });
  });

  // live integration against the real weatherapi.com API, opt-in via RUN_LIVE=1
  // plus WEATHERAPI_KEY - catches the upstream changing response shape
  describe.skipIf(process.env.RUN_LIVE !== '1' || !process.env.WEATHERAPI_KEY)('live', () => {
    const KEY = process.env.WEATHERAPI_KEY;
    const live = (opts) => new Promise((resolve) => weatherapi.fetch(opts, fetchRequest, resolve));

    /** The real place-name response must still parse into a usable reading. */
    test('returns a usable reading for a place name', async () => {
      const result = await live({ key: KEY, place: 'London', coords: null, fahrenheit: false });

      expect(result.ok).toBe(true);
      expect(typeof result.temperature).toBe('number');
      expect(result.condition).toBeTruthy();
    });

    /** The real coords response must still parse into a usable reading. */
    test('returns a usable reading for gps coords', async () => {
      const result = await live({ key: KEY, place: '', coords: { lat: 51.5074, lon: -0.1278 }, fahrenheit: false });

      expect(result.ok).toBe(true);
      expect(typeof result.temperature).toBe('number');
    });
  });
});
