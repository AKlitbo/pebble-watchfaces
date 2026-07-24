/**
 * Open-Meteo weather provider (free, no API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.ts), so this module never geocodes.
 */

import util from '../util';
import conditions from '../conditions';
import type { RequestFn, DoneFn, WeatherOpts, WeatherCoords, HourlyStrip, DailyStrip, ForecastCols } from '../util';

const OPEN_METEO_FORECAST_API = 'https://api.open-meteo.com/v1/forecast';

// how many columns the forecast row shows and how far apart the hourly ones sit. eight fills
// the 2x4's two stacked rows (the hourly strip then spans the next 16 hours at a 2-hour step)
const FORECAST_COLS = 8;
const HOURLY_STEP_HOURS = 2;

/** The subset of an Open-Meteo forecast response this module reads. */
export interface OpenMeteoResponse {
  error?: boolean;
  reason?: string;
  current?: {
    time?: string;
    temperature_2m?: number;
    weather_code?: number;
    is_day?: number;
    relative_humidity_2m?: number;
    wind_speed_10m?: number;
    wind_direction_10m?: number;
    uv_index?: number;
    precipitation?: number;
    apparent_temperature?: number;
    surface_pressure?: number;
    cloud_cover?: number;
    wind_gusts_10m?: number;
    dew_point_2m?: number;
  };
  hourly?: {
    time?: string[];
    temperature_2m?: number[];
    weather_code?: number[];
    is_day?: number[];
  };
  daily?: {
    time?: string[];
    temperature_2m_max?: number[];
    temperature_2m_min?: number[];
    weather_code?: number[];
    sunrise?: string[];
    sunset?: string[];
    precipitation_probability_max?: number[];
    precipitation_sum?: number[];
    uv_index_max?: number[];
  };
}

/**
 * Rounds a value to a whole number, or null when it isn't a real number.
 *
 * A forecast column with no temperature ships null so the packer can swap in the
 * no-reading sentinel instead of a bogus zero.
 */
function roundOrNull(value: unknown): number | null {
  // null/undefined/'' all coerce to 0 through Number() so reject them first
  if (value === null || value === undefined || value === '') {
    return null;
  }

  const number = Number(value);
  return Number.isFinite(number) ? Math.round(number) : null;
}

/** Pulls the hour (0-23) out of an ISO timestamp, or -1 when it has no clock part. */
function hourOfIso(iso: unknown): number {
  const match = /T(\d{2}):/.exec(String(iso || ''));
  return match ? parseInt(match[1], 10) : -1;
}

/**
 * Works out the weekday (0=Sunday .. 6=Saturday) of an ISO date like "2026-07-05".
 * Parsed as UTC so the phone's own timezone can't nudge it onto the wrong day.
 */
function weekdayOfIso(isoDate: unknown): number {
  const match = /^(\d{4})-(\d{2})-(\d{2})/.exec(String(isoDate || ''));
  if (!match) {
    return -1;
  }

  const year = parseInt(match[1], 10);
  const month = parseInt(match[2], 10);
  const day = parseInt(match[3], 10);
  return new Date(Date.UTC(year, month - 1, day)).getUTCDay();
}

/**
 * Builds the hourly strip from an Open-Meteo response.
 *
 * Starts at the first slot at or after now so the row shows upcoming hours, then
 * strides by HOURLY_STEP_HOURS. Each column carries the condition as its wire
 * code and the temperature in the unit the request already asked for. A column
 * whose hour falls after dark (is_day 0) gets the night bit so the watch loads
 * the night glyph. A missing is_day is treated as day, same as the current icon.
 */
function parseHourly(json: OpenMeteoResponse | null): HourlyStrip | null {
  const hourly = json?.hourly;
  const current = json?.current;
  if (!hourly || !Array.isArray(hourly.time) || !Array.isArray(hourly.temperature_2m)) {
    return null;
  }

  const times = hourly.time;
  const temps = hourly.temperature_2m;
  const codes = hourly.weather_code || [];
  const isDay = hourly.is_day || [];

  const now = current?.time || '';
  let start = times.findIndex((time) => time >= now);
  if (start < 0) {
    start = 0;
  }

  const cols = [];
  for (let column = 0; column < FORECAST_COLS; column++) {
    const index = start + column * HOURLY_STEP_HOURS;
    if (index >= times.length) {
      break;
    }
    let code = conditions.codeFor(util.wmoToCondition(codes[index]));
    // mark the night hours. an unknown code has no night glyph so leave it alone
    if (code !== conditions.UNKNOWN_CODE && isDay[index] === 0) {
      code |= conditions.FORECAST_NIGHT_BIT;
    }
    cols.push({
      code: code,
      temp: roundOrNull(temps[index]),
    });
  }

  if (!cols.length) {
    return null;
  }

  // a base we can't read would ship a bogus hour the watch labels the whole strip
  // with so drop the strip instead of sending a wrong sentinel
  const baseHour = hourOfIso(times[start]);
  if (baseHour < 0) {
    return null;
  }

  return { baseHour: baseHour, stepHours: HOURLY_STEP_HOURS, cols };
}

