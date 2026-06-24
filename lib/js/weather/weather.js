/**
 * Weather lookup dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns
 * its result through `done` as {temperature, condition, location, ok}, already
 * formatted to the user's unit preference.
 */
'use strict';

const openMeteo = require('./providers/openmeteo');
const owm = require('./providers/owm');
const weatherApi = require('./providers/weatherapi');

/** @const {!Object<string, {fetch: !Function}>} */
const PROVIDERS = {
  openmeteo: openMeteo,
  owm: owm,
  weatherapi: weatherApi
};

/**
 * Looks up current weather using the configured provider and location.
 *
 * @param {{provider: string, key: string, place: string, fahrenheit: boolean, coords: ?{lat: number, lon: number}, label: (string|undefined)}} opts
 * @param {function(string, function(?string, string=))} request
 * @param {function(!Object)} done
 */
function fetchWeather(opts, request, done) {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unrecognized provider would silently use open-meteo and hide the misconfig, so log it
    console.log(`Unknown weather provider "${opts.provider}", using open-meteo`);
  }

  (provider || openMeteo).fetch(opts, request, done);
}

module.exports = { fetchWeather };
