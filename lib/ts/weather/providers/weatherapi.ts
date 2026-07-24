/**
 * WeatherAPI.com weather provider (requires an API key).
 *
 * WeatherAPI resolves a place name or "lat,lon" query server-side, so this
 * module needs no separate geocoding step.
 */

import util from '../util';
import openMeteo from './openmeteo';
import type { RequestFn, DoneFn, WeatherOpts, WeatherResult, ForecastCols } from '../util';

const WEATHER_API_URL = 'https://api.weatherapi.com/v1/forecast.json';

// WeatherAPI's numeric condition.code is stable across locales unlike the text
// which is localized. map it to our tokens directly and an unlisted code falls back
// to text aliasing via shorten(). codes come from WeatherAPI's published list
const WEATHERAPI_CODE: Record<number, string> = {
  1000: 'CLEAR',
  1003: 'PCLDY',
  1006: 'CLDY', 1009: 'CLDY',
  1030: 'FOGGY', 1135: 'FOGGY', 1147: 'FOGGY',
  1063: 'RAIN', 1180: 'RAIN', 1183: 'RAIN', 1186: 'RAIN', 1189: 'RAIN', 1192: 'RAIN', 1195: 'RAIN',
  1240: 'SHWR', 1243: 'SHWR', 1246: 'SHWR',
  1150: 'DRZL', 1153: 'DRZL',
  1072: 'FZDZ', 1168: 'FZDZ', 1171: 'FZDZ',
  1066: 'SNOW', 1210: 'SNOW', 1213: 'SNOW', 1216: 'SNOW', 1219: 'SNOW', 1222: 'SNOW', 1225: 'SNOW',
  1114: 'SNOW', 1117: 'SNOW',
  1255: 'SNSH', 1258: 'SNSH',
  1069: 'FZRN', 1198: 'FZRN', 1201: 'FZRN', 1204: 'FZRN', 1207: 'FZRN', 1249: 'FZRN', 1252: 'FZRN',
  1237: 'FZRN', 1261: 'FZRN', 1264: 'FZRN',
  1087: 'STRM', 1273: 'STRM', 1276: 'STRM', 1279: 'STRM', 1282: 'STRM',
};

interface WeatherApiDay {
  maxtemp_f?: number; maxtemp_c?: number; mintemp_f?: number; mintemp_c?: number;
  daily_chance_of_rain?: number; totalprecip_mm?: number; uv?: number;
}

interface WeatherApiForecastDay {
  astro?: { sunrise?: string; sunset?: string };
  day?: WeatherApiDay;
}

/** The subset of a WeatherAPI forecast.json response this provider reads. */
interface WeatherApiResponse {
  error?: { message?: string; code?: number };
  location?: { name?: string; lat?: number; lon?: number };
  current?: {
    condition?: { code?: number; text?: string };
    temp_f?: number; temp_c?: number; is_day?: number;
    humidity?: number; wind_kph?: number; wind_dir?: string; uv?: number;
    precip_mm?: number; feelslike_f?: number; feelslike_c?: number;
    pressure_mb?: number; cloud?: number; gust_kph?: number;
    dewpoint_f?: number; dewpoint_c?: number;
  };
  forecast?: { forecastday?: WeatherApiForecastDay[] };
}

/**
 * Turns a WeatherAPI forecast.json response into a weather result.
 *
 * Kept separate from the network flow so the parsing is easy to exercise on its own.
 * Returns a status result for an error or missing-data response, or a full reading.
 */
