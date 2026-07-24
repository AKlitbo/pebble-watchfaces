/**
 * OpenWeatherMap weather provider (requires an API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.ts), so this module never geocodes.
 */

import util from '../util';
import openMeteo from './openmeteo';
import type { RequestFn, DoneFn, WeatherOpts } from '../util';
import type { OpenMeteoResponse } from './openmeteo';

const OWM_WEATHER_API = 'https://api.openweathermap.org/data/2.5/weather';

interface OwmMain { temp?: number; humidity?: number; feels_like?: number; pressure?: number }
interface OwmWind { speed?: number; deg?: number; gust?: number }
interface OwmSys { sunrise?: number; sunset?: number }

/** The subset of an OpenWeatherMap current-weather response this provider reads. */
interface OwmResponse {
  cod?: number | string;
  message?: string;
  name?: string;
  timezone?: number;
  main?: OwmMain;
  weather?: Array<{ icon?: string; main?: string }>;
  wind?: OwmWind;
  sys?: OwmSys;
  rain?: { '1h'?: number };
  snow?: { '1h'?: number };
  clouds?: { all?: number };
}

/** Fetches current weather from OpenWeatherMap for the supplied coordinates. */
function fetch(opts: WeatherOpts, request: RequestFn, done: DoneFn): void {
  if (!opts.key) {
    return done(util.status('No API Key'));
  }

  if (!opts.coords) {
    return done(util.status('No Location'));
  }

  const units = opts.fahrenheit ? 'imperial' : 'metric';
  const encodedKey = encodeURIComponent(opts.key);
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  const url = `${OWM_WEATHER_API}?lat=${lat}&lon=${lon}&units=${units}&appid=${encodedKey}`;

  // fetch OWM first, then the Open-Meteo extras, one request in flight at a time rather than
  // both at once. a parallel join could strand done() when a request never called back, which
  // left the watch stuck on its placeholder with no retry. openmeteo and weatherapi go the
  // same one-at-a-time way
  util.requestJson<OwmResponse>(url, request, done, (json) => {
    // OWM returns a 'cod' field with the HTTP status. 401 is usually a bad key
    // it comes as a number on success but often a string on errors so normalize first
    const cod = Number(json.cod);
    if (Number.isFinite(cod) && cod !== 200) {
      console.log('owm api error:', json.message);
      return done(util.status(cod === 401 ? 'Invalid Key' : 'API Error'));
    }

    if (!json.main || !json.weather || !json.weather.length) {
      return done(util.status('No Wx Data'));
    }

    // OWM icon codes end in 'd' for day or 'n' for night. a missing icon counts
    // as day so a clear sky never wrongly shows a moon
    const icon = json.weather[0].icon || '';
    const isDay = !icon.endsWith('n');

    // OWM wind speed is m/s for metric or mph for imperial so normalize to km/h
    const toKmh = (value: unknown) => Number(value) * (opts.fahrenheit ? 1.609344 : 3.6);
    const wind: OwmWind = json.wind || {};
    const windKmh = Number.isFinite(Number(wind.speed)) ? toKmh(wind.speed) : undefined;

    const sys: OwmSys = json.sys || {};
    const rain: { '1h'?: number } = json.rain || {};
    const snow: { '1h'?: number } = json.snow || {};
    const clouds: { all?: number } = json.clouds || {};
    const precip = (rain['1h'] || snow['1h'] || 0);

    // gust uses the same unit as wind.speed (m/s metric or mph imperial) so normalize to km/h
    const windGustKmh = Number.isFinite(Number(wind.gust)) ? toKmh(wind.gust) : undefined;

    // OWM lumps every cloud level under main "Clouds". split partly (icon code
    // 02) from overcast (03/04) using the icon code before night promotion
    let cond = json.weather[0].main || '';
    if (String(cond).toLowerCase() === 'clouds') {
      cond = icon.slice(0, 2) === '02' ? 'PCLDY' : 'CLDY';
    }

    const result = util.ok(
      json.main.temp,
      util.applyNight(cond, isDay),
      opts.label || json.name,
      lat,
      lon,
      {
        humidity: json.main.humidity,
        windKmh: windKmh,
        windDir: util.degToCompass(wind.deg),
        precip: precip,
        feelsLike: json.main.feels_like,
        pressure: json.main.pressure,
        cloud: clouds.all,
        windGustKmh: windGustKmh,
        sunrise: util.hmFromUnix(sys.sunrise, json.timezone),
        sunset: util.hmFromUnix(sys.sunset, json.timezone),
      }
    );

    // a non-numeric temp already yielded a status, so there is nothing to enrich
    if (!result.ok) {
      return done(result);
    }

    // OWM's free endpoint is missing UV and dew point and any forecast so a follow-up
    // Open-Meteo request grabs them. when the face wants the forecast the same call also
    // brings back the hourly and daily strips. the URL and its field list live in the
    // openmeteo module so the two never drift apart. a failure just keeps the OWM reading
    const omUrl = openMeteo.extrasUrl(opts);
    util.requestJson<OpenMeteoResponse>(omUrl, request, () => done(result), (om) => {
      util.attachExtras(result, openMeteo.parseExtras(om));
      if (opts.wantForecast) {
        util.attachForecast(result, openMeteo.parseForecast(om));
      }
      done(result);
    });
  });
}

export default { fetch };