/**
 * Builds the 7-day strip from an Open-Meteo response.
 *
 * Takes the first FORECAST_COLS days starting today. Each column carries the
 * condition code plus the day's high and low in the user's unit.
 */
function parseDaily(json: OpenMeteoResponse | null): DailyStrip | null {
  const daily = json?.daily;
  if (!daily || !Array.isArray(daily.time)) {
    return null;
  }

  const times = daily.time;
  const maxes = daily.temperature_2m_max || [];
  const mins = daily.temperature_2m_min || [];
  const codes = daily.weather_code || [];

  const cols = [];
  for (let column = 0; column < FORECAST_COLS && column < times.length; column++) {
    cols.push({
      code: conditions.codeFor(util.wmoToCondition(codes[column])),
      tempMax: roundOrNull(maxes[column]),
      tempMin: roundOrNull(mins[column]),
    });
  }

  if (!cols.length) {
    return null;
  }

  // same guard as the hourly strip: a base weekday we can't read would mislabel every day
  const baseWeekday = weekdayOfIso(times[0]);
  if (baseWeekday < 0) {
    return null;
  }

  return { baseWeekday: baseWeekday, cols };
}

/**
 * Pulls the hourly and 7-day forecast strips out of an Open-Meteo response.
 *
 * Shared like parseExtras: the openmeteo provider parses it from its own
 * response, while owm/weatherapi feed a supplemental Open-Meteo response
 * through the same parser so every face gets the same forecast shape.
 */
function parseForecast(json: OpenMeteoResponse | null): ForecastCols {
  return { hourly: parseHourly(json), daily: parseDaily(json) };
}

/**
 * Builds a minimal Open-Meteo URL that carries only the forecast strips.
 * Used by the other providers to fill in a forecast they can't fetch natively.
 */
function forecastUrl(opts: WeatherOpts): string {
  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const coords = opts.coords as WeatherCoords;
  const lat = coords.lat;
  const lon = coords.lon;
  // current=temperature_2m is only here to bring back current.time so the hourly
  // strip knows where "now" is. is_day per hour marks which columns fall after dark
  const hourly = 'temperature_2m,weather_code,is_day';
  const daily = 'weather_code,temperature_2m_max,temperature_2m_min';
  // forecast_days=8 so the daily strip can fill the 2x4's eight columns (the default is 7)
  return `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=temperature_2m` +
    `&hourly=${hourly}&daily=${daily}&forecast_days=8&temperature_unit=${unit}&timezone=auto`;
}

/**
 * Builds an Open-Meteo URL carrying the extras parseExtras reads (UV, dew point,
 * and the daily high/low/precip/uv strip), plus the forecast strips when the face
 * asks for them.
 *
 * Paired with parseExtras so the field list lives in one place. Used by OWM, whose
 * free endpoint lacks all of this, instead of OWM hand-maintaining its own copy.
 * When opts.wantForecast is set the same call also brings back the hourly and daily
 * strips, so OWM fills its forecast row from this one request instead of a second one.
 */
function extrasUrl(opts: WeatherOpts): string {
  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const coords = opts.coords as WeatherCoords;
  const lat = coords.lat;
  const lon = coords.lon;

  let current = 'uv_index,dew_point_2m';
  let daily = 'temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum,uv_index_max';
  let forecast = '';
  if (opts.wantForecast) {
    // temperature_2m rides along only to bring back current.time so parseHourly knows
    // where now is. weather_code and the hourly block feed the forecast strips
    current += ',temperature_2m';
    daily += ',weather_code';
    forecast = '&hourly=temperature_2m,weather_code,is_day&forecast_days=8';
  }

  return `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}` +
    `&current=${current}&daily=${daily}${forecast}&temperature_unit=${unit}&timezone=auto`;
}

/**
 * Fetches just the forecast strips from Open-Meteo, for a provider that can't
 * supply its own. Any failure yields null so the caller keeps its reading.
 */
