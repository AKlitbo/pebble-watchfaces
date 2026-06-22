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
  const provider = PROVIDERS[opts.provider] || openMeteo;

  provider.fetch(opts, request, done);
}

module.exports = { fetchWeather };
