/**
 * Shared helpers for the stock provider modules.
 *
 * Provides the HTTP wrapper and the result builders so finnhub and alphavantage
 * both return the same normalized shape. Kept separate from the weather util on
 * purpose so the two data layers stay decoupled.
 */
'use strict';

/**
 * Parses a JSON string, returning null on failure.
 *
 * @param {string} body The response text.
 * @return {?Object} The parsed object, or null when it does not parse.
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
 * A parseable body always goes to onJson together with the request error, so a
 * provider can read both the HTTP status (a 401 key error) and its own error
 * JSON. When the body does not parse at all there is nothing to read, so finish
 * with 'NET ERROR'.
 *
 * @param {string} url The request URL.
 * @param {function} request HTTP GET (url, callback).
 * @param {function} done Called with a finished status result on a hard failure.
 * @param {function} onJson Receives (err, json) when the body parsed.
 */
function requestJson(url, request, done, onJson) {
  // cachebust so a proxy can't hand us a stale quote
  const separator = url.indexOf('?') === -1 ? '?' : '&';
  const freshUrl = `${url}${separator}_=${Date.now()}`;

  request(freshUrl, (err, body) => {
    const json = body ? safeParse(body) : null;
    if (json) {
      return onJson(err, json);
    }

    return done(status('NET ERROR'));
  });
}

/**
 * Builds a successful quote result.
 *
 * The numbers are kept as the provider's raw values. Rounding and the scale to
 * integer cents happens at the wire step, not here.
 *
 * @param {string} symbol The ticker, echoed and uppercased.
 * @param {number} price The last price in the provider's currency.
 * @param {number} change The absolute change against the previous close, signed.
 * @param {number} changePercent The percent change, signed.
 * @param {string=} asOf The latest trading day or time, '' when unknown.
 * @return {!Object}
 */
function ok(symbol, price, change, changePercent, asOf) {
  const value = Number(price);
  if (!Number.isFinite(value)) {
    // a missing price would otherwise ship as a real reading of NaN
    return status('NO DATA');
  }

  const abs = Number(change);
  const pct = Number(changePercent);

  return {
    symbol: String(symbol || '').toUpperCase(),
    price: value,
    change: Number.isFinite(abs) ? abs : 0,
    changePercent: Number.isFinite(pct) ? pct : 0,
    asOf: asOf ? String(asOf) : '',
    ok: true
  };
}

/**
 * Builds a status/error result that carries no live reading.
 *
 * @param {string} text Short message shown on the watch.
 * @return {!Object}
 */
function status(text) {
  return {
    symbol: '',
    price: 0,
    change: 0,
    changePercent: 0,
    asOf: '',
    ok: false,
    status: (text || '').toUpperCase()
  };
}

module.exports = {
  safeParse,
  requestJson,
  ok,
  status
};
