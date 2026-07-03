/**
 * Open-Meteo weather provider (free, no API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.js), so this module never geocodes.
 */
'use strict';

const util = require('../util');

const OPEN_METEO_FORECAST_API = 'https://api.open-meteo.com/v1/forecast';

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
  const daily = 'sunrise,sunset,temperature_2m_max,temperature_2m_min,precipitation_probability_max,' +
    'precipitation_sum,uv_index_max';
  const url = `${OPEN_METEO_FORECAST_API}?latitude=${lat}&longitude=${lon}&current=${current}` +
    `&daily=${daily}&temperature_unit=${unit}&wind_speed_unit=kmh&timezone=auto`;

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

    done(util.ok(
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
    ));
  });
}

module.exports = { fetch, parseExtras };
