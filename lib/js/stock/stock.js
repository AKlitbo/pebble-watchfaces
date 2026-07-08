/**
 * Stock quote dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns its
 * result through `done` as {symbol, price, change, changePercent, asOf, ok}.
 */
'use strict';

const finnhub = require('./providers/finnhub');
const alphavantage = require('./providers/alphavantage');

// provider name -> module
const PROVIDERS = {
  finnhub: finnhub,
  alphavantage: alphavantage
};

/**
 * Looks up a quote for one symbol using the configured provider.
 *
 * @param {Object} opts provider, key, symbol
 * @param {function} request HTTP GET (url, callback)
 * @param {function} done Called with the quote result.
 */
function fetchQuote(opts, request, done) {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unknown provider would quietly fall back to finnhub and hide the mistake so log it
    console.log(`Unknown stock provider "${opts.provider}", using finnhub`);
  }

  (provider || finnhub).fetch(opts, request, done);
}

module.exports = { fetchQuote };
