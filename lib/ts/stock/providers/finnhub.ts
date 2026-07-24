/**
 * Finnhub stock quote provider (requires an API key).
 *
 * Real-time US quotes on the free tier, so this is the provider used for live
 * polling. One call per symbol.
 */

import util from '../util';
import type { RequestFn, DoneFn, StockOpts } from '../util';

const FINNHUB_QUOTE_API = 'https://finnhub.io/api/v1/quote';

/** The subset of a Finnhub /quote response this provider reads. */
interface FinnhubQuote {
  error?: string;
  c?: number;
  pc?: number;
  d?: number | null;
  dp?: number | null;
  t?: number;
}

/** Fetches a quote from Finnhub for the supplied symbol. */
function fetch(opts: StockOpts, request: RequestFn, done: DoneFn): void {
  const start = util.begin(opts, true);
  if (start.error) {
    return done(start.error);
  }

  const symbol = start.symbol;
  const url = `${FINNHUB_QUOTE_API}?symbol=${encodeURIComponent(symbol)}&token=${start.encodedKey}`;

  util.requestJson<FinnhubQuote>(url, request, done, (err, json) => {
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

    if (!json) {
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
    const derived = util.deriveChange(current, prevClose);
    if (!Number.isFinite(change)) {
      change = derived.change;
    }
    if (!Number.isFinite(changePercent)) {
      changePercent = derived.changePercent;
    }

    done(util.ok(symbol, current, change, changePercent, util.isoDateFromUnix(json.t)));
  });
}

export default { fetch };
