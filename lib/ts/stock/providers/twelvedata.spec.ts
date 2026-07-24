/**
 * Deterministic specs for the Twelve Data provider.
 *
 * Twelve Data answers 200 for everything and every number is a string, so the
 * risky parts are the string coercion and the status:error bodies it returns for
 * a bad key, a hit cap, or an unknown symbol.
 */

import { describe, test, expect } from 'vitest';
import twelvedata from './twelvedata';
import { fetchRequest } from '../../../testing/fetch-request';
import type { RequestFn, StockOpts, StockQuote } from '../util';

/**
 * Stub `request` that records the requested url and replies with a canned body.
 *
 * @param {string} body The response body to feed back.
 * @param {!Array<string>} calls Sink for requested urls, in order.
 * @return {Function}
 */
function replying(body: string, calls: string[]): RequestFn {
  return (url, callback) => {
    calls.push(url);
    callback(null, body);
  };
}

/** Runs the provider synchronously and returns the result object. */
function run(opts: StockOpts, request: RequestFn): StockQuote {
  let result!: StockQuote;
  twelvedata.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { key: 'k', symbol: 'AAPL' };
const QUOTE_OK = JSON.stringify({
  symbol: 'AAPL',
  exchange: 'NASDAQ',
  currency: 'USD',
  datetime: '2026-07-01',
  close: '261.74',
  previous_close: '260.50',
  change: '1.24',
  percent_change: '0.47601',
});

describe('twelvedata provider', () => {
  describe('guard clauses', () => {
    /** A missing key must short-circuit before any network call. */
    test('reports a missing key without making a request', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, key: '' }, replying('{}', calls));

      expect(result.status).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });

    /** With no symbol there is nothing to look up. */
    test('reports No Symbol when the symbol is blank', () => {
      const calls: string[] = [];

      const result = run({ ...BASE, symbol: '' }, replying('{}', calls));

      expect(result.status).toBe('NO SYMBOL');
      expect(calls).toHaveLength(0);
    });
  });

  describe('request', () => {
    /** The quote call must carry the symbol and key, or it reads the wrong ticker. */
    test('queries the quote endpoint with the symbol and key', () => {
      const calls: string[] = [];

      run(BASE, replying(QUOTE_OK, calls));

      expect(calls[0]).toContain('api.twelvedata.com/quote');
      expect(calls[0]).toContain('symbol=AAPL');
      expect(calls[0]).toContain('apikey=k');
    });
  });

  describe('successful reads', () => {
    /** The string fields must coerce to numbers or the watch gets text where it wants a price. */
    test('coerces the string price, change and percent to numbers', () => {
      const result = run(BASE, replying(QUOTE_OK, []));

      expect(result.ok).toBe(true);
      expect(result.price).toBe(261.74);
      expect(result.change).toBe(1.24);
      expect(result.changePercent).toBe(0.47601);
    });

    /** The datetime is the as-of label, kept to its day part. */
    test('carries the datetime day as the as-of date', () => {
      const body = JSON.stringify({ symbol: 'AAPL', close: '261.74', change: '1.24', percent_change: '0.47', datetime: '2026-07-01 15:59:00' });

      const result = run(BASE, replying(body, []));

      expect(result.asOf).toBe('2026-07-01');
    });
  });

  describe('errors', () => {
    /** An unknown symbol comes back as a 404 status:error and must read as No Symbol. */
    test('maps a 404 error body to No Symbol', () => {
      const body = JSON.stringify({ code: 404, message: '**symbol** GARBAGE not found', status: 'error' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('NO SYMBOL');
    });

    /** A run-out of API credits arrives as a 429 and must read as Rate Limit. */
    test('maps a 429 credit error to Rate Limit', () => {
      const body = JSON.stringify({ code: 429, message: 'You have run out of API credits for the current minute', status: 'error' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** A bad key arrives as a 401 status:error and must read as Invalid Key. */
    test('maps a 401 error body to Invalid Key', () => {
      const body = JSON.stringify({ code: 401, message: 'Invalid API key', status: 'error' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('INVALID KEY');
    });

    /** A hit cap must win over the No Symbol check so a throttled read never looks empty. */
    test('checks the error body before the quote', () => {
      const body = JSON.stringify({ code: 429, message: 'out of API credits', status: 'error', close: '1.0' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('RATE LIMIT');
    });
  });

  // live integration against the real Twelve Data API. opt-in via RUN_LIVE_STOCK=1
  // plus TWELVEDATA_KEY. the free tier is 800 calls a day so keep this to one
  describe.skipIf(process.env.RUN_LIVE_STOCK !== '1' || !process.env.TWELVEDATA_KEY)('live', () => {
    const KEY = process.env.TWELVEDATA_KEY;
    const live = (opts: StockOpts) => new Promise<StockQuote>((resolve) => twelvedata.fetch(opts, fetchRequest, resolve));

    /** The real quote path must still parse into a usable reading. */
    test('returns a usable quote for a known symbol', async () => {
      const result = await live({ key: KEY, symbol: 'AAPL' });

      expect(result.ok).toBe(true);
      expect(typeof result.price).toBe('number');
      expect(result.price).toBeGreaterThan(0);
    });
  });
});
