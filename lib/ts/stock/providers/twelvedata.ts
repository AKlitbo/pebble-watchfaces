/**
 * Twelve Data stock quote provider (requires an API key).
 *
 * The free tier is 800 calls a day across global markets, so this is the option
 * for non-US tickers. Change and percent come back ready to use. One call per
 * symbol.
 */

import util from '../util';
import type { RequestFn, DoneFn, StockOpts } from '../util';

const TWELVEDATA_QUOTE_API = 'https://api.twelvedata.com/quote';

/** The subset of a Twelve Data /quote response this provider reads. */
interface TwelveDataQuote {
  status?: string;
  code?: number;
  message?: string;
  close?: string;
  change?: string;
  percent_change?: string;
  symbol?: string;
  datetime?: string;
}

/** Fetches a quote from Twelve Data for the supplied symbol. */
function fetch(opts: StockOpts, request: RequestFn, done: DoneFn): void {
  const start = util.begin(opts, true);
  if (start.error) {
    return done(start.error);
  }

  const symbol = start.symbol;
  const url = `${TWELVEDATA_QUOTE_API}?symbol=${encodeURIComponent(symbol)}&apikey=${start.encodedKey}`;

  util.requestJson<TwelveDataQuote>(url, request, done, (err, json) => {
    // a bodyless error arrives with no json to read so there's nothing to diagnose
    if (!json) {
      return done(util.status('Net Error'));
    }

    // Twelve Data answers 200 for a good read and puts the trouble in a
    // status:error body so check that before the quote. a hit cap wins over a
    // bad key so a throttled read never reads as a token problem
    if (json.status === 'error' || json.code) {
      const message = String(json.message || '').toLowerCase();
      const code = Number(json.code);
      if (code === 429 || message.indexOf('limit') !== -1 || message.indexOf('credits') !== -1) {
        return done(util.status('Rate Limit'));
      }
      if (code === 404 || message.indexOf('not found') !== -1) {
        return done(util.status('No Symbol'));
      }
      return done(util.status('Invalid Key'));
    }

    // a good read always carries the close so its absence means there is no quote
    if (json.close === undefined) {
      return done(util.status('No Symbol'));
    }

    const price = parseFloat(json.close);
    const change = parseFloat(String(json.change));
    const changePercent = parseFloat(String(json.percent_change));
    // datetime is the bar date or a full timestamp intraday so keep the day part
    const asOf = String(json.datetime || '').slice(0, 10);

    done(util.ok(json.symbol || symbol, price, change, changePercent, asOf));
  });
}

export default { fetch };
