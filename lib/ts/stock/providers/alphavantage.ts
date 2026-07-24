/**
 * Alpha Vantage stock quote provider (requires an API key).
 *
 * The free tier is end-of-day data with a 25 requests/day cap, so this is a
 * secondary option. One call per symbol.
 */

import util from '../util';
import type { RequestFn, DoneFn, StockOpts } from '../util';

const ALPHAVANTAGE_API = 'https://www.alphavantage.co/query';

interface AvGlobalQuote {
  '05. price'?: string;
  '09. change'?: string;
  '10. change percent'?: string;
  '07. latest trading day'?: string;
}

/** The subset of an Alpha Vantage GLOBAL_QUOTE response this provider reads. */
interface AlphaVantageResponse {
  'Error Message'?: string;
  Information?: string;
  Note?: string;
  'Global Quote'?: AvGlobalQuote;
}

/** Fetches a quote from Alpha Vantage for the supplied symbol. */
function fetch(opts: StockOpts, request: RequestFn, done: DoneFn): void {
  const start = util.begin(opts, true);
  if (start.error) {
    return done(start.error);
  }

  const symbol = start.symbol;
  const url = `${ALPHAVANTAGE_API}?function=GLOBAL_QUOTE&symbol=${encodeURIComponent(symbol)}&apikey=${start.encodedKey}`;

  util.requestJson<AlphaVantageResponse>(url, request, done, (err, json) => {
    // a bodyless error arrives with no json to read so there's nothing to diagnose
    if (!json) {
      return done(util.status('Net Error'));
    }

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
    const change = parseFloat(String(quote['09. change']));
    // the percent arrives as a string with a trailing % like "0.5190%"
    const changePercent = parseFloat(String(quote['10. change percent']).replace('%', ''));
    const asOf = quote['07. latest trading day'] || '';

    done(util.ok(symbol, price, change, changePercent, asOf));
  });
}

export default { fetch };
