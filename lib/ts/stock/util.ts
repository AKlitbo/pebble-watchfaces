/**
 * Shared helpers for the stock provider modules.
 *
 * Provides the HTTP wrapper and the result builders so finnhub and alphavantage
 * both return the same normalized shape. Kept separate from the weather util on
 * purpose so the two data layers stay decoupled.
 */

/** One normalized quote result (or a status/error when ok is false). */
export interface StockQuote {
  symbol: string;
  price: number;
  change: number;
  changePercent: number;
  asOf: string;
  ok: boolean;
  status?: string;
}

/** The lookup options a face hands the dispatcher and each provider. */
export interface StockOpts {
  provider?: string;
  key?: string;
  symbol?: string;
}

/** An HTTP GET the caller supplies, so the specs can swap in a fake. */
export type RequestFn = (url: string, callback: (err: string | null, body?: string) => void) => void;

/** Called once with the finished quote result. */
export type DoneFn = (result: StockQuote) => void;

/** The ready pieces from begin(), or an error status the caller returns as-is. */
export type BeginResult =
  | { symbol: string; encodedKey: string; error?: undefined }
  | { error: StockQuote; symbol?: undefined; encodedKey?: undefined };

/** Parses a JSON string, returning null on failure. The result is the raw
 * unknown JSON, so a provider casts it to its own quote shape. */
function safeParse(body: string): unknown {
  try {
    return JSON.parse(body);
  } catch (error) {
    return null;
  }
}

/**
 * Performs an HTTP GET and shared response handling.
 *
 * A parseable body goes to onJson together with the request error, so a provider
 * can read both the HTTP status (a 401 key error) and its own error JSON. A
 * bodyless error (an empty or non-JSON 401/429 page) still hands the status to
 * onJson with a null body, so the provider can tell an auth or rate error from a
 * plain network fault. Only a response with neither a readable body nor an error
 * has nothing to act on, so it finishes with 'NET ERROR'.
 */
function requestJson<T = unknown>(url: string, request: RequestFn, done: DoneFn, onJson: (err: string | null, json: T | null) => void): void {
  // cachebust so a proxy can't hand us a stale quote
  const separator = url.indexOf('?') === -1 ? '?' : '&';
  const freshUrl = `${url}${separator}_=${Date.now()}`;

  request(freshUrl, (err, body) => {
    const json = body ? safeParse(body) : null;
    if (json) {
      return onJson(err, json as T);
    }

    // no readable body. an http status still lets a provider read a bodyless
    // 401/429 so pass it through. no status at all is a genuine network fault
    if (err) {
      return onJson(err, null);
    }

    return done(status('NET ERROR'));
  });
}

/**
 * Builds a successful quote result.
 *
 * The numbers are kept as the provider's raw values. Rounding and the scale to
 * integer cents happens at the wire step, not here.
 */
// the numbers arrive straight off a parsed provider payload, so they are untrusted until
// the Number()/isFinite guards below run. saying `number` here would be a lie
function ok(symbol: string, price: unknown, change: unknown, changePercent: unknown, asOf?: string): StockQuote {
  const value = Number(price);
  if (!Number.isFinite(value)) {
    // a missing price would otherwise ship as a real reading of NaN
    return status('NO DATA');
  }

  const abs = Number(change);
  const pct = Number(changePercent);

  return {
    symbol: String(symbol || '').toUpperCase(),
    price: value,
    change: Number.isFinite(abs) ? abs : 0,
    changePercent: Number.isFinite(pct) ? pct : 0,
    asOf: asOf ? String(asOf) : '',
    ok: true,
  };
}

/**
 * Works the change and its percent out from a price and the previous close, for the providers
 * that send no change field of their own. Either one missing leaves both NaN, which ok() maps
 * to a zero change. A previous close of 0 has no meaningful percent so that stays NaN too.
 */
function deriveChange(price: number, prevClose: number): { change: number; changePercent: number } {
  if (!Number.isFinite(price) || !Number.isFinite(prevClose)) {
    return { change: NaN, changePercent: NaN };
  }

  return {
    change: price - prevClose,
    changePercent: prevClose !== 0 ? (price - prevClose) / prevClose * 100 : NaN,
  };
}

/**
 * Runs the shared provider preamble: an optional API-key check, the symbol
 * uppercase and presence check, and URL-encoding the key. Returns the ready
 * pieces, or { error } carrying a status result the caller should return as-is.
 */
function begin(opts: StockOpts, needsKey: boolean): BeginResult {
  if (needsKey && !opts.key) {
    return { error: status('No API Key') };
  }

  const symbol = String(opts.symbol || '').toUpperCase();
  if (!symbol) {
    return { error: status('No Symbol') };
  }

  return { symbol: symbol, encodedKey: opts.key ? encodeURIComponent(opts.key) : '' };
}

/**
 * Turns a unix timestamp in seconds into the US market trading day as "YYYY-MM-DD".
 *
 * US markets quote in New York, so the trading day is the New York date. Reading it in
 * another zone would roll a late after-hours print past 20:00 ET onto the next day.
 */
function isoDateFromUnix(unixSeconds: unknown): string {
  const unix = Number(unixSeconds);
  if (!Number.isFinite(unix) || unix <= 0) {
    return '';
  }

  const date = new Date(unix * 1000);
  try {
    return new Intl.DateTimeFormat('en-CA', {
      timeZone: 'America/New_York',
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
    }).format(date);
  } catch (error) {
    // older engines without IANA time zone data fall back to the plain UTC date
    return date.toISOString().slice(0, 10);
  }
}

/** Builds a status/error result that carries no live reading. */
function status(text: string): StockQuote {
  return {
    symbol: '',
    price: 0,
    change: 0,
    changePercent: 0,
    asOf: '',
    ok: false,
    status: (text || '').toUpperCase(),
  };
}

export default {
  safeParse,
  requestJson,
  begin,
  isoDateFromUnix,
  deriveChange,
  ok,
  status,
};