function fetchForecast(opts: WeatherOpts, request: RequestFn, done: (forecast: ForecastCols | null) => void): void {
  if (!opts || !opts.coords) {
    return done(null);
  }

  util.requestJson<OpenMeteoResponse>(forecastUrl(opts), request, () => done(null), (json) => {
    if (json.error) {
      return done(null);
    }
    done(parseForecast(json));
  });
}

/**
 * Pulls the UV, dew point, and daily-forecast extras out of an Open-Meteo
 * forecast response, raw and unrounded so the shared attachExtras can round and
 * scale them.
 *
 * Shared with the OWM provider: OWM's free endpoint lacks these, so it steals
 * them from a parallel Open-Meteo call. Keeping the field mapping here means the
 * Open-Meteo paths live in one place instead of drifting across two providers.
 */
function parseExtras(json: OpenMeteoResponse | null): Record<string, unknown> {
  const cur = json?.current;
  const daily = json?.daily;
  const first = (arr: unknown): unknown => (Array.isArray(arr) ? arr[0] : undefined);

  return {
    uvIndex: cur?.uv_index,
    dewPoint: cur?.dew_point_2m,
    tempMax: first(daily?.temperature_2m_max),
    tempMin: first(daily?.temperature_2m_min),
    precipChance: first(daily?.precipitation_probability_max),
    precipTotal: first(daily?.precipitation_sum),
    uvMax: first(daily?.uv_index_max),
  };
}

/** Fetches current weather from Open-Meteo for the supplied coordinates. */
function fetch(opts: WeatherOpts, request: RequestFn, done: DoneFn): void {
  if (!opts.coords) {
    return done(util.status('No Location'));
  }

  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  // wind_speed_unit=kmh keeps wind in km/h whatever the temperature unit
  // timezone=auto makes the daily sunrise/sunset come back as local times
  const current = 'temperature_2m,weather_code,is_day,relative_humidity_2m,wind_speed_10m,wind_direction_10m,' +
    'uv_index,precipitation,apparent_temperature,surface_pressure,cloud_cover,wind_gusts_10m,dew_point_2m';
  // the sunrise and sunset fields stay first so existing URL assertions still match daily=
  let dailyFields = 'sunrise,sunset,temperature_2m_max,temperature_2m_min,precipitation_probability_max,' +
    'precipitation_sum,uv_index_max';
  let url = `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=${current}`;

  // only a face that shows the forecast row pays for the hourly block and the extra
  // days. everyone else gets just the current reading plus today's daily extras
  if (opts.wantForecast) {
    // weather_code rides last so the daily strip knows each day's sky
    dailyFields += ',weather_code';
    // hourly feeds the forecast row. temperature_2m and weather_code per hour plus
    // is_day so each column can pick a day or night glyph
    url += '&hourly=temperature_2m,weather_code,is_day';
    // forecast_days=8 so the daily strip can fill the 2x4's eight columns (the default is 7)
    url += '&forecast_days=8';
  }

  url += `&daily=${dailyFields}&temperature_unit=${unit}&wind_speed_unit=kmh&timezone=auto`;

  util.requestJson<OpenMeteoResponse>(url, request, done, (json) => {
    if (json.error) {
      console.log('open-meteo api error:', json.reason);
      return done(util.status('API Error'));
    }

    if (!json.current) {
      return done(util.status('No Wx Data'));
    }

    const cur = json.current;
    const daily = json.daily;

    // is_day is 1 by day and 0 at night. treat a missing value as day so a
    // clear sky never wrongly shows a moon
    const isDay = cur.is_day !== 0;

    const result = util.ok(
      cur.temperature_2m,
      util.applyNight(util.wmoToCondition(Number(cur.weather_code)), isDay),
      opts.label || 'My Location',
      lat,
      lon,
      Object.assign({
        humidity: cur.relative_humidity_2m,
        windKmh: cur.wind_speed_10m,
        windDir: util.degToCompass(cur.wind_direction_10m),
        precip: cur.precipitation,
        feelsLike: cur.apparent_temperature,
        pressure: cur.surface_pressure,
        cloud: cur.cloud_cover,
        windGustKmh: cur.wind_gusts_10m,
        sunrise: util.hmFromIso(daily?.sunrise?.[0]),
        sunset: util.hmFromIso(daily?.sunset?.[0]),
        // uv and dew point and the daily high/low and precip chance/total and uv max
      }, parseExtras(json))
    );

    // the response already carries the hourly and daily strips so read them
    // straight off it. no extra request needed for the openmeteo provider
    if (result.ok && opts.wantForecast) {
      util.attachForecast(result, parseForecast(json));
    }

    done(result);
  });
}

export default { fetch, parseExtras, extrasUrl, parseForecast, fetchForecast };
