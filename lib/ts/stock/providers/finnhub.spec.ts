/**
 * Deterministic specs for the Finnhub provider.
 *
 * The provider makes one quote call per symbol. The injected `request` is
 * stubbed and every call is recorded so the URL and the error mapping can be
 * asserted. The all-zero body and the null change-at-open path are the two
 * shapes Finnhub really returns, so both are pinned here.
 */

import { describe, test, expect } from 'vitest';
import finnhub from './finnhub';
import { fetchRequest } from '../../../testing/fetch-request';
import type { RequestFn, StockOpts, StockQuote } from '../util';

/**
 * Stub `request` that records the requested url and replies with a canned
 * response.
 *
 * @param {{err?: ?string, body?: string}} response The error and body to feed back.
 * @param {!Array<string>} calls Sink for requested urls, in order.
 * @return {Function}
 */
function replying(response: { err?: string | null; body?: string }, calls: string[]): RequestFn {
  return (url, callback) => {
    calls.push(url);
    callback(response.err || null, response.body);
  };
}

/** Runs the provider synchronously and returns the result object. */
function run(opts: StockOpts, request: RequestFn): StockQuote {
  let result!: StockQuote;
  finnhub.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { key: 'k', symbol: 'AAPL' };
const QUOTE_OK = JSON.stringify({ c: 261.74, d: 1.24, dp: 0.4744, pc: 260.50, t: 1719849600 });

describe('finnhub provider', () => {
  describe('guard clauses', () => {
    /** A missing key must short-circuit before any network call. */
    test('reports a missing key without making a request', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, key: '' }, replying({}, calls));

      expect(result.status).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });

    /** With no symbol there is nothing to look up. */
    test('reports No Symbol when the symbol is blank', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, symbol: '' }, replying({}, calls));

      expect(result.status).toBe('NO SYMBOL');
      expect(calls).toHaveLength(0);
    });
  });

  describe('request', () => {
    /** The quote call must carry the symbol and token, or it reads the wrong ticker. */
    test('queries the quote endpoint with the symbol and token', () => {
      const calls: string[] = [];

      run(BASE, replying({ body: QUOTE_OK }, calls));

      expect(calls[0]).toContain('/api/v1/quote');
      expect(calls[0]).toContain('symbol=AAPL');
      expect(calls[0]).toContain('token=k');
    });
  });

  describe('successful reads', () => {
    /** Price, change, and percent must come from the documented c/d/dp fields. */
    test('parses the price, change and percent', () => {
      const result = run(BASE, replying({ body: QUOTE_OK }, []));

      expect(result.ok).toBe(true);
      expect(result.price).toBe(261.74);
      expect(result.change).toBe(1.24);
      expect(result.changePercent).toBe(0.4744);
    });

    /** Right at the open d and dp arrive null, so the change must fall back to c minus pc. */
    test('works out the change from c and pc when d and dp are null', () => {
      const body = JSON.stringify({ c: 110, d: null, dp: null, pc: 100, t: 1719849600 });

      const result = run(BASE, replying({ body }, []));

      expect(result.change).toBe(10);
      expect(result.changePercent).toBe(10);
    });
  });

  describe('errors', () => {
    /** An unknown symbol comes back all zeros and must read as No Symbol, not a $0 quote. */
    test('maps an all-zero body to No Symbol', () => {
      const body = JSON.stringify({ c: 0, d: null, dp: null, pc: 0, t: 0 });

      const result = run(BASE, replying({ body }, []));

      expect(result.status).toBe('NO SYMBOL');
    });

    /** A 401 must surface as Invalid Key so the user knows to fix the token. */
    test('maps a 401 to Invalid Key', () => {
      const result = run(BASE, replying({ err: 'http 401', body: '{"error":"Invalid API key"}' }, []));

      expect(result.status).toBe('INVALID KEY');
    });

    /** A 429 must surface as Rate Limit rather than a generic error. */
    test('maps a 429 to Rate Limit', () => {
      const result = run(BASE, replying({ err: 'http 429', body: '{"error":"API limit reached"}' }, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** Some wrappers return a 200 with the error in the body, so a limit message must still read as Rate Limit. */
    test('maps a limit message in a 200 body to Rate Limit', () => {
      const result = run(BASE, replying({ err: null, body: '{"error":"API limit reached. Try again later."}' }, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** A key error in a 200 body must read as Invalid Key, not fall through to a NaN quote. */
    test('maps a non-limit error in a 200 body to Invalid Key', () => {
      const result = run(BASE, replying({ err: null, body: '{"error":"Invalid API key"}' }, []));

      expect(result.status).toBe('INVALID KEY');
    });

    /** A non-JSON body must not crash and must report Net Error. */
    test('maps a non-JSON body to Net Error', () => {
      const result = run(BASE, replying({ err: 'http 500', body: '<html>500</html>' }, []));

      expect(result.status).toBe('NET ERROR');
    });

    /** A bodyless 401 (empty auth-error page) must still read as Invalid Key, not a generic Net Error. */
    test('maps a bodyless 401 to Invalid Key', () => {
      const result = run(BASE, replying({ err: 'http 401', body: '' }, []));

      expect(result.status).toBe('INVALID KEY');
    });

    /** A bodyless 429 (empty rate-limit page) must still read as Rate Limit. */
    test('maps a bodyless 429 to Rate Limit', () => {
      const result = run(BASE, replying({ err: 'http 429', body: '' }, []));

      expect(result.status).toBe('RATE LIMIT');
    });
  });

  // live integration against the real Finnhub API. opt-in via RUN_LIVE_STOCK=1 plus
  // FINNHUB_KEY. catches the upstream changing its response shape
  describe.skipIf(process.env.RUN_LIVE_STOCK !== '1' || !process.env.FINNHUB_KEY)('live', () => {
    const KEY = process.env.FINNHUB_KEY;
    const live = (opts: StockOpts) => new Promise<StockQuote>((resolve) => finnhub.fetch(opts, fetchRequest, resolve));

    /** The real quote path must still parse into a usable reading. */
    test('returns a usable quote for a known symbol', async () => {
      const result = await live({ key: KEY, symbol: 'AAPL' });

      expect(result.ok).toBe(true);
      expect(typeof result.price).toBe('number');
      expect(result.price).toBeGreaterThan(0);
    });
  });
});
