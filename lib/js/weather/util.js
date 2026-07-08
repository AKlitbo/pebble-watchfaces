/**
 * Shared helpers for the weather provider modules.
 *
 * Provides the parsing, formatting, and result builders used by openmeteo,
 * owm, and weatherapi so every provider returns the same normalized shape.
 */
'use strict';

const conditions = require('./conditions');

// the tokens the C icon table knows about (the single source of truth in conditions.js)
// a night-capable row adds both its day token and the "<token>_NIGHT" form the wire
// carries after dark so shorten() still knows a token once it is night-promoted
const KNOWN_TOKENS = new Set();
for (const entry of conditions.conditions) {
  KNOWN_TOKENS.add(entry.token);
  if (entry.nightResource) {
    KNOWN_TOKENS.add(`${entry.token}_NIGHT`);
  }
}

// rows that ship a distinct night glyph. applyNight only promotes these
const NIGHT_TOKENS = new Set(
  conditions.conditions.filter((entry) => entry.nightResource).map((entry) => entry.token)
);

/**
 * Maps an Open-Meteo WMO weather code to a short condition string.
 *
 * @param {number} code
 * @return {string}
 */
function wmoToCondition(code) {
  if (code === 0) { return 'CLEAR'; }
  if (code === 1 || code === 2) { return 'PCLDY'; }
  if (code === 3) { return 'CLDY'; }
  if (code <= 48) { return 'FOGGY'; }
  if (code <= 55) { return 'DRZL'; }
  if (code <= 57) { return 'FZDZ'; }
  if (code <= 65) { return 'RAIN'; }
  if (code <= 67) { return 'FZRN'; }
  if (code <= 77) { return 'SNOW'; }
  if (code <= 82) { return 'SHWR'; }
  if (code <= 86) { return 'SNSH'; }
  if (code <= 99) { return 'STRM'; }

  return 'UNKNOWN';
}

/**
 * Resolves a condition to its known token, promoting to the night form
 * after dark when the condition has a distinct night glyph.
 *
 * Resolving through shorten() first (rather than a bare uppercase) is what makes
 * night work for every provider: Open-Meteo already passes a token, while OWM and
 * WeatherAPI pass phrases/aliases that must be resolved to a token before the night
 * check. Conditions without a night glyph (and unrecognized ones) are returned
 * as their resolved token, unchanged by day or night.
 *
 * @param {string} condition A condition word, alias, or token (see wmoToCondition).
 * @param {boolean} isDay True for daytime, false for night.
 * @return {string}
 */
function applyNight(condition, isDay) {
  const token = shorten(condition);

  if (!isDay && NIGHT_TOKENS.has(token)) {
    return `${token}_NIGHT`;
  }

  return token;
}

// provider phrases that aren't our tokens mapped to the closest one
// the cloud band splits "partly" phrases -> PCLDY and "overcast" or plain cloud -> CLDY
const CONDITION_ALIASES = {
  'SUNNY': 'CLEAR',
  'PARTLY CLOUDY': 'PCLDY',
  'PATCHY RAIN POSSIBLE': 'RAIN',
  'PATCHY RAIN NEARBY': 'RAIN',
  'THUNDERSTORM': 'STRM',
  'THUNDERSTORMS': 'STRM',
  'LIGHT RAIN': 'RAIN',
  'HEAVY RAIN': 'RAIN',
  'LIGHT SNOW': 'SNOW',
  'HEAVY SNOW': 'SNOW',
  'CLOUDS': 'CLDY',
  'CLOUDY': 'CLDY',
  'OVERCAST': 'CLDY',
  'FOG': 'FOGGY',
  'MIST': 'FOGGY',
  'HAZE': 'FOGGY',
  'SMOKE': 'FOGGY'
};

/**
 * Shortens a condition string so it fits beside the temperature.
 *
 * @param {string} text
 * @return {string}
 */
function shorten(text) {
  if (!text) {
    return 'UNKNOWN';
  }

  const upper = text.toUpperCase();
  if (CONDITION_ALIASES[upper]) {
    return CONDITION_ALIASES[upper];
  }
  if (KNOWN_TOKENS.has(upper)) {
    return upper;
  }

  // an unknown phrase (like a verbose WeatherAPI string) would truncate to a
  // partial word the C icon table can't match and show the wrong icon so emit
  // UNKNOWN (-> WI_NA) instead of a fragment
  return 'UNKNOWN';
}

// 16-point compass indexed by the rounded sector (each spans 22.5 degrees)
const COMPASS = ['N', 'NNE', 'NE', 'ENE', 'E', 'ESE', 'SE', 'SSE',
  'S', 'SSW', 'SW', 'WSW', 'W', 'WNW', 'NW', 'NNW'];

/**
 * Turns a wind bearing in degrees into a short compass label.
 *
 * @param {*} degrees
 * @return {string} A compass point, or '' when the bearing is not a number.
 */
