/**
 * OpenWeatherMap weather provider (requires an API key).
 *
 * Takes resolved coordinates only. Place-name geocoding happens once at
 * settings time (see clay/location-component.js), so this module never geocodes.
 */
'use strict';

const util = require('../util');
const openMeteo = require('./openmeteo');

const OWM_WEATHER_API = 'https://api.openweathermap.org/data/2.5/weather';

/**
 * Fetches current weather from OpenWeatherMap for the supplied coordinates.
 *
 * @param {Object} opts key, fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetch(opts, request, done) {
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

  // OWM's free endpoint is missing UV and dew point and any daily forecast
  // so a parallel Open-Meteo request grabs them. temperature_unit keeps
  // dew point and the daily high and low in the unit the user picked
  const omUnit = opts.fahrenheit ? 'fahrenheit' : 'celsius';
  const omDaily = 'temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum,uv_index_max';
  const omUrl = `https://api.open-meteo.com/v1/forecast?latitude=${lat}&longitude=${lon}` +
    `&current=uv_index,dew_point_2m&daily=${omDaily}&temperature_unit=${omUnit}&timezone=auto`;
  let pending = 2;
  let owmJson = null;
  let owmErrStatus = null;
  let omExtra = {};

  const tryDone = () => {
    if (--pending > 0) {
      return;
    }

    if (owmErrStatus) {
      return done(owmErrStatus);
    }

    const json = owmJson;

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
    const wind = json.wind || {};
    const windKmh = Number.isFinite(Number(wind.speed))
      ? Number(wind.speed) * (opts.fahrenheit ? 1.609344 : 3.6)
      : undefined;

    const sys = json.sys || {};
    const rain = json.rain || {};
    const snow = json.snow || {};
    const clouds = json.clouds || {};
    const precip = (rain['1h'] || snow['1h'] || 0);

    // gust uses the same unit as wind.speed (m/s metric or mph imperial) so normalize to km/h
    const windGustKmh = Number.isFinite(Number(wind.gust))
      ? Number(wind.gust) * (opts.fahrenheit ? 1.609344 : 3.6)
      : undefined;

    // OWM lumps every cloud level under main "Clouds". split partly (icon code
    // 02) from overcast (03/04) using the icon code before night promotion
    let cond = json.weather[0].main;
    if (String(cond).toLowerCase() === 'clouds') {
      cond = icon.slice(0, 2) === '02' ? 'PCLDY' : 'CLDY';
    }

    done(util.ok(
      json.main.temp,
      util.applyNight(cond, isDay),
      opts.label || json.name,
      lat,
      lon,
      Object.assign({
        humidity: json.main.humidity,
        windKmh: windKmh,
        windDir: util.degToCompass(wind.deg),
        precip: precip,
        feelsLike: json.main.feels_like,
        pressure: json.main.pressure,
        cloud: clouds.all,
        windGustKmh: windGustKmh,
        sunrise: util.hmFromUnix(sys.sunrise, json.timezone),
        sunset: util.hmFromUnix(sys.sunset, json.timezone)
        // uv and dew point and the daily forecast from the parallel Open-Meteo call
      }, omExtra)
    ));
  };

  // 1. Fetch OWM
  util.requestJson(url, request, (status) => {
    owmErrStatus = status;
    tryDone();
  }, (json) => {
    owmJson = json;
    tryDone();
  });

  // 2. Fetch Open-Meteo (ignore failures). parseExtras keeps the
  // Open-Meteo field mapping shared with the openmeteo provider
  util.requestJson(omUrl, request, () => {
    tryDone();
  }, (json) => {
    omExtra = openMeteo.parseExtras(json);
    tryDone();
  });
}

module.exports = { fetch };
