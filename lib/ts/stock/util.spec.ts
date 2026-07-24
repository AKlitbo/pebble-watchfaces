/**
 * Specs for the shared stock helpers.
 *
 * ok() and status() are the normalized shape every provider returns, and
 * requestJson() is the seam that decides whether a body reaches the provider or
 * short-circuits to an error. A drift here breaks every provider at once.
 */

import { describe, test, expect } from 'vitest';
import util from './util';
import type { RequestFn } from './util';

describe('stock util ok', () => {
  /** A non-finite price would ship as a real reading of NaN and draw a garbage number. */
  test('rejects a non-finite price as No Data', () => {
    const result = util.ok('AAPL', 'not-a-number', 1, 1, '2026-07-01');

    expect(result.ok).toBe(false);
    expect(result.status).toBe('NO DATA');
  });

  /** The symbol must come back uppercased or the watch header shows a lowercase ticker. */
  test('echoes the symbol uppercased', () => {
    const result = util.ok('aapl', 261.74, 1.24, 0.47, '2026-07-01');

    expect(result.symbol).toBe('AAPL');
  });

  /** Raw prices must survive unrounded so the wire step owns the scale to cents. */
  test('keeps the price and change as their raw values', () => {
    const result = util.ok('AAPL', 261.74, 1.24, 0.4744, '2026-07-01');

    expect(result.price).toBe(261.74);
    expect(result.change).toBe(1.24);
    expect(result.changePercent).toBe(0.4744);
  });

  /** A missing change must settle to 0, not NaN, so a valid price still ships. */
  test('defaults a non-finite change to zero', () => {
    const result = util.ok('AAPL', 261.74, null, undefined, '');

    expect(result.change).toBe(0);
    expect(result.changePercent).toBe(0);
    expect(result.ok).toBe(true);
  });
});

describe('stock util status', () => {
  /** The watch shows this text verbatim, so a lowercase message reads wrong. */
  test('uppercases the message', () => {
    const result = util.status('Rate Limit');

    expect(result.status).toBe('RATE LIMIT');
    expect(result.ok).toBe(false);
  });
});

describe('stock util begin', () => {
  /** A key-required provider must short-circuit before building a URL, or it fires a keyless request that 401s. */
  test('reports No API Key when a key is required but missing', () => {
    const result = util.begin({ key: '', symbol: 'AAPL' }, true);

    expect(result.error.status).toBe('NO API KEY');
  });

  /** A keyless provider (Yahoo) must skip the key check, or it would refuse to run at all. */
  test('skips the key check when needsKey is false', () => {
    const result = util.begin({ symbol: 'AAPL' }, false);

    expect(result.error).toBeUndefined();
    expect(result.symbol).toBe('AAPL');
  });

  /** A missing symbol must short-circuit, or the provider queries the API with an empty ticker. */
  test('reports No Symbol when the symbol is missing', () => {
    const result = util.begin({ key: 'k' }, true);

    expect(result.error.status).toBe('NO SYMBOL');
  });

  /** The symbol must come back uppercased and the key URL-encoded, or the request URL is malformed. */
  test('returns the uppercased symbol and the encoded key', () => {
    const result = util.begin({ key: 'a b/c', symbol: 'aapl' }, true);

    expect(result.symbol).toBe('AAPL');
    expect(result.encodedKey).toBe('a%20b%2Fc');
  });
});

describe('stock util isoDateFromUnix', () => {
  /** The trading day is the New York date, so a UTC-midnight time reads as the day before. */
  test('formats a unix time as the New York trading day', () => {
    const result = util.isoDateFromUnix(86400);

    expect(result).toBe('1970-01-01');
  });

  /** A zero or missing timestamp means no known day, so it must be blank not 1970. */
  test('returns blank for a zero timestamp', () => {
    const result = util.isoDateFromUnix(0);

    expect(result).toBe('');
  });
});

describe('stock util requestJson', () => {
  /** A bodyless http error (an empty or non-JSON 401/429) must still reach the provider so it can read the status, not short-circuit to Net Error. */
  test('hands a bodyless http error to onJson with a null body', () => {
    let seenErr;
    let seenJson;
    const request: RequestFn = (url, callback) => callback('http 401', 'not json');

    util.requestJson('https://x', request, () => {}, (err, json) => {
      seenErr = err;
      seenJson = json;
    });

    expect(seenErr).toBe('http 401');
    expect(seenJson).toBeNull();
  });

  /** With neither a readable body nor an error there is nothing to act on, so it must map to Net Error. */
  test('reports Net Error when there is no body and no error', () => {
    let result;
    const request: RequestFn = (url, callback) => callback(null, '');

    util.requestJson('https://x', request, (status) => { result = status; }, () => {});

    expect(result.status).toBe('NET ERROR');
  });

  /** The provider needs both the HTTP error and the parsed body to tell a bad key from a bad symbol. */
  test('hands the error and parsed body to onJson together', () => {
    let seenErr;
    let seenJson;
    const request: RequestFn = (url, callback) => callback('http 401', '{"error":"bad"}');

    util.requestJson('https://x', request, () => {}, (err, json) => {
      seenErr = err;
      seenJson = json;
    });

    expect(seenErr).toBe('http 401');
    expect(seenJson).toEqual({ error: 'bad' });
  });

  /** Without a cachebust param a proxy can serve a stale quote forever. */
  test('appends a cachebust param to the url', () => {
    let requested;
    const request: RequestFn = (url, callback) => { requested = url; callback(null, '{}'); };

    util.requestJson('https://x?symbol=AAPL', request, () => {}, () => {});

    expect(requested).toMatch(/[?&]_=\d+/);
  });
});
