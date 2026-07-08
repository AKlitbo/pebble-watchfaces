/**
 * Specs for the stock provider dispatcher.
 *
 * fetchQuote routes the configured provider name to its module and falls back to
 * finnhub for anything unknown. The provider names are a wire contract with
 * config.js (the STOCK_PROVIDER select values), so a drift between the two
 * silently routes the user to the wrong source.
 *
 * Each provider is identified by the distinct endpoint it requests, so the real
 * dispatch path is exercised end to end rather than mocked.
 */

import { describe, test, expect } from 'vitest';
import stock from './stock.js';

/** The endpoint each provider hits. The fingerprint that tells them apart. */
const ENDPOINT = {
  finnhub: '/api/v1/quote',
  alphavantage: 'function=GLOBAL_QUOTE'
};

/**
 * Stub request that records the requested url, then feeds back an empty body so
 * the routed provider finishes without a network call.
 *
 * @param {!Array<string>} calls Sink for requested urls.
 * @return {Function}
 */
function routing(calls) {
  return (url, callback) => {
    calls.push(url);
    callback(null, '{}');
  };
}

const BASE = { key: 'k', symbol: 'AAPL' };

describe('fetchQuote dispatcher', () => {
  /** Each config provider name must route to its own module, or the user's choice is silently ignored. */
  test.each(['finnhub', 'alphavantage'])('routes provider "%s" to its endpoint', (name) => {
    const calls = [];

    stock.fetchQuote({ ...BASE, provider: name }, routing(calls), () => {});

    expect(calls.length).toBeGreaterThanOrEqual(1);
    expect(calls[0]).toContain(ENDPOINT[name]);
  });

  /** An unknown provider must fall back to finnhub, never leave the quote unfetched. */
  test('falls back to finnhub for an unknown provider', () => {
    const calls = [];

    stock.fetchQuote({ ...BASE, provider: 'definitely-not-a-provider' }, routing(calls), () => {});

    expect(calls[0]).toContain(ENDPOINT.finnhub);
  });

  /** A missing provider (no config yet) must also fall back to finnhub, not crash on undefined. */
  test('falls back to finnhub when no provider is given', () => {
    const calls = [];

    stock.fetchQuote({ ...BASE, provider: undefined }, routing(calls), () => {});

    expect(calls[0]).toContain(ENDPOINT.finnhub);
  });

  /** The dispatcher must hand the provider the symbol it was given, or the request reads the wrong ticker. */
  test('passes the symbol through to the routed provider', () => {
    const calls = [];

    stock.fetchQuote({ ...BASE, provider: 'finnhub', symbol: 'MSFT' }, routing(calls), () => {});

    expect(calls[0]).toContain('symbol=MSFT');
  });
});
