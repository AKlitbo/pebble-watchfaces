/**
 * Stock quote dispatcher.
 *
 * Routes a request to the configured provider module. Each provider returns its
 * result through `done` as {symbol, price, change, changePercent, asOf, ok}.
 */

import finnhub from './providers/finnhub';
import alphavantage from './providers/alphavantage';
import yahoo from './providers/yahoo';
import twelvedata from './providers/twelvedata';
import type { RequestFn, DoneFn, StockOpts } from './util';

/** A provider module: the one fetch entry the dispatcher calls. */
interface StockProvider {
  fetch: (opts: StockOpts, request: RequestFn, done: DoneFn) => void;
}

// provider name -> module
const PROVIDERS: Record<string, StockProvider> = {
  finnhub: finnhub,
  alphavantage: alphavantage,
  yahoo: yahoo,
  twelvedata: twelvedata,
};

/** Looks up a quote for one symbol using the configured provider. */
function fetchQuote(opts: StockOpts, request: RequestFn, done: DoneFn): void {
  const key = String(opts.provider || '').toLowerCase();
  const provider = PROVIDERS[key];
  if (!provider) {
    // an unknown provider would quietly fall back to finnhub and hide the mistake so log it
    console.log(`Unknown stock provider "${opts.provider}", using finnhub`);
  }

  (provider || finnhub).fetch(opts, request, done);
}

export default { fetchQuote };
