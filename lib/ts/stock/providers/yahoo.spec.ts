/**
 * Deterministic specs for the Yahoo provider.
 *
 * Yahoo needs no key and returns no change field, so the parts worth pinning are
 * the keyless request, the change worked out from the previous close, and the
 * result-null error body an unknown symbol comes back with.
 */

import { describe, test, expect } from 'vitest';
import yahoo from './yahoo';
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
  yahoo.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { symbol: 'AAPL' };
const QUOTE_OK = JSON.stringify({
  chart: {
    result: [
      {
        meta: {
          symbol: 'AAPL',
          currency: 'USD',
          regularMarketPrice: 261.74,
          chartPreviousClose: 260.50,
          regularMarketTime: 1719849600,
        },
      },
    ],
    error: null,
  },
});

describe('yahoo provider', () => {
  describe('guard clauses', () => {
    /** With no symbol there is nothing to look up, and this is the only guard since no key is needed. */
    test('reports No Symbol when the symbol is blank', () => {
      const calls: string[] = [];

      const result = run({ symbol: '' }, replying({}, calls));

      expect(result.status).toBe('NO SYMBOL');
      expect(calls).toHaveLength(0);
    });

    /** A missing key must not stop Yahoo, since the feed is keyless. */
    test('fetches even without a key', () => {
      const calls: string[] = [];

      run({ symbol: 'AAPL' }, replying({ body: QUOTE_OK }, calls));

      expect(calls).toHaveLength(1);
    });
  });

  describe('request', () => {
    /** The chart call must carry the symbol, or it reads the wrong ticker. */
    test('queries the chart endpoint with the symbol', () => {
      const calls: string[] = [];

      run(BASE, replying({ body: QUOTE_OK }, calls));

      expect(calls[0]).toContain('/v8/finance/chart/AAPL');
    });
  });

  describe('successful reads', () => {
    /** The price must come from the documented regularMarketPrice field. */
    test('parses the price from the meta block', () => {
      const result = run(BASE, replying({ body: QUOTE_OK }, []));

      expect(result.ok).toBe(true);
      expect(result.price).toBe(261.74);
    });

    /** Yahoo ships no change field, so it must be worked out from the previous close. */
    test('works out the change and percent from the previous close', () => {
      const result = run(BASE, replying({ body: QUOTE_OK }, []));

      expect(result.change).toBeCloseTo(1.24, 5);
      expect(result.changePercent).toBeCloseTo(0.47601, 4);
    });

    /** The market time is the as-of label, read as the New York trading day. */
    test('carries the market time as the as-of date', () => {
      const result = run(BASE, replying({ body: QUOTE_OK }, []));

      expect(result.asOf).toBe('2024-07-01');
    });
  });

  describe('errors', () => {
    /** An unknown symbol comes back with result null and an error block, and must read as No Symbol. */
    test('maps a result-null error body to No Symbol', () => {
      const body = JSON.stringify({ chart: { result: null, error: { code: 'Not Found', description: 'No data found' } } });

      const result = run(BASE, replying({ body }, []));

      expect(result.status).toBe('NO SYMBOL');
    });

    /** A 429 with a non-JSON body must still read as Rate Limit, not a generic Net Error. */
    test('maps a bodyless 429 to Rate Limit', () => {
      const result = run(BASE, replying({ err: 'http 429', body: 'Too Many Requests' }, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** A non-JSON body with no status must not crash and must report Net Error. */
    test('maps a non-JSON body to Net Error', () => {
      const result = run(BASE, replying({ err: 'http 500', body: '<html>500</html>' }, []));

      expect(result.status).toBe('NET ERROR');
    });
  });

  // live integration against the real Yahoo endpoint. opt-in via RUN_LIVE_STOCK=1.
  // no key is needed so this only guards on the live flag. catches Yahoo changing
  // its response shape or adding auth
  describe.skipIf(process.env.RUN_LIVE_STOCK !== '1')('live', () => {
    const live = (opts: StockOpts) => new Promise<StockQuote>((resolve) => yahoo.fetch(opts, fetchRequest, resolve));

    /** The real quote path must still parse into a usable reading. */
    test('returns a usable quote for a known symbol', async () => {
      const result = await live({ symbol: 'AAPL' });

      expect(result.ok).toBe(true);
      expect(typeof result.price).toBe('number');
      expect(result.price).toBeGreaterThan(0);
    });
  });
});
