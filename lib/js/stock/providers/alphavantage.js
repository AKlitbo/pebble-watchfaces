/**
 * Alpha Vantage stock quote provider (requires an API key).
 *
 * The free tier is end-of-day data with a 25 requests/day cap, so this is a
 * secondary option. One call per symbol.
 */
'use strict';

const util = require('../util');

const ALPHAVANTAGE_API = 'https://www.alphavantage.co/query';

/**
 * Fetches a quote from Alpha Vantage for the supplied symbol.
 *
 * @param {Object} opts key, symbol
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done Called with the quote result.
 */
function fetch(opts, request, done) {
  if (!opts.key) {
    return done(util.status('No API Key'));
  }

  const symbol = String(opts.symbol || '').toUpperCase();
  if (!symbol) {
    return done(util.status('No Symbol'));
  }

  const encodedKey = encodeURIComponent(opts.key);
  const url = `${ALPHAVANTAGE_API}?function=GLOBAL_QUOTE&symbol=${encodeURIComponent(symbol)}&apikey=${encodedKey}`;

  util.requestJson(url, request, done, (err, json) => {
    // Alpha Vantage always answers 200 and puts the trouble in the body. check
    // the diagnostic fields before the quote so a bad key or a hit cap does not
    // read as an empty quote
    if (json['Error Message']) {
      return done(util.status('Invalid Key'));
    }
    if (json.Information || json.Note) {
      return done(util.status('Rate Limit'));
    }

    // an unknown symbol comes back as an empty Global Quote object
    const quote = json['Global Quote'];
    if (!quote || quote['05. price'] === undefined) {
      return done(util.status('No Symbol'));
    }

    const price = parseFloat(quote['05. price']);
    const change = parseFloat(quote['09. change']);
    // the percent arrives as a string with a trailing % like "0.5190%"
    const changePercent = parseFloat(String(quote['10. change percent']).replace('%', ''));
    const asOf = quote['07. latest trading day'] || '';

    done(util.ok(symbol, price, change, changePercent, asOf));
  });
}

module.exports = { fetch };
