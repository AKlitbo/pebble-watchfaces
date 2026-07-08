/**
 * Weather lookup dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns
 * its result through `done` as {temperature, condition, location, ok}, already
 * formatted to the user's unit preference.
 */
'use strict';

const util = require('./util');
const openMeteo = require('./providers/openmeteo');
const owm = require('./providers/owm');
const weatherApi = require('./providers/weatherapi');

// provider name -> module
const PROVIDERS = {
  openmeteo: openMeteo,
  owm: owm,
  weatherapi: weatherApi
};

/**
 * Looks up current weather using the configured provider and location.
 *
 * @param {Object} opts provider, key, place, fahrenheit, coords, label
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done called with the weather result
 */
function fetchWeather(opts, request, done) {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unknown provider would quietly fall back to open-meteo and hide the mistake so log it
    console.log(`Unknown weather provider "${opts.provider}", using open-meteo`);
  }

  (provider || openMeteo).fetch(opts, request, (result) => {
    // the openmeteo provider already attached the forecast off its own response.
    // any other provider gets a supplemental open-meteo call to fill the strip
    // the same way owm borrows the extras. a failed reading is left untouched
    if (!result || !result.ok || result.forecastHourly) {
      return done(result);
    }

    openMeteo.fetchForecast(opts, request, (forecast) => {
      util.attachForecast(result, forecast);
      done(result);
    });
  });
}

module.exports = { fetchWeather };
