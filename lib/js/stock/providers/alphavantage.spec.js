/**
 * Deterministic specs for the Alpha Vantage provider.
 *
 * Alpha Vantage answers 200 for everything and every field is a string, so the
 * risky parts are the string coercion, the trailing % on the percent, and the
 * diagnostic bodies it returns for a bad key, a hit cap, or an unknown symbol.
 */

import { describe, test, expect } from 'vitest';
import alphavantage from './alphavantage.js';
import { fetchRequest } from '../../../../testing/fetch-request.js';

/**
 * Stub `request` that records the requested url and replies with a canned body.
 *
 * @param {string} body The response body to feed back.
 * @param {!Array<string>} calls Sink for requested urls, in order.
 * @return {Function}
 */
function replying(body, calls) {
  return (url, callback) => {
    calls.push(url);
    callback(null, body);
  };
}

/** Runs the provider synchronously and returns the result object. */
function run(opts, request) {
  let result;
  alphavantage.fetch(opts, request, (received) => { result = received; });
  return result;
}

const BASE = { key: 'k', symbol: 'IBM' };
const QUOTE_OK = JSON.stringify({
  'Global Quote': {
    '01. symbol': 'IBM',
    '05. price': '290.5000',
    '07. latest trading day': '2026-07-01',
    '08. previous close': '289.0000',
    '09. change': '1.5000',
    '10. change percent': '0.5190%'
  }
});

describe('alphavantage provider', () => {
  describe('guard clauses', () => {
    /** A missing key must short-circuit before any network call. */
    test('reports a missing key without making a request', () => {
      const calls = [];

      const result = run({ ...BASE, key: '' }, replying('{}', calls));

      expect(result.status).toBe('NO API KEY');
      expect(calls).toHaveLength(0);
    });
  });

  describe('request', () => {
    /** The call must use the GLOBAL_QUOTE function and the symbol, or it reads the wrong thing. */
    test('queries the global quote function with the symbol', () => {
      const calls = [];

      run(BASE, replying(QUOTE_OK, calls));

      expect(calls[0]).toContain('function=GLOBAL_QUOTE');
      expect(calls[0]).toContain('symbol=IBM');
      expect(calls[0]).toContain('apikey=k');
    });
  });

  describe('successful reads', () => {
    /** The string fields must coerce to numbers or the watch gets text where it wants a price. */
    test('coerces the string price and change to numbers', () => {
      const result = run(BASE, replying(QUOTE_OK, []));

      expect(result.ok).toBe(true);
      expect(result.price).toBe(290.5);
      expect(result.change).toBe(1.5);
    });

    /** The percent ships with a trailing %, so leaving it on would make parseFloat keep it out of the number. */
    test('strips the trailing percent sign', () => {
      const result = run(BASE, replying(QUOTE_OK, []));

      expect(result.changePercent).toBe(0.519);
    });

    /** The latest trading day is the as-of label, so it must carry through. */
    test('carries the latest trading day as the as-of date', () => {
      const result = run(BASE, replying(QUOTE_OK, []));

      expect(result.asOf).toBe('2026-07-01');
    });
  });

  describe('errors', () => {
    /** An unknown symbol comes back as an empty Global Quote and must read as No Symbol. */
    test('maps an empty Global Quote to No Symbol', () => {
      const result = run(BASE, replying(JSON.stringify({ 'Global Quote': {} }), []));

      expect(result.status).toBe('NO SYMBOL');
    });

    /** The daily cap arrives as an Information note and must read as Rate Limit. */
    test('maps an Information note to Rate Limit', () => {
      const body = JSON.stringify({ Information: 'you have hit your 25 requests per day limit' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** The legacy per-minute cap arrives as a Note and must also read as Rate Limit. */
    test('maps a Note to Rate Limit', () => {
      const body = JSON.stringify({ Note: 'please consider your call frequency' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('RATE LIMIT');
    });

    /** A bad key arrives as an Error Message and must read as Invalid Key. */
    test('maps an Error Message to Invalid Key', () => {
      const body = JSON.stringify({ 'Error Message': 'the apikey parameter is invalid' });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('INVALID KEY');
    });

    /** A limit note must win over a stray quote object so a capped read never looks live. */
    test('checks the diagnostic fields before the quote', () => {
      const body = JSON.stringify({ Information: 'limit', 'Global Quote': { '05. price': '1.0' } });

      const result = run(BASE, replying(body, []));

      expect(result.status).toBe('RATE LIMIT');
    });
  });

  // live integration against the real Alpha Vantage API. opt-in via RUN_LIVE_STOCK=1
  // plus ALPHAVANTAGE_KEY. the free tier is 25 calls a day so keep this to one
  describe.skipIf(process.env.RUN_LIVE_STOCK !== '1' || !process.env.ALPHAVANTAGE_KEY)('live', () => {
    const KEY = process.env.ALPHAVANTAGE_KEY;
    const live = (opts) => new Promise((resolve) => alphavantage.fetch(opts, fetchRequest, resolve));

    /** The real quote path must still parse into a usable reading. */
    test('returns a usable quote for a known symbol', async () => {
      const result = await live({ key: KEY, symbol: 'IBM' });

      expect(result.ok).toBe(true);
      expect(typeof result.price).toBe('number');
    });
  });
});