function degToCompass(degrees) {
  // null/undefined/'' all coerce to 0 ("N") so reject them before Number()
  if (degrees === null || degrees === undefined || degrees === '') {
    return '';
  }

  const value = Number(degrees);
  if (!Number.isFinite(value)) {
    return '';
  }

  const sector = Math.round((value % 360) / 22.5) % 16;
  return COMPASS[(sector + 16) % 16];
}

/**
 * Pulls "HH:MM" out of an ISO timestamp like "2026-06-26T06:30".
 *
 * Open-Meteo returns local times when the request asks for timezone=auto, so
 * the clock portion can be shown as-is.
 *
 * @param {*} iso
 * @return {string} "HH:MM", or '' when the value is not a usable timestamp.
 */
function hmFromIso(iso) {
  const match = /T(\d{2}:\d{2})/.exec(String(iso || ''));
  return match ? match[1] : '';
}

/**
 * Formats a UTC unix time as a local "HH:MM" using a timezone offset.
 *
 * OpenWeatherMap reports sunrise/sunset as UTC unix seconds plus a `timezone`
 * offset (also seconds), so the local clock is the two added together.
 *
 * @param {*} unixSeconds
 * @param {*} offsetSeconds
 * @return {string} "HH:MM", or '' when either value is not a number.
 */
function hmFromUnix(unixSeconds, offsetSeconds) {
  const unix = Number(unixSeconds);
  const offset = Number(offsetSeconds);
  if (!Number.isFinite(unix) || !Number.isFinite(offset)) {
    return '';
  }

  const local = new Date((unix + offset) * 1000);
  const hours = String(local.getUTCHours()).padStart(2, '0');
  const minutes = String(local.getUTCMinutes()).padStart(2, '0');
  return `${hours}:${minutes}`;
}

/**
 * Parses a 12-hour time string like "05:42 AM" into "HH:MM".
 *
 * @param {string} timeStr
 * @return {string} "HH:MM", or '' if unparseable.
 */
function hmFrom12Hour(timeStr) {
  const match = /^(\d{1,2}):(\d{2})\s*(AM|PM)$/i.exec(String(timeStr || '').trim());
  if (!match) {
    return '';
  }

  let hours = parseInt(match[1], 10);
  const minutes = match[2];
  const isPM = match[3].toUpperCase() === 'PM';

  if (hours === 12) {
    hours = isPM ? 12 : 0;
  } else if (isPM) {
    hours += 12;
  }

  return `${String(hours).padStart(2, '0')}:${minutes}`;
}

/**
 * Copies the optional weather extras onto a result, skipping missing values so
 * the watch keeps showing a placeholder rather than a phantom reading.
 *
 * @param {!Object} result The result being built.
 * @param {?Object} extra {humidity, windKmh, windDir, sunrise, sunset, uvIndex, precip,
 *     feelsLike, pressure, cloud, windGustKmh, dewPoint, tempMax, tempMin,
 *     precipChance, uvMax, precipTotal}
 */
function attachExtras(result, extra) {
  if (!extra) {
    return;
  }

  const humidity = Number(extra.humidity);
  if (Number.isFinite(humidity)) {
    result.humidity = Math.round(humidity);
  }

  const windKmh = Number(extra.windKmh);
  if (Number.isFinite(windKmh)) {
    result.windKmh = Math.round(windKmh);
  }

  if (extra.windDir) {
    result.windDir = String(extra.windDir);
  }

  if (extra.sunrise) {
    result.sunrise = String(extra.sunrise);
  }

  if (extra.sunset) {
    result.sunset = String(extra.sunset);
  }

  const uv = Number(extra.uvIndex);
  if (Number.isFinite(uv)) {
    result.uvIndex = Math.round(uv);
  }

  const precip = Number(extra.precip);
  if (Number.isFinite(precip)) {
    // AppMessage can't carry floats so precip (mm) ships as hundredths
    // (1.25mm -> 125) and the C side divides by 100 to render it
    result.precip = Math.round(precip * 100);
  }

  // scalars already normalized by each provider (feels-like in the user's unit
  // and pressure in hPa and cloud % and gust km/h). finite-guarded so a genuine
  // zero (0% cloud or calm wind) still ships while a missing field drops out
  const feelsLike = Number(extra.feelsLike);
  if (Number.isFinite(feelsLike)) {
    result.feelsLike = Math.round(feelsLike);
  }

  const pressure = Number(extra.pressure);
  if (Number.isFinite(pressure)) {
    result.pressure = Math.round(pressure);
  }

  const cloud = Number(extra.cloud);
  if (Number.isFinite(cloud)) {
    result.cloud = Math.round(cloud);
  }

  const windGustKmh = Number(extra.windGustKmh);
  if (Number.isFinite(windGustKmh)) {
    result.windGustKmh = Math.round(windGustKmh);
  }

  // dew point and the daily high/low ride in the user's temperature unit like feels-like
  const dewPoint = Number(extra.dewPoint);
  if (Number.isFinite(dewPoint)) {
    result.dewPoint = Math.round(dewPoint);
  }

  const tempMax = Number(extra.tempMax);
  if (Number.isFinite(tempMax)) {
    result.tempMax = Math.round(tempMax);
  }

  const tempMin = Number(extra.tempMin);
  if (Number.isFinite(tempMin)) {
    result.tempMin = Math.round(tempMin);
  }

  const precipChance = Number(extra.precipChance);
  if (Number.isFinite(precipChance)) {
    result.precipChance = Math.round(precipChance);
  }

  const uvMax = Number(extra.uvMax);
  if (Number.isFinite(uvMax)) {
    result.uvMax = Math.round(uvMax);
  }

  // today's precip total ships as hundredths like precip to avoid floats in AppMessage
  const precipTotal = Number(extra.precipTotal);
  if (Number.isFinite(precipTotal)) {
    result.precipTotal = Math.round(precipTotal * 100);
  }
}

