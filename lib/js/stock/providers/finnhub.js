/**
 * Finnhub stock quote provider (requires an API key).
 *
 * Real-time US quotes on the free tier, so this is the provider used for live
 * polling. One call per symbol.
 */
'use strict';

const util = require('../util');

const FINNHUB_QUOTE_API = 'https://finnhub.io/api/v1/quote';

/**
 * Turns a unix timestamp in seconds into a plain "YYYY-MM-DD" date.
 *
 * @param {*} unixSeconds The quote time from Finnhub.
 * @return {string} The date, or '' when the value is not a usable number.
 */
function isoDateFromUnix(unixSeconds) {
  const unix = Number(unixSeconds);
  if (!Number.isFinite(unix) || unix <= 0) {
    return '';
  }

  return new Date(unix * 1000).toISOString().slice(0, 10);
}

/**
 * Fetches a quote from Finnhub for the supplied symbol.
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
  const url = `${FINNHUB_QUOTE_API}?symbol=${encodeURIComponent(symbol)}&token=${encodedKey}`;

  util.requestJson(url, request, done, (err, json) => {
    // Finnhub uses the HTTP status for auth and rate errors. the body is a
    // small {"error":"..."} either way so the status is what we branch on
    if (err) {
      if (String(err).indexOf('401') !== -1) {
        return done(util.status('Invalid Key'));
      }
      if (String(err).indexOf('429') !== -1) {
        return done(util.status('Rate Limit'));
      }
      return done(util.status('Net Error'));
    }

    // some request wrappers hand back a 200 with the trouble in the body instead
    // of an HTTP status so read the error text too. a mention of a limit is the
    // rate cap otherwise treat it as a bad key
    if (json.error) {
      const message = String(json.error).toLowerCase();
      return done(util.status(message.indexOf('limit') !== -1 ? 'Rate Limit' : 'Invalid Key'));
    }

    // an unknown symbol comes back all zeros. no current and no previous close
    // means there is nothing to show
    const current = Number(json.c);
    const prevClose = Number(json.pc);
    if (current === 0 && prevClose === 0) {
      return done(util.status('No Symbol'));
    }

    // d and dp can be null right at the open before a print exists. fall back to
    // working the change out from the current and previous close. Number(null) is
    // 0 not NaN so map a missing field to NaN first or the fallback never fires
    let change = (json.d === null || json.d === undefined) ? NaN : Number(json.d);
    let changePercent = (json.dp === null || json.dp === undefined) ? NaN : Number(json.dp);
    if (!Number.isFinite(change) && Number.isFinite(prevClose)) {
      change = current - prevClose;
    }
    if (!Number.isFinite(changePercent) && Number.isFinite(prevClose) && prevClose !== 0) {
      changePercent = (current - prevClose) / prevClose * 100;
    }

    done(util.ok(symbol, current, change, changePercent, isoDateFromUnix(json.t)));
  });
}

module.exports = { fetch, isoDateFromUnix };
