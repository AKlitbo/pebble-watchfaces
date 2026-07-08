/**
 * Open-Meteo weather provider (free, no API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.js), so this module never geocodes.
 */
'use strict';

const util = require('../util');
const conditions = require('../conditions');

const OPEN_METEO_FORECAST_API = 'https://api.open-meteo.com/v1/forecast';

// how many columns the forecast row shows, and how far apart the hourly ones sit. eight fills
// the 2x4's two stacked rows (the hourly strip then spans the next 16 hours at a 2-hour step)
const FORECAST_COLS = 8;
const HOURLY_STEP_HOURS = 2;

/**
 * Rounds a value to a whole number, or null when it isn't a real number.
 *
 * A forecast column with no temperature ships null so the packer can swap in the
 * no-reading sentinel instead of a bogus zero.
 *
 * @param {*} value
 * @return {?number}
 */
function roundOrNull(value) {
  // null/undefined/'' all coerce to 0 through Number() so reject them first
  if (value === null || value === undefined || value === '') {
    return null;
  }

  const number = Number(value);
  return Number.isFinite(number) ? Math.round(number) : null;
}

/**
 * Pulls the hour (0-23) out of an ISO timestamp like "2026-07-05T09:00".
 *
 * @param {*} iso
 * @return {number} The hour, or -1 when the value has no clock part.
 */
function hourOfIso(iso) {
  const match = /T(\d{2}):/.exec(String(iso || ''));
  return match ? parseInt(match[1], 10) : -1;
}

/**
 * Works out the weekday (0=Sunday .. 6=Saturday) of an ISO date like "2026-07-05".
 *
 * Parsed as UTC so the phone's own timezone can't nudge it onto the wrong day.
 *
 * @param {*} isoDate
 * @return {number} The weekday, or -1 when the value isn't a date.
 */
function weekdayOfIso(isoDate) {
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
 *
 * @param {?Object} json An Open-Meteo forecast response.
 * @return {?{baseHour: number, stepHours: number, cols: {code: number, temp: ?number}[]}}
 */
function parseHourly(json) {
  const hourly = json && json.hourly;
  const current = (json && json.current) || {};
  if (!hourly || !Array.isArray(hourly.time) || !Array.isArray(hourly.temperature_2m)) {
    return null;
  }

  const times = hourly.time;
  const temps = hourly.temperature_2m;
  const codes = hourly.weather_code || [];
  const isDay = hourly.is_day || [];

  const now = current.time || '';
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
      temp: roundOrNull(temps[index])
    });
  }

  if (!cols.length) {
    return null;
  }

  return { baseHour: hourOfIso(times[start]), stepHours: HOURLY_STEP_HOURS, cols };
}

/**
 * Builds the 7-day strip from an Open-Meteo response.
 *
 * Takes the first FORECAST_COLS days starting today. Each column carries the
 * condition code plus the day's high and low in the user's unit.
 *
 * @param {?Object} json An Open-Meteo forecast response.
 * @return {?{baseWeekday: number, cols: {code: number, tempMax: ?number, tempMin: ?number}[]}}
 */
function parseDaily(json) {
  const daily = json && json.daily;
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
      tempMin: roundOrNull(mins[column])
    });
  }

  if (!cols.length) {
    return null;
  }

  return { baseWeekday: weekdayOfIso(times[0]), cols };
}

/**
 * Pulls the hourly and 7-day forecast strips out of an Open-Meteo response.
 *
 * Shared like parseExtras: the openmeteo provider parses it from its own
 * response, while owm/weatherapi feed a supplemental Open-Meteo response
 * through the same parser so every face gets the same forecast shape.
 *
 * @param {?Object} json An Open-Meteo forecast response.
 * @return {{hourly: ?Object, daily: ?Object}}
 */
function parseForecast(json) {
  return { hourly: parseHourly(json), daily: parseDaily(json) };
}

/**
 * Builds a minimal Open-Meteo URL that carries only the forecast strips.
 *
 * Used by the other providers to fill in a forecast they can't fetch natively.
 *
 * @param {Object} opts fahrenheit, coords
 * @return {string}
 */
