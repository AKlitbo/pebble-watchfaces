/**
 * Specs for the shared stock helpers.
 *
 * ok() and status() are the normalized shape every provider returns, and
 * requestJson() is the seam that decides whether a body reaches the provider or
 * short-circuits to an error. A drift here breaks every provider at once.
 */

import { describe, test, expect } from 'vitest';
import util from './util.js';

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

describe('stock util requestJson', () => {
  /** A body that is not JSON has nothing a provider can read, so it must map to Net Error. */
  test('reports Net Error when the body does not parse', () => {
    let result;
    const request = (url, callback) => callback('http 500', 'not json');

    util.requestJson('https://x', request, (status) => { result = status; }, () => {});

    expect(result.status).toBe('NET ERROR');
  });

  /** The provider needs both the HTTP error and the parsed body to tell a bad key from a bad symbol. */
  test('hands the error and parsed body to onJson together', () => {
    let seenErr;
    let seenJson;
    const request = (url, callback) => callback('http 401', '{"error":"bad"}');

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
    const request = (url, callback) => { requested = url; callback(null, '{}'); };

    util.requestJson('https://x?symbol=AAPL', request, () => {}, () => {});

    expect(requested).toMatch(/[?&]_=\d+/);
  });
});
