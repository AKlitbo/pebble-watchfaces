// @vitest-environment jsdom
/**
 * Specs for the shared PebbleKit JS bootstrap.
 *
 * This module is the only copy of the weather/settings glue both faces run, so
 * the hardening it carries (HTTP-status handling, untrusted-payload guards,
 * coordinate range checks, and the change-gated refetch) is tested here once.
 *
 * The webview globals (localStorage) come from jsdom. XMLHttpRequest is stubbed
 * so nothing touches the network. Clay and the per-face `message_keys` alias are
 * only loaded inside startPebbleApp, so the exported helpers test without them.
 * seedConfigFromWatch takes its message-key map as an argument.
 */

import { describe, test, expect, beforeEach, vi } from 'vitest';
import app from './app';
import type { StockQuote } from '../stock/util';

// stands in for the per-face generated message_keys: each key name maps to
// itself so payloads and stored config use readable string keys
const messageKeys = new Proxy({}, { get: (_target, prop) => prop });

/**
 * Captures every XMLHttpRequest the code opens and lets a spec drive the
 * response, an error, or a timeout.
 * @return {Array}
 */
function installFakeXhr() {
  const sent: FakeXhr[] = [];

  class FakeXhr {
    method = '';
    url = '';
    status = 0;
    responseText = '';
    // app.ts hangs these on the request, so the fake has to model them
    onload?: () => void;
    onerror?: () => void;
    ontimeout?: () => void;

    open(method: string, url: string) {
      this.method = method;
      this.url = url;
    }

    send() {
      sent.push(this);
    }

    respond(httpStatus: number, body: string) {
      this.status = httpStatus;
      this.responseText = body;
      this.onload?.();
    }

    fail() {
      this.onerror?.();
    }

    timeOut() {
      this.ontimeout?.();
    }
  }

  global.XMLHttpRequest = FakeXhr as unknown as typeof XMLHttpRequest;
  return sent;
}

describe('request', () => {
  /** A successful response must reach the callback as data with no error. */
  test('reports a 2xx response as success with the body', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].respond(200, '{"ok":1}');

    expect(callback).toHaveBeenCalledWith(null, '{"ok":1}');
  });

  /** A non-2xx must be flagged as an error while still forwarding the body so a provider can read a structured error. */
  test('reports a non-2xx as an http error but still forwards the body', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].respond(404, '{"cod":404}');

    expect(callback).toHaveBeenCalledWith('http 404', '{"cod":404}');
  });

  /** A transport failure must surface as a network error, never a silent success. */
  test('reports a transport failure as a network error', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].fail();

    expect(callback).toHaveBeenCalledWith('network error');
  });

  /** A timeout must surface distinctly so the caller can show a clear status. */
  test('reports a timeout', () => {
    const sent = installFakeXhr();
    const callback = vi.fn();

    app.request('https://example', callback);
    sent[0].timeOut();

    expect(callback).toHaveBeenCalledWith('timeout');
  });
});

describe('validCoord', () => {
  /** An in-range pair is the only thing that should be forwarded to a provider. */
  test('accepts an in-range coordinate pair', () => {
    const result = app.validCoord(33.4, -112);

    expect(result).toBe(true);
  });

  /** Out-of-range or non-numeric coordinates must be rejected before they reach the network. */
  test.each([
    ['latitude too high', 91, 0],
    ['latitude too low', -91, 0],
    ['longitude too high', 0, 181],
    ['longitude too low', 0, -181],
    ['non-numeric latitude', '33', -112],
    ['missing longitude', 33, undefined],
  ])('rejects %s', (label, lat, lon) => {
    const result = app.validCoord(lat, lon);

    expect(result).toBe(false);
  });
});

describe('getManualLocation', () => {
  /** A valid saved place must yield its coordinates and label so the watch fetches the right city. */
  test('returns coordinates and label for an in-range saved location', () => {
    const config = { LOCATION_NAME: JSON.stringify({ lat: 33.4, lon: -112, label: 'Phoenix' }) };

    const result = app.getManualLocation(config);

    expect(result).toEqual({ coords: { lat: 33.4, lon: -112 }, label: 'Phoenix' });
  });

  /** An out-of-range blob must be treated as no manual location, never forwarded verbatim. */
  test('rejects an out-of-range saved coordinate', () => {
    const config = { LOCATION_NAME: JSON.stringify({ lat: 999, lon: -112, label: 'Bad' }) };

    const result = app.getManualLocation(config);

    expect(result).toBeNull();
  });

  /** Malformed JSON must not throw and must fall back to no location. */
  test('returns null for a malformed saved value', () => {
    const config = { LOCATION_NAME: 'not json' };

    const result = app.getManualLocation(config);

    expect(result).toBeNull();
  });

  /** An unset location must be null so the caller falls back to GPS or a no-location status. */
  test('returns null when no location is saved', () => {
    const result = app.getManualLocation({});

    expect(result).toBeNull();
  });
});