function parseWeather(json: WeatherApiResponse, opts: WeatherOpts): WeatherResult {
  // WeatherAPI signals failures with an error object carrying a numeric code
  if (json.error) {
    console.log('weatherapi error:', json.error.message);

    // 1006 is 'no matching location found'
    if (json.error.code === 1006) {
      return util.status('Loc Not Found');
    }

    // codes 1002 and 2006 and 2008 are forms of invalid or disabled API keys
    if ([1002, 2006, 2008].includes(Number(json.error.code))) {
      return util.status('Invalid Key');
    }

    return util.status('API Error');
  }

  if (!json.current || !json.current.condition) {
    return util.status('No Wx Data');
  }

  // prefer the label resolved at geocode time then WeatherAPI's own
  // resolved name then the raw place the user typed
  const label = opts.label ||
      (json.location && json.location.name) ||
      (opts.coords ? 'My Location' : opts.place);

  const temperature = opts.fahrenheit ? json.current.temp_f : json.current.temp_c;

  // is_day is 1 by day and 0 at night. a missing value counts as day so a
  // clear sky never wrongly shows a moon
  const isDay = json.current.is_day !== 0;

  // WeatherAPI echoes the resolved location's coordinates in json.location
  const loc: { name?: string; lat?: number; lon?: number } = json.location || {};

  const forecastDay: WeatherApiForecastDay = (json.forecast?.forecastday && json.forecast.forecastday[0]) || {};
  const astro: { sunrise?: string; sunset?: string } = forecastDay.astro || {};
  const day: WeatherApiDay = forecastDay.day || {};

  // prefer the stable numeric code and fall back to the localized text which
  // shorten() still resolves through CONDITION_ALIASES for any unlisted code
  const cond = WEATHERAPI_CODE[Number(json.current.condition.code)] || json.current.condition.text || '';

  return util.ok(
    temperature,
    util.applyNight(cond, isDay),
    label,
    loc.lat,
    loc.lon,
    {
      humidity: json.current.humidity,
      windKmh: json.current.wind_kph,
      windDir: json.current.wind_dir,
      uvIndex: json.current.uv,
      precip: json.current.precip_mm,
      feelsLike: opts.fahrenheit ? json.current.feelslike_f : json.current.feelslike_c,
      pressure: json.current.pressure_mb,
      cloud: json.current.cloud,
      windGustKmh: json.current.gust_kph,
      dewPoint: opts.fahrenheit ? json.current.dewpoint_f : json.current.dewpoint_c,
      tempMax: opts.fahrenheit ? day.maxtemp_f : day.maxtemp_c,
      tempMin: opts.fahrenheit ? day.mintemp_f : day.mintemp_c,
      precipChance: day.daily_chance_of_rain,
      precipTotal: day.totalprecip_mm,
      uvMax: day.uv,
      sunrise: util.hmFrom12Hour(astro.sunrise),
      sunset: util.hmFrom12Hour(astro.sunset),
    }
  );
}

/** Fetches current weather from WeatherAPI.com. */
function fetch(opts: WeatherOpts, request: RequestFn, done: DoneFn): void {
  if (!opts.key) {
    return done(util.status('No API Key'));
  }

  // WeatherAPI accepts a lat/lon string or a place name in the same q param
  const query = opts.coords
    ? `${opts.coords.lat},${opts.coords.lon}`
    : opts.place;

  if (!query) {
    return done(util.status('No Location'));
  }

  const url = `${WEATHER_API_URL}?key=${encodeURIComponent(opts.key)}&q=${encodeURIComponent(query)}&days=1`;

  // WeatherAPI has no forecast strip of its own so when the face shows the forecast row we
  // borrow it from Open-Meteo the same way OWM does. firing it in parallel here keeps it from
  // becoming a fragile follow-up request. Open-Meteo needs coordinates so a place-name-only
  // lookup just goes without a strip
  const withForecast = opts.wantForecast && opts.coords;
  let pending = withForecast ? 2 : 1;
  let result: WeatherResult | null = null;
  let forecast: ForecastCols | null = null;

  const tryDone = () => {
    if (--pending > 0) {
      return;
    }

    if (result && result.ok && forecast) {
      util.attachForecast(result, forecast);
    }

    done(result as WeatherResult);
  };

  util.requestJson<WeatherApiResponse>(url, request, (status) => {
    result = status;
    tryDone();
  }, (json) => {
    result = parseWeather(json, opts);
    tryDone();
  });

  if (withForecast) {
    openMeteo.fetchForecast(opts, request, (fetched) => {
      forecast = fetched;
      tryDone();
    });
  }
}

export default { fetch, parseWeather };
