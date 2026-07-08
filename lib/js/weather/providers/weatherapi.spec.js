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

const WX = '/v1/forecast.json';

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
  current: { temp_c: 13.4, temp_f: 56.1, condition: { text: 'Light rain' } },
  forecast: { forecastday: [{ astro: { sunrise: '06:00 AM', sunset: '08:00 PM' } }] }
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

      expect(result).toEqual({ temperature: 13, condition: 'RAIN', location: 'London', ok: true, sunrise: '06:00', sunset: '20:00' });
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

    /** Humidity, wind, uv, and precip come straight from current.json (wind already km/h, dir a compass word). */
    test('parses humidity, wind, uv, and precip from current.json', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'Clear' }, humidity: 61, wind_kph: 12, wind_dir: 'NW', uv: 5, precip_mm: 2.5 }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.humidity).toBe(61);
      expect(result.windKmh).toBe(12);
      expect(result.windDir).toBe('NW');
      expect(result.uvIndex).toBe(5);
      expect(result.precip).toBe(250);
    });

    /** The Tier-1 extras come straight from current.json, already in display units. */
    test('parses feels-like, pressure, cloud and gust from current.json', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: {
          temp_c: 10, temp_f: 50, condition: { text: 'Clear' },
          feelslike_c: 8.6, feelslike_f: 47.5, pressure_mb: 1011, cloud: 25, gust_kph: 33
        }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.feelsLike).toBe(9); // celsius by default
      expect(result.pressure).toBe(1011);
      expect(result.cloud).toBe(25);
      expect(result.windGustKmh).toBe(33);
    });

    /** Feels-like must follow the unit toggle like the main temperature, not stay stuck on celsius. */
    test('reads feelslike_f when fahrenheit is requested', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'Clear' }, feelslike_c: 8.6, feelslike_f: 47.5 }
      });

      const result = run({ ...BASE_OPTS, fahrenheit: true }, routing({ [WX]: { body } }, []));

      expect(result.feelsLike).toBe(48);
    });

    /** Dew point is current; the daily high/low, rain chance, precip total and peak UV come from the day block. */
    test('parses dew point and the daily high/low, precip chance/total and uv max', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 13, temp_f: 55, condition: { text: 'Light rain' }, dewpoint_c: -2.4, dewpoint_f: 27.7 },
        forecast: { forecastday: [{ day: {
          maxtemp_c: 18.6, maxtemp_f: 65.5, mintemp_c: 9.2, mintemp_f: 48.6,
          daily_chance_of_rain: 80, totalprecip_mm: 4.25, uv: 7.6
        } }] }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.dewPoint).toBe(-2);
      expect(result.tempMax).toBe(19);
      expect(result.tempMin).toBe(9);
      expect(result.precipChance).toBe(80);
      expect(result.precipTotal).toBe(425);
      expect(result.uvMax).toBe(8);
    });

    /** Astronomy data is extracted from the forecast array. */
    test('parses sunrise and sunset from astro block', () => {
      const result = run(BASE_OPTS, routing({ [WX]: { body: OK_BODY } }, []));

      expect(result.sunrise).toBe('06:00');
      expect(result.sunset).toBe('20:00');
    });

    /** Missing astronomy data safely leaves sunrise and sunset unset. */
    test('omits sunrise and sunset when astro data is missing', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'Clear' }, humidity: 61, wind_kph: 12, wind_dir: 'NW' }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result).not.toHaveProperty('sunrise');
      expect(result).not.toHaveProperty('sunset');
    });

    /** WeatherAPI says "Sunny" for daytime clear. It must alias to Clear so the watch shows the sun, not the NA icon. */
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

  describe('numeric condition codes', () => {
    /** WeatherAPI's text is localized but condition.code is stable. The code must win
     *  so a non-English "partly cloudy" still maps to PCLDY instead of UNKNOWN. */
    test('maps condition.code to a token, preferring it over the localized text', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'algo distinto', code: 1003 } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('PCLDY');
    });

    /** Both "cloudy" (1006) and "overcast" (1009) must resolve to CLDY. */
    test.each([[1006], [1009]])('maps cloud code %i to CLDY', (code) => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'x', code } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('CLDY');
    });

    /** An unmapped code must fall back to text aliasing, not render UNKNOWN. */
    test('falls back to text aliasing for an unmapped code', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, condition: { text: 'Light rain', code: 4321 } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('RAIN');
    });

    /** Night promotion must work off the resolved code, so a partly-cloudy night shows its moon glyph. */
    test('promotes a coded condition to its night form when is_day is 0', () => {
      const body = JSON.stringify({
        location: { name: 'London' },
        current: { temp_c: 10, temp_f: 50, is_day: 0, condition: { text: 'x', code: 1003 } }
      });

      const result = run(BASE_OPTS, routing({ [WX]: { body } }, []));

      expect(result.condition).toBe('PCLDY_NIGHT');
    });
  });

  // live integration against the real weatherapi.com API. opt-in via RUN_LIVE_WEATHER=1
  // plus WEATHERAPI_KEY. catches the upstream changing response shape
  describe.skipIf(process.env.RUN_LIVE_WEATHER !== '1' || !process.env.WEATHERAPI_KEY)('live', () => {
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