function forecastUrl(opts) {
  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  // current=temperature_2m is only here to bring back current.time so the hourly
  // strip knows where "now" is. is_day per hour marks which columns fall after dark
  const hourly = 'temperature_2m,weather_code,is_day';
  const daily = 'weather_code,temperature_2m_max,temperature_2m_min';
  // forecast_days=8 so the daily strip can fill the 2x4's eight columns (the default is 7)
  return `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=temperature_2m` +
    `&hourly=${hourly}&daily=${daily}&forecast_days=8&temperature_unit=${unit}&timezone=auto`;
}

/**
 * Fetches just the forecast strips from Open-Meteo, for a provider that can't
 * supply its own. Any failure yields null so the caller keeps its reading.
 *
 * @param {Object} opts fahrenheit, coords
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with {hourly, daily} or null
 */
function fetchForecast(opts, request, done) {
  if (!opts || !opts.coords) {
    return done(null);
  }

  util.requestJson(forecastUrl(opts), request, () => done(null), (json) => {
    if (json && json.error) {
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
 *
 * @param {?Object} json An Open-Meteo forecast response (or null on failure).
 * @return {!Object} {uvIndex, dewPoint, tempMax, tempMin, precipChance, precipTotal, uvMax}
 */
function parseExtras(json) {
  const cur = (json && json.current) || {};
  const daily = (json && json.daily) || {};
  const first = (arr) => (Array.isArray(arr) ? arr[0] : undefined);

  return {
    uvIndex: cur.uv_index,
    dewPoint: cur.dew_point_2m,
    tempMax: first(daily.temperature_2m_max),
    tempMin: first(daily.temperature_2m_min),
    precipChance: first(daily.precipitation_probability_max),
    precipTotal: first(daily.precipitation_sum),
    uvMax: first(daily.uv_index_max)
  };
}

/**
 * Fetches current weather from Open-Meteo for the supplied coordinates.
 *
 * @param {Object} opts fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetch(opts, request, done) {
  if (!opts.coords) {
    return done(util.status('No Location'));
  }

  const unit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const lat = opts.coords.lat;
  const lon = opts.coords.lon;
  // wind_speed_unit=kmh keeps wind in km/h whatever the temperature unit
  // timezone=auto makes the daily sunrise/sunset come back as local times
  const current ='temperature_2m,weather_code,is_day,relative_humidity_2m,wind_speed_10m,wind_direction_10m,' +
    'uv_index,precipitation,apparent_temperature,surface_pressure,cloud_cover,wind_gusts_10m,dew_point_2m';
  // the sunrise and sunset fields stay first so existing URL assertions still match daily=
  // weather_code rides last so the daily strip knows each day's sky
  const daily = 'sunrise,sunset,temperature_2m_max,temperature_2m_min,precipitation_probability_max,' +
    'precipitation_sum,uv_index_max,weather_code';
  // hourly feeds the forecast row. temperature_2m and weather_code per hour, plus is_day
  // so each column can pick a day or night glyph
  const hourly = 'temperature_2m,weather_code,is_day';
  // forecast_days=8 so the daily strip can fill the 2x4's eight columns (the default is 7)
  const url = `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=${current}` +
    `&hourly=${hourly}&daily=${daily}&forecast_days=8&temperature_unit=${unit}&wind_speed_unit=kmh&timezone=auto`;

  util.requestJson(url, request, done, (json) => {
    if (json.error) {
      console.log('open-meteo api error:', json.reason);
      return done(util.status('API Error'));
    }

    if (!json.current) {
      return done(util.status('No Wx Data'));
    }

    const cur = json.current;
    const daily = json.daily || {};

    // is_day is 1 by day and 0 at night. treat a missing value as day so a
    // clear sky never wrongly shows a moon
    const isDay = cur.is_day !== 0;

    const result = util.ok(
      cur.temperature_2m,
      util.applyNight(util.wmoToCondition(cur.weather_code), isDay),
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
        sunrise: util.hmFromIso(daily.sunrise && daily.sunrise[0]),
        sunset: util.hmFromIso(daily.sunset && daily.sunset[0])
        // uv and dew point and the daily high/low and precip chance/total and uv max
      }, parseExtras(json))
    );

    // the response already carries the hourly and daily strips so read them
    // straight off it. no extra request needed for the openmeteo provider
    if (result.ok) {
      util.attachForecast(result, parseForecast(json));
    }

    done(result);
  });
}

module.exports = { fetch, parseExtras, parseForecast, fetchForecast };
