/**
 * Deterministic specs for the OpenWeatherMap provider.
 *
 * The provider takes resolved coordinates only, so this is a single weather
 * call. The injected `request` is stubbed and every call is recorded so the
 * URL and error mapping can be asserted.
 */

import { describe, test, expect } from 'vitest';
import owm from './owm';
import { fetchRequest } from '../../../testing/fetch-request';
import type { RequestFn, WeatherOpts, WeatherResult } from '../util';

const WX = '/data/2.5/weather';
const OM = 'open-meteo.com/v1/forecast';

/**
 * Stub `request` that routes by URL substring and records calls in order.
 *
 * @param {!Object<string, {err?: string, body?: string}>} byPath substring -> response
 * @param {!Array<string>} calls sink for requested URLs, in order
 * @return {Function}
 */
function routing(byPath: Record<string, { err?: string | null; body?: string }>, calls: string[]): RequestFn {
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
function run(opts: WeatherOpts, request: RequestFn): WeatherResult {
  let result!: WeatherResult;
  owm.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { key: 'k', coords: { lat: 40, lon: -73 }, fahrenheit: false };
const WX_OK = JSON.stringify({ cod: 200, name: 'WeatherCity', main: { temp: 13.4 }, weather: [{ main: 'Rain' }] });

describe('owm provider', () => {
  describe('guard clauses', () => {
    /** A missing key must short-circuit before any network call. */
    test('reports a missing key without making a request', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, key: '' }, routing({}, calls));

      expect(result.condition).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });

    /** With no coordinates there is nothing to look up. */
    test('reports No Location when coords are missing', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, coords: null }, routing({}, calls));

      expect(result.condition).toBe('NO LOCATION');
      expect(calls).toHaveLength(0);
    });
  });

  describe('weather request', () => {
    /** The weather call must use the supplied coords. A wrong URL reads the wrong place. */
    test('queries weather at the supplied coordinates', () => {
      const calls: string[] = [];

      const result = run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      expect(calls).toHaveLength(2);
      expect(calls[0]).toContain(WX);
      expect(calls[0]).toMatch(/lat=40&lon=-73/);
      expect(result.ok).toBe(true);
    });

    /** A wrong units param shows Fahrenheit numbers labelled as Celsius (or vice-versa). */
    test.each([
      [false, 'units=metric'],
      [true, 'units=imperial'],
    ])('selects units in the weather URL (fahrenheit=%s)', (fahrenheit, expected) => {
      const calls: string[] = [];

      run({ ...BASE, fahrenheit }, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      expect(calls[0]).toContain(expected);
    });
  });

  describe('request order', () => {
    /** The old parallel join could strand done(), so OWM must lead and Open-Meteo follow it. */
    test('requests OWM before the Open-Meteo extras', () => {
      const calls: string[] = [];

      run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      expect(calls[0]).toContain(WX);
      expect(calls[1]).toContain(OM);
    });

    /** A failed OWM call has nothing to enrich, so the Open-Meteo extras must not be fetched. */
    test('skips the Open-Meteo call when the OWM request fails', () => {
      const calls: string[] = [];

      run(BASE, routing({ [WX]: { err: 'HTTP 500' } }, calls));

      expect(calls).toHaveLength(1);
      expect(calls[0]).toContain(WX);
    });
  });

  describe('successful reads', () => {
    /** Temp and condition must come from the documented fields, rounded for the watch. */
    test('returns the rounded temp and primary condition', () => {
      const result = run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, []));

      expect(result.temperature).toBe(13);
      expect(result.condition).toBe('RAIN');
    });

    /** A clear sky with a night icon code must report Clear Night so the watch shows a moon. */
    test('promotes a clear sky to Clear Night when the icon is a night code', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear', icon: '01n' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('CLEAR_NIGHT');
    });

    /** A clear sky with a day icon code must stay Clear so the watch shows the sun. */
    test('keeps a clear sky as Clear when the icon is a day code', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear', icon: '01d' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** A clear sky with no icon must default to day and stay Clear, never showing a moon. */
    test('keeps a clear sky as Clear when the icon is absent', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clear' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('CLEAR');
    });

    /** OWM lumps every cloud level under main "Clouds". Icon code 02 is "few clouds"
     *  and must split to PCLDY so partly-cloudy gets its own glyph, not full overcast. */
    test('splits OWM "Clouds" with icon code 02 into PCLDY', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clouds', icon: '02d' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('PCLDY');
    });

    /** Heavier cloud icon codes (03/04) must resolve to overcast CLDY, never PCLDY. */
    test('splits OWM "Clouds" with icon code 04 into CLDY', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Clouds', icon: '04d' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('CLDY');
    });

    /** Every condition ships a night glyph, so a rainy night (icon ending 'n')
     *  must promote to RAIN_NIGHT rather than keep the daytime rain icon. */
    test('promotes a non-clear condition to its night form after dark', () => {
      const body = JSON.stringify({ cod: 200, main: { temp: 10 }, weather: [{ main: 'Rain', icon: '10n' }] });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('RAIN_NIGHT');
    });

    /** With no label, OWM's own city name (json.name) is used. */
    test('labels the reading with the response name when no label is given', () => {
      const result = run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, []));

      expect(result.location).toBe('WeatherCity');
    });

    /** A supplied label (the geocoded place name) must win over the response name. */
    test('prefers the supplied label for the reading location', () => {
      const result = run({ ...BASE, label: 'Phoenix' }, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, []));

      expect(result.location).toBe('Phoenix');
    });

    /** Metric wind (m/s) must be normalized to km/h so the watch shows the right speed. */
    test('parses extras and converts metric wind from m/s to km/h', () => {
      const body = JSON.stringify({
        cod: 200, main: { temp: 10, humidity: 61 },
        weather: [{ main: 'Clear', icon: '01d' }],
        wind: { speed: 10, deg: 315 }, sys: { sunrise: 0, sunset: 24 * 3600 - 1 }, timezone: 0,
        rain: { '1h': 2.5 },
      });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.humidity).toBe(61);
      expect(result.windKmh).toBe(36); // 10 m/s * 3.6
      expect(result.windDir).toBe('NW');
      expect(result.sunrise).toBe('00:00');
      expect(result.precip).toBe(250);
    });

    /** The hybrid fetch must inject Open-Meteo's UV index into the payload. */
    test('injects UV index from the parallel Open-Meteo request', () => {
      const omBody = JSON.stringify({ current: { uv_index: 4.8 } });

      const result = run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: omBody } }, []));

      expect(result.uvIndex).toBe(5);
    });

    /** OWM's free endpoint lacks dew point and any daily forecast, so they must be injected from Open-Meteo. */
    test('injects dew point and the daily high/low, precip chance/total and uv max from Open-Meteo', () => {
      const omBody = JSON.stringify({
        current: { uv_index: 4.8, dew_point_2m: -2.4 },
        daily: {
          temperature_2m_max: [18.6], temperature_2m_min: [9.2],
          precipitation_probability_max: [80], precipitation_sum: [4.25], uv_index_max: [7.6],
        },
      });

      const result = run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: omBody } }, []));

      expect(result.dewPoint).toBe(-2);
      expect(result.tempMax).toBe(19);
      expect(result.tempMin).toBe(9);
      expect(result.precipChance).toBe(80);
      expect(result.precipTotal).toBe(425);
      expect(result.uvMax).toBe(8);
    });

    /** The injected dew point and daily highs must follow the unit toggle, so the hybrid call must ask for it. */
    test('requests the Open-Meteo daily fields in the chosen temperature unit', () => {
      const calls: string[] = [];

      run({ ...BASE, fahrenheit: true }, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      const omCall = calls.find((url) => url.includes(OM));
      expect(omCall).toContain('dew_point_2m');
      expect(omCall).toContain('temperature_2m_max');
      expect(omCall).toContain('temperature_unit=fahrenheit');
    });

    /** The Tier-1 extras come from their documented fields. Gust normalizes from m/s like wind speed. */
    test('parses feels-like, pressure, cloud and converts gust from m/s', () => {
      const body = JSON.stringify({
        cod: 200, main: { temp: 10, feels_like: 8.6, pressure: 1009 },
        weather: [{ main: 'Clear', icon: '01d' }],
        clouds: { all: 40 }, wind: { speed: 5, gust: 10 },
      });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.feelsLike).toBe(9);
      expect(result.pressure).toBe(1009);
      expect(result.cloud).toBe(40);
      expect(result.windGustKmh).toBe(36); // 10 m/s * 3.6
    });

    /** Imperial gust (mph) must be normalized to km/h, mirroring the wind-speed conversion. */
    test('converts imperial gust from mph to km/h', () => {
      const body = JSON.stringify({
        cod: 200, main: { temp: 10 },
        weather: [{ main: 'Clear', icon: '01d' }], wind: { speed: 5, gust: 10 },
      });

      const result = run({ ...BASE, fahrenheit: true }, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.windGustKmh).toBe(16); // 10 mph * 1.609344
    });

    /** Imperial wind (mph) must be normalized to km/h, not shipped as a raw mph number. */
    test('converts imperial wind from mph to km/h', () => {
      const body = JSON.stringify({
        cod: 200, main: { temp: 10, humidity: 50 },
        weather: [{ main: 'Clear', icon: '01d' }], wind: { speed: 10 },
      });

      const result = run({ ...BASE, fahrenheit: true }, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.windKmh).toBe(16); // 10 mph * 1.609344
    });
  });

  describe('forecast strip', () => {
    // a single Open-Meteo response carrying both the extras and the forecast blocks
    const OM_FORECAST = JSON.stringify({
      current: { time: '2026-07-10T12:00', uv_index: 4, dew_point_2m: -2 },
      hourly: {
        time: ['2026-07-10T12:00', '2026-07-10T13:00', '2026-07-10T14:00', '2026-07-10T15:00', '2026-07-10T16:00'],
        temperature_2m: [18, 19, 20, 21, 22],
        weather_code: [0, 1, 2, 3, 45],
        is_day: [1, 1, 1, 1, 1],
      },
      daily: {
        time: ['2026-07-10', '2026-07-11'],
        weather_code: [0, 1],
        temperature_2m_max: [22, 23],
        temperature_2m_min: [12, 13],
      },
    });

    /** OWM has no hourly of its own, so a blank forecast row means the borrowed Open-Meteo strip never attached. */
    test('attaches the Open-Meteo forecast strip when the face wants it', () => {
      const result = run({ ...BASE, wantForecast: true }, routing({ [WX]: { body: WX_OK }, [OM]: { body: OM_FORECAST } }, []));

      expect(result.forecastHourly).toBeTruthy();
      expect(result.forecastHourly.cols.length).toBeGreaterThan(0);
      expect(result.forecastDaily).toBeTruthy();
    });

    /** With a forecast face the single Open-Meteo call must carry the hourly block and extra days. */
    test('asks Open-Meteo for the hourly block and forecast days for a forecast face', () => {
      const calls: string[] = [];

      run({ ...BASE, wantForecast: true }, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      const omCall = calls.find((url) => url.includes(OM));
      expect(omCall).toContain('hourly=temperature_2m,weather_code,is_day');
      expect(omCall).toContain('forecast_days=8');
    });

    /** A plain reading face must not pay for the hourly block it never shows. */
    test('leaves the hourly block off the Open-Meteo url without a forecast row', () => {
      const calls: string[] = [];

      run(BASE, routing({ [WX]: { body: WX_OK }, [OM]: { body: '{}' } }, calls));

      const omCall = calls.find((url) => url.includes(OM));
      expect(omCall).not.toContain('hourly=');
    });
  });

  describe('weather-step errors', () => {
    /** A weather-call failure must not be silently swallowed. */
    test('maps a weather network error to Net Error', () => {
      const result = run(BASE, routing({ [WX]: { err: 'HTTP 500' }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('NET ERROR');
    });

    /** A 401 on the weather call must surface as Invalid Key. */
    test('maps a 401 from the weather call to Invalid Key', () => {
      const body = JSON.stringify({ cod: 401 });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('INVALID KEY');
    });

    /** Any non-200 code other than 401 is a generic API error. */
    test('maps a non-401 error code to API Error', () => {
      const body = JSON.stringify({ cod: 429, message: 'rate' });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('API ERROR');
    });

    /** A response missing main/weather must report No Wx Data, not crash on undefined. */
    test('maps a weather response missing fields to No Wx Data', () => {
      const body = JSON.stringify({ cod: 200 });

      const result = run(BASE, routing({ [WX]: { body }, [OM]: { body: '{}' } }, []));

      expect(result.condition).toBe('NO WX DATA');
    });
  });

  // live integration against the real OpenWeatherMap API. opt-in via RUN_LIVE_WEATHER=1
  // plus OWM_KEY. catches the upstream changing response shape
  describe.skipIf(process.env.RUN_LIVE_WEATHER !== '1' || !process.env.OWM_KEY)('live', () => {
    const KEY = process.env.OWM_KEY;
    const live = (opts: WeatherOpts) => new Promise<WeatherResult>((resolve) => owm.fetch(opts, fetchRequest, resolve));

    /** The real coords path must still parse into a usable reading. */
    test('returns a usable reading for coordinates', async () => {
      const result = await live({ key: KEY, coords: { lat: 51.5074, lon: -0.1278 }, fahrenheit: false });

      expect(result.ok).toBe(true);
      expect(typeof result.temperature).toBe('number');
      expect(result.condition).toBeTruthy();
    });
  });
});