describe('seedConfigFromWatch', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /** Reads the value seedConfigFromWatch persisted under a given key. */
  function stored(key: string) {
    return JSON.parse(localStorage.getItem('clay-settings'))[key];
  }

  /** A valid date format string from the watch must seed the config so it opens with the real value. */
  test('copies a valid string CLOCK_DATE_FORMAT into the store', () => {
    app.seedConfigFromWatch(messageKeys, { CLOCK_DATE_FORMAT: '%Y.%m.%d' });

    const result = stored('CLOCK_DATE_FORMAT');

    expect(result).toBe('%Y.%m.%d');
  });

  /** A non-string CLOCK_DATE_FORMAT is corrupt and must be skipped, not seeded as junk. */
  test('skips a non-string CLOCK_DATE_FORMAT', () => {
    app.seedConfigFromWatch(messageKeys, { CLOCK_DATE_FORMAT: 42 });

    const result = 'CLOCK_DATE_FORMAT' in JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toBe(false);
  });

  /** A numeric enum (the on-wire form) must be stringified to match Clay's option values. */
  test('stringifies a numeric enum value', () => {
    app.seedConfigFromWatch(messageKeys, { APPEARANCE_THEME: 3 });

    const result = stored('APPEARANCE_THEME');

    expect(result).toBe('3');
  });

  /** A non-primitive enum is malformed and must be skipped. */
  test('skips an enum value that is neither string nor number', () => {
    app.seedConfigFromWatch(messageKeys, { APPEARANCE_THEME: { nested: true } });

    const result = 'APPEARANCE_THEME' in JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toBe(false);
  });

  /** Temperature unit is a select, so it seeds through seedKeys as the "0"/"1" string Clay expects. */
  test.each([
    [1, '1'],
    [0, '0'],
  ])('seeds WEATHER_TEMPERATURE_UNIT %s as the string "%s" via seedKeys', (value, expected) => {
    app.seedConfigFromWatch(messageKeys, { WEATHER_TEMPERATURE_UNIT: value }, ['WEATHER_TEMPERATURE_UNIT']);

    const result = stored('WEATHER_TEMPERATURE_UNIT');

    expect(result).toBe(expected);
  });

  /** A dropped field (or wrong coercion) in the seed table silently stops seeding that setting, so its config page opens on the default. */
  test('seeds every supported field from a full payload', () => {
    app.seedConfigFromWatch(messageKeys, {
      WEATHER_TEMPERATURE_UNIT: 1,
      CLOCK_DATE_FORMAT: '%a %d %b',
      APPEARANCE_THEME: 2,
      HEALTH_STEPS_MODE: 1,
      CLOCK_TIME_FORMAT: 0,
      CONNECTION_BLUETOOTH_ICON: 0,
      CONNECTION_VIBE_CONNECT: 3,
      CONNECTION_VIBE_DISCONNECT: 1,
    }, ['WEATHER_TEMPERATURE_UNIT']);

    const result = JSON.parse(localStorage.getItem('clay-settings'));

    expect(result).toEqual({
      WEATHER_TEMPERATURE_UNIT: '1',
      CLOCK_DATE_FORMAT: '%a %d %b',
      APPEARANCE_THEME: '2',
      HEALTH_STEPS_MODE: '1',
      CLOCK_TIME_FORMAT: '0',
      CONNECTION_BLUETOOTH_ICON: false,
      CONNECTION_VIBE_CONNECT: '3',
      CONNECTION_VIBE_DISCONNECT: '1',
    });
  });
});