/**
 * Copies the parsed forecast strips onto a result, skipping missing ones so a
 * provider that can't supply a forecast just leaves the row blank.
 *
 * @param {!Object} result The result being built.
 * @param {?{hourly: ?Object, daily: ?Object}} forecast From openmeteo.parseForecast, or null.
 */
function attachForecast(result, forecast) {
  if (!forecast) {
    return;
  }

  if (forecast.hourly) {
    result.forecastHourly = forecast.hourly;
  }

  if (forecast.daily) {
    result.forecastDaily = forecast.daily;
  }
}

/**
 * Parses a JSON string, returning null on failure.
 *
 * @param {string} body
 * @return {?Object}
 */
function safeParse(body) {
  try {
    return JSON.parse(body);
  } catch (error) {
    return null;
  }
}

/**
 * Performs an HTTP GET and shared response handling.
 *
 * A parseable body always goes to onJson, so a provider can read its own error
 * JSON (e.g. an 'Invalid Key' on a 4xx). If the body does not parse, finish with
 * 'NET ERROR' when the request failed, else 'BAD WX DATA'.
 *
 * @param {string} url
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the finished result
 * @param {function} onJson receives the parsed JSON object
 */
function requestJson(url, request, done, onJson) {
  // cachebust
  const separator = url.indexOf('?') === -1 ? '?' : '&';
  const freshUrl = `${url}${separator}_=${Date.now()}`;

  request(freshUrl, (err, body) => {
    const json = body ? safeParse(body) : null;
    if (json) {
      return onJson(json);
    }

    if (err) {
      return done(status('NET ERROR'));
    }

    return done(status('BAD WX DATA'));
  });
}

/**
 * Builds a successful weather result.
 *
 * The temperature is already in the unit the user selected, so the watch
 * displays it without converting.
 *
 * @param {number} temperature Temperature in the user's chosen unit.
 * @param {string} condition
 * @param {?string} location Display label for the resolved location.
 * @param {number=} lat Latitude the weather was fetched for (GPS or geocoded).
 * @param {number=} lon Longitude the weather was fetched for.
 * @param {Object=} extra Optional {humidity, windKmh, windDir, sunrise, sunset,
 *     uvIndex, precip, feelsLike, pressure, cloud, windGustKmh, dewPoint,
 *     tempMax, tempMin, precipChance, uvMax, precipTotal}.
 * @return {!Object}
 */
function ok(temperature, condition, location, lat, lon, extra) {
  const value = Number(temperature);
  if (!Number.isFinite(value)) {
    // a missing or non-numeric provider field would otherwise round to NaN and ship as a real reading
    return status('No Wx Data');
  }

  const result = {
    temperature: Math.round(value),
    condition: shorten(condition),
    location: location || '',
    ok: true
  };

  // carry the resolved coordinates so the watch can show lat/lon for whatever
  // location the weather actually used (manual place name included) not just
  // the phone GPS path. only attached when both are real numbers
  if (typeof lat === 'number' && typeof lon === 'number') {
    result.lat = lat;
    result.lon = lon;
  }

  attachExtras(result, extra);

  return result;
}

/**
 * Builds a status/error result that carries no live reading.
 *
 * @param {string} text Short message shown on the watch.
 * @return {!Object}
 */
function status(text) {
  return {
    temperature: 0,
    condition: (text || '').toUpperCase(),
    location: '',
    ok: false
  };
}

module.exports = {
  wmoToCondition,
  applyNight,
  shorten,
  degToCompass,
  hmFromIso,
  hmFromUnix,
  hmFrom12Hour,
  attachForecast,
  safeParse,
  requestJson,
  ok,
  status
};