describe('weatherSettingsSnapshot', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /** The snapshot must JSON-encode the stored weather keys so a later save can be diffed by content. */
  test('json-encodes the weather keys read from the store', () => {
    localStorage.setItem('clay-settings', JSON.stringify({ WEATHER_PROVIDER: 'owm' }));

    const result = app.weatherSettingsSnapshot();

    expect(result[app.WEATHER_KEYS.indexOf('WEATHER_PROVIDER')]).toBe('"owm"');
  });
});

describe('weatherSettingsChanged', () => {
  /** With no snapshot (the page never reported opening) the safe default is to refetch. */
  test('returns true when there is no prior snapshot', () => {
    const result = app.weatherSettingsChanged(null, ['"metric"']);

    expect(result).toBe(true);
  });

  /** Identical snapshots mean only non-weather settings changed, so no refetch. */
  test('returns false when every weather key is unchanged', () => {
    const before = app.WEATHER_KEYS.map(() => '"same"');

    const result = app.weatherSettingsChanged(before, before.slice());

    expect(result).toBe(false);
  });

  /** A single differing weather key must trigger a refetch. */
  test('returns true when a weather key differs', () => {
    const before = app.WEATHER_KEYS.map(() => '"a"');
    const after = before.slice();
    after[0] = '"b"';

    const result = app.weatherSettingsChanged(before, after);

    expect(result).toBe(true);
  });
});

describe('weatherRetryDelayMs', () => {
  /** A successful fetch that still scheduled a retry would re-poll the provider for no reason. */
  test('returns null when the fetch succeeded', () => {
    const result = app.weatherRetryDelayMs(true, 0);

    expect(result).toBe(null);
  });

  /** Without the first retry a cold-launch miss sits blank until the 30-min poll. */
  test('returns the first delay when the first attempt failed', () => {
    const result = app.weatherRetryDelayMs(false, 0);

    expect(result).toBe(5000);
  });

  /** The second attempt must back off further so a still-warming gps gets more time. */
  test('returns the longer delay for the second attempt', () => {
    const result = app.weatherRetryDelayMs(false, 1);

    expect(result).toBe(15000);
  });

  /** Uncapped retries would loop forever on a genuinely bad key or a dead feed. */
  test('returns null once the attempts are used up', () => {
    const result = app.weatherRetryDelayMs(false, 2);

    expect(result).toBe(null);
  });
});

describe('parseSymbols', () => {
  /** Whitespace and case sloppiness in the setting must clean up or the watch gets a bad symbol. */
  test('trims and uppercases each symbol', () => {
    const result = app.parseSymbols('aapl, msft ,  tsla');

    expect(result).toEqual(['AAPL', 'MSFT', 'TSLA']);
  });

  /** A junk entry (spaces, digits, too long) must be dropped so it never reaches the provider. */
  test('drops entries that are not valid tickers', () => {
    const result = app.parseSymbols('AAPL, 123, ,TOOLONGSYMBOLXX, BRK.B');

    expect(result).toEqual(['AAPL', 'BRK.B']);
  });

  /** A punctuation-only entry has no letter, so it must be dropped before it wastes a provider call. */
  test('drops a punctuation-only entry', () => {
    const result = app.parseSymbols('AAPL, ., -, ...');

    expect(result).toEqual(['AAPL']);
  });

  /** More than the strip can hold must be capped so the wire never overruns the store. */
  test('caps the list at the strip size', () => {
    const result = app.parseSymbols('A,B,C,D,E,F');

    expect(result).toHaveLength(4);
  });
});

describe('runStockRound', () => {
  // a fresh cross-round state for each test. it is the object getStocks carries in the closure
  function freshState() {
    return { inFlight: false, round: 0, lastFetchMs: 0, lastAsOf: '' };
  }

  /** A symbol whose fetch throws right away must not leave the in-flight flag stuck, or the watchlist freezes for the whole JS session (the 0.10 freeze). */
  test('clears the in-flight flag when a symbol fetch throws right away', () => {
    const state = freshState();
    const sendStocks = vi.fn();
    const deps = {
      fetchQuote: () => { throw new Error('bad url'); },
      sendStocks, now: () => 100, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL', 'MSFT'], false, deps);

    expect(state.inFlight).toBe(false);
    expect(sendStocks).toHaveBeenCalledTimes(1);
  });

  /** If a provider never calls back the watchdog must clear the in-flight flag, otherwise every future fetch is dropped at the in-flight guard. */
  test('clears the in-flight flag via the watchdog when a quote never calls back', () => {
    vi.useFakeTimers();
    const state = freshState();
    const sendStocks = vi.fn();
    const deps = {
      fetchQuote: () => {}, // never invokes onQuote
      sendStocks, now: () => 100, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL'], false, deps);
    expect(state.inFlight).toBe(true);
    vi.advanceTimersByTime(1000);

    expect(state.inFlight).toBe(false);
    expect(sendStocks).toHaveBeenCalledTimes(1);
    vi.useRealTimers();
  });

  /** A second unforced trigger mid-round (ready plus the watch's STOCK_REQUEST) must be dropped, or the round double-spends the provider quota. */
  test('ignores a second unforced round while one is in flight', () => {
    vi.useFakeTimers();
    const state = freshState();
    const started: Array<(q: StockQuote) => void> = [];
    const deps = {
      fetchQuote: (symbol: string, onQuote: (q: StockQuote) => void) => started.push(onQuote), // hold the callbacks open
      sendStocks: vi.fn(), now: () => 100, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL'], false, deps);
    app.runStockRound(state, ['AAPL'], false, deps);

    expect(started).toHaveLength(1);
    vi.useRealTimers();
  });

  /** A forced fetch after a settings change must take over a running round, and the old round's late callback must not send stale symbols to the watch. */
  test('lets a forced round take over one still running and ignores the stale callback', () => {
    vi.useFakeTimers();
    const state = freshState();
    const sendStocks = vi.fn();
    const callbacks: Array<(q: StockQuote) => void> = [];
    const deps = {
      fetchQuote: (symbol: string, onQuote: (q: StockQuote) => void) => callbacks.push(onQuote),
      sendStocks, now: () => 100, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL'], false, deps);
    app.runStockRound(state, ['AAPL'], true, deps);
    callbacks[0]({ ok: true, symbol: 'AAPL', price: 1, change: 0, changePercent: 0, asOf: '' });
    const supersededSend = sendStocks.mock.calls.length;
    callbacks[1]({ ok: true, symbol: 'AAPL', price: 2, change: 0, changePercent: 0, asOf: '' });

    expect(supersededSend).toBe(0);
    expect(sendStocks).toHaveBeenCalledTimes(1);
    expect(state.inFlight).toBe(false);
    vi.useRealTimers();
  });

  /** The throttle time is set from the first good quote, so a later poll knows data was already captured and does not spend quota again. */
  test('records the throttle time from the first good quote', () => {
    const state = freshState();
    const deps = {
      fetchQuote: (symbol: string, onQuote: (q: StockQuote) => void) => onQuote({ ok: true, symbol: symbol, price: 1, change: 0, changePercent: 0, asOf: '2026-07-01' }),
      sendStocks: vi.fn(), now: () => 4242, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL'], false, deps);

    expect(state.lastFetchMs).toBe(4242);
    expect(state.lastAsOf).toBe('2026-07-01');
  });

  /** An all-failed round must leave the gate unstamped so the next poll retries instead of freezing the watchlist on a transient error. */
  test('does not stamp the gate when every quote failed', () => {
    const state = freshState();
    const deps = {
      fetchQuote: (symbol: string, onQuote: (q: StockQuote) => void) => onQuote({ ok: false, symbol: '', status: 'RATE LIMIT', price: 0, change: 0, changePercent: 0, asOf: '' }),
      sendStocks: vi.fn(), now: () => 4242, timeoutMs: 1000,
    };

    app.runStockRound(state, ['AAPL'], false, deps);

    expect(state.lastFetchMs).toBe(0);
    expect(state.inFlight).toBe(false);
  });
});

describe('collectDefaults', () => {
  /** The defaults seed the store before the config page opens, so a dropped pair opens a setting on nothing. */
  test('collects every messageKey with a default, recursing into nested items', () => {
    const items = [
      { type: 'select', messageKey: 'WEATHER_PROVIDER', defaultValue: 'openmeteo' },
      { type: 'section', items: [{ type: 'slider', messageKey: 'STOCK_POLL', defaultValue: 5 }] },
    ];

    const result = app.collectDefaults(items);

    expect(result).toEqual({ WEATHER_PROVIDER: 'openmeteo', STOCK_POLL: 5 });
  });

  /** A heading (no messageKey) or a keyed item with no default must be skipped, not seeded as junk. */
  test('skips items with no messageKey or no default', () => {
    const items = [
      { type: 'heading', defaultValue: 'Some Heading' },
      { type: 'input', messageKey: 'STOCK_API_KEY' },
    ];

    const result = app.collectDefaults(items);

    expect(result).toEqual({});
  });
});

describe('getConfig', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /** The persisted settings must parse back out or every read falls through to defaults. */
  test('returns the parsed clay settings', () => {
    localStorage.setItem('clay-settings', JSON.stringify({ WEATHER_PROVIDER: 'owm' }));

    const result = app.getConfig();

    expect(result).toEqual({ WEATHER_PROVIDER: 'owm' });
  });

  /** With nothing saved the store must read as an empty object, never null that a caller dots into. */
  test('returns an empty object when nothing is saved', () => {
    const result = app.getConfig();

    expect(result).toEqual({});
  });

  /** A corrupt blob must not throw: it falls back to empty so the app still starts. */
  test('returns an empty object on corrupt json', () => {
    localStorage.setItem('clay-settings', '{ not valid');

    const result = app.getConfig();

    expect(result).toEqual({});
  });
});

describe('readValue', () => {
  /** A plain stored value must pass straight through. */
  test('returns a primitive value unchanged', () => {
    const result = app.readValue('finnhub', 'fallback');

    expect(result).toBe('finnhub');
  });

  /** Clay wraps some values as {value}, so the wrapper must be unwrapped or the setting reads as an object. */
  test('unwraps a Clay value wrapper', () => {
    const result = app.readValue({ value: '3' }, 'fallback');

    expect(result).toBe('3');
  });

  /** An empty string, null, or undefined must take the fallback so a blank setting uses its default. */
  test.each([
    ['empty string', ''],
    ['null', null],
    ['undefined', undefined],
  ])('falls back on an empty value (%s)', (label, value) => {
    const result = app.readValue(value, 'fallback');

    expect(result).toBe('fallback');
  });

  /** Zero is a real reading, not an empty value, so it must not be swallowed by the fallback. */
  test('keeps a zero value rather than taking the fallback', () => {
    const result = app.readValue(0, 5);

    expect(result).toBe(0);
  });
});

describe('readBool', () => {
  /** The truthy wire forms (true, "true", 1, "1") must all read as true or a toggle silently stays off. */
  test.each([true, 'true', 1, '1'])('reads %s as true', (value) => {
    const result = app.readBool(value, false);

    expect(result).toBe(true);
  });

  /** Anything else, including the string "0", must read as false so an off toggle stays off. */
  test.each([false, 'false', 0, '0'])('reads %s as false', (value) => {
    const result = app.readBool(value, true);

    expect(result).toBe(false);
  });

  /** An unset setting must take the fallback so a defaulted-on toggle starts on. */
  test('applies the fallback when the value is unset', () => {
    const result = app.readBool(undefined, true);

    expect(result).toBe(true);
  });
});

describe('stockSettingsSnapshot', () => {
  beforeEach(() => {
    localStorage.clear();
  });

  /** The snapshot must JSON-encode the stored stock keys so a later save can be diffed by content. */
  test('json-encodes the stock keys read from the store', () => {
    localStorage.setItem('clay-settings', JSON.stringify({ STOCK_PROVIDER: 'finnhub' }));

    const result = app.stockSettingsSnapshot();

    expect(result[app.STOCK_KEYS.indexOf('STOCK_PROVIDER')]).toBe('"finnhub"');
  });
});

describe('stockSettingsChanged', () => {
  /** With no snapshot (the page never reported opening) the safe default is to refetch. */
  test('returns true when there is no prior snapshot', () => {
    const result = app.stockSettingsChanged(null, ['"finnhub"']);

    expect(result).toBe(true);
  });

  /** Identical snapshots mean only non-stock settings changed, so no refetch. */
  test('returns false when every stock key is unchanged', () => {
    const before = app.STOCK_KEYS.map(() => '"same"');

    const result = app.stockSettingsChanged(before, before.slice());

    expect(result).toBe(false);
  });

  /** A single differing stock key must trigger a refetch. */
  test('returns true when a stock key differs', () => {
    const before = app.STOCK_KEYS.map(() => '"a"');
    const after = before.slice();
    after[0] = '"b"';

    const result = app.stockSettingsChanged(before, after);

    expect(result).toBe(true);
  });
});
