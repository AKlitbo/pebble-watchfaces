/**
 * Shared PebbleKit JS bootstrap for the weather watchfaces.
 *
 * Reads Clay settings, picks the location (GPS or saved city), fetches weather
 * through the chosen provider, and sends it to the watch. The faces only differ
 * in how they format coordinates, so each passes its own formatCoords hook.
 */

// the `any`s that remain in this file sit at two genuinely-dynamic boundaries: the
// Clay-settings readers (getConfig/readValue/readBool/validCoord/isString/isEnum/asBool/
// getManualLocation) that read whatever the user saved to localStorage and the
// face-generated message_keys map plus the AppMessage dict it keys
/* eslint-disable @typescript-eslint/no-explicit-any */
import weather from '../weather/weather';
import weatherUtil from '../weather/util';
import stock from '../stock/stock';
import locationComponent from '../clay/location-component';
import ical from '../calendar/ical';
import wire from './wire';
import schedule from '../stock/schedule';
import stockCache from '../stock/cache';
import type { WeatherResult } from '../weather/util';
import stockUtil from '../stock/util';
import type { StockQuote } from '../stock/util';
import type { ClayConfigItem } from '../clay/types';

/** Fetch state carried across stock rounds, mutated in place by runStockRound. */
interface StockState {
  inFlight: boolean;
  round: number;
  lastFetchMs: number;
  lastAsOf: string;
}

/** The helpers runStockRound needs, passed in so the specs can swap them. */
interface StockDeps {
  fetchQuote: (symbol: string, onQuote: (result: StockQuote) => void) => void;
  sendStocks: (results: StockQuote[]) => void;
  now: () => number;
  timeoutMs: number;
}

/** The options a face hands startPebbleApp. */
interface StartOptions {
  clayConfig: ClayConfigItem[];
  formatCoords: (messageKeys: Record<string, number>, result: WeatherResult) => AppMessageDict;
  components?: unknown[];
  seedKeys?: string[];
  customClay?: unknown;
}

const WEATHER_KEYS = ['WEATHER_PROVIDER', 'WEATHER_API_KEY', 'WEATHER_TEMPERATURE_UNIT', 'LOCATION_USE_GPS', 'LOCATION_GPS_FALLBACK', 'LOCATION_NAME'];

// the stock settings that mean a refetch is worth it after the config closes
const STOCK_KEYS = ['STOCK_PROVIDER', 'STOCK_API_KEY', 'STOCK_SYMBOLS'];

// a ticker is upper letters plus dot and dash (BRK.B and other suffixes) up to 12 long.
// the lookahead demands at least one letter so a punctuation-only entry (a "." or "-") can't
// slip through and burn a provider call on a bogus symbol
const STOCK_SYMBOL_RE = /^(?=.*[A-Z])[A-Z.-]{1,12}$/;

// extra weather readings that map a message key to its field on the provider result
const EXTRA_WEATHER_FIELDS = [
  { key: 'WEATHER_HUMIDITY', field: 'humidity' },
  { key: 'WEATHER_WIND_SPEED', field: 'windKmh' },
  { key: 'WEATHER_WIND_DIR', field: 'windDir' },
  { key: 'WEATHER_SUNRISE', field: 'sunrise' },
  { key: 'WEATHER_SUNSET', field: 'sunset' },
  { key: 'WEATHER_UV_INDEX', field: 'uvIndex' },
  { key: 'WEATHER_PRECIPITATION', field: 'precip' },
  { key: 'WEATHER_FEELS_LIKE', field: 'feelsLike' },
  { key: 'WEATHER_PRESSURE', field: 'pressure' },
  { key: 'WEATHER_CLOUD', field: 'cloud' },
  { key: 'WEATHER_WIND_GUST', field: 'windGustKmh' },
  { key: 'WEATHER_DEW_POINT', field: 'dewPoint' },
  { key: 'WEATHER_TEMP_MAX', field: 'tempMax' },
  { key: 'WEATHER_TEMP_MIN', field: 'tempMin' },
  { key: 'WEATHER_PRECIP_CHANCE', field: 'precipChance' },
  { key: 'WEATHER_PRECIP_TOTAL', field: 'precipTotal' },
  { key: 'WEATHER_UV_MAX', field: 'uvMax' },
];

/**
 * Splits the comma list of tickers into clean uppercase symbols. A junk or
 * over-long entry is dropped, and the list is capped at the strip size.
 */
function parseSymbols(raw: unknown): string[] {
  return String(raw || '')
    .split(',')
    .map((symbol) => symbol.trim().toUpperCase())
    .filter((symbol) => STOCK_SYMBOL_RE.test(symbol))
    .slice(0, wire.STOCK_MAX_SLOTS);
}

// backstop for a stock round whose last quote never calls back: without it the in-flight
// flag would stay stuck and block every future fetch. set above the 15s per-request timeout
// so normal timeouts resolve on their own first
const STOCK_ROUND_TIMEOUT_MS = 30 * 1000;

/**
 * Collects {messageKey: defaultValue} from the Clay config so the same
 * defaults apply whether or not the user has opened the settings page yet.
 */
function collectDefaults(items: ClayConfigItem[]): Record<string, any> {
  return items.reduce((defaults: Record<string, any>, item) => {
    if (item.messageKey && item.defaultValue !== undefined) {
      defaults[item.messageKey] = item.defaultValue;
    }
    if (item.items) {
      Object.assign(defaults, collectDefaults(item.items));
    }
    return defaults;
  }, {});
}

/** Performs an HTTP GET. callback(error, responseText). */
function request(url: string, callback: (err: string | null, body?: string) => void): void {
  const xhr = new XMLHttpRequest();

  // fire the callback exactly once. the pebble app's XHR doesn't reliably honor xhr.timeout, so
  // a request can hang forever with no onload/onerror/ontimeout, which strands the whole fetch
  // (the watch just sits blank). an independent watchdog guarantees the caller always hears back
  let settled = false;
  const finish = (err: string | null, body?: string) => {
    if (settled) {
      return;
    }
    settled = true;
    clearTimeout(watchdog);
    // keep the no-body error paths one-arg like before so a caller reading arity sees no change
    if (body === undefined) {
      callback(err);
    } else {
      callback(err, body);
    }
  };

  const watchdog = setTimeout(() => finish('timeout'), 15000);

  xhr.onload = () => {
    // 2xx is success. otherwise flag an error but still pass the body so a
    // provider can read its own error JSON
    if (xhr.status >= 200 && xhr.status < 300) {
      finish(null, xhr.responseText);
    } else {
      finish('http ' + xhr.status, xhr.responseText);
    }
  };

  xhr.onerror = () => {
    finish('network error');
  };

  xhr.ontimeout = () => {
    finish('timeout');
  };

  xhr.timeout = 15000;

  // a malformed url or a blocked send can throw synchronously, which would otherwise kill the
  // fetch before any result is delivered. treat it as a failed request so the caller recovers
  try {
    xhr.open('GET', url);
    xhr.send();
  } catch (error) {
    finish('send error');
  }
}

/** Reads the persisted Clay settings from localStorage. */
function getConfig(): Record<string, any> {
  try {
    return JSON.parse(localStorage.getItem('clay-settings') as string) || {};
  } catch (error) {
    return {};
  }
}

/** Unwraps a Clay value, applying a fallback for empty values. */
function readValue(value: any, fallback: any): any {
  let result = value;

  if (result && typeof result === 'object' && 'value' in result) {
    result = result.value;
  }

  if (result === undefined || result === null || result === '') {
    return fallback;
  }

  return result;
}

/** Reads a boolean Clay setting, applying a fallback when it is unset. */
function readBool(value: any, fallback: boolean): boolean {
  const result = readValue(value, fallback);
  return result === true || result === 'true' || result === 1 || result === '1';
}

/** Reports whether a coordinate pair is within the valid geographic range. */
function validCoord(lat: any, lon: any): boolean {
  return typeof lat === 'number' && lat >= -90 && lat <= 90 &&
    typeof lon === 'number' && lon >= -180 && lon <= 180;
}

/** Reads the manual location the user picked in settings. */
function getManualLocation(config: any): { coords: { lat: number; lon: number }; label: string } | null {
  const raw = readValue(config.LOCATION_NAME, '');

  if (!raw) {
    return null;
  }

  try {
    let parsed = raw;
    if (typeof raw === 'string') {
      parsed = JSON.parse(raw);
    }

    // skip a corrupt or out-of-range blob instead of sending bad coords on
    if (parsed && validCoord(parsed.lat, parsed.lon)) {
      return {
        coords: { lat: parsed.lat, lon: parsed.lon },
        label: parsed.label || '',
      };
    }
  } catch (error) {
    // fall through on parsing error
  }

  return null;
}

// the watch payload is already sanitized C-side but guard each copy anyway so
// a malformed round-trip can't seed junk into the Clay store
const isString = (value: any) => typeof value === 'string';
const isEnum = (value: any) => typeof value === 'string' || typeof value === 'number';
const asBool = (value: any) => value === 1;

// settings we copy from the watch payload into the Clay store
// accept is an optional type guard and coerce an optional transform (default is copy as-is)
const SEED_FIELDS: Array<{ key: string; accept?: (value: any) => boolean; coerce?: (value: any) => any }> = [
  // temperature unit is a select so it seeds through seedKeys as a "0"/"1" string not here
  { key: 'CLOCK_DATE_FORMAT', accept: isString },
  { key: 'APPEARANCE_THEME', accept: isEnum, coerce: String },
  { key: 'HEALTH_STEPS_MODE', accept: isEnum, coerce: String },
  { key: 'CLOCK_TIME_FORMAT', accept: isEnum, coerce: String },
  { key: 'CONNECTION_BLUETOOTH_ICON', coerce: asBool },
  { key: 'CONNECTION_VIBE_CONNECT', accept: isEnum, coerce: String },
  { key: 'CONNECTION_VIBE_DISCONNECT', accept: isEnum, coerce: String },
];

/**
 * Seeds the Clay store from the watch's current settings so the config opens
 * with the real values instead of defaults. The watch persist is the source of
 * truth, since the phone's clay-settings can be empty or stale after an update.
 */
function seedConfigFromWatch(messageKeys: any, payload: any, seedKeys?: string[]): void {
  const config = getConfig();

  SEED_FIELDS.forEach((field) => {
    const messageKey = messageKeys[field.key];
    if (!(messageKey in payload)) {
      return;
    }

    const value = payload[messageKey];
    if (field.accept && !field.accept(value)) {
      return;
    }

    config[field.key] = field.coerce ? field.coerce(value) : value;
  });

  // extra face keys (like layout rows and goals) are all selects so they
  // seed as the string form Clay expects
  (seedKeys || []).forEach((key) => {
    const messageKey = messageKeys[key];
    if (messageKey in payload && isEnum(payload[messageKey])) {
      config[key] = String(payload[messageKey]);
    }
  });

  localStorage.setItem('clay-settings', JSON.stringify(config));
}

/**
 * Snapshots the settings behind a key list so a later save can be diffed against them. Each
 * value is JSON-encoded so objects compare by content.
 */
function settingsSnapshot(keys: string[]): string[] {
  const config = getConfig();
  return keys.map((key) => JSON.stringify(config[key]));
}

/** Reports whether any setting behind a key list changed between two snapshots. */
function settingsChanged(keys: string[], before: string[] | null, after: string[]): boolean {
  // no snapshot means the page never reported opening so refetch to be safe
  if (!before) {
    return true;
  }

  // walks the key list rather than the snapshot so a short after array cannot cut the diff early
  return keys.some((key, index) => after[index] !== before[index]);
}

/** Snapshots the weather-relevant settings. */
function weatherSettingsSnapshot(): string[] {
  return settingsSnapshot(WEATHER_KEYS);
}

/** Reports whether any weather-relevant setting changed between two snapshots. */
function weatherSettingsChanged(before: string[] | null, after: string[]): boolean {
  return settingsChanged(WEATHER_KEYS, before, after);
}

// how long each failed weather fetch waits before the next try. a cold launch (gps still warming
// and network not up yet) misses the first fetch so a couple of spaced retries let it recover instead
// of sitting blank until the 30-min poll
const WEATHER_RETRY_DELAYS_MS = [5000, 15000];

// how long to wait on a geolocation lookup before giving up and using the fallback. shorter than
// the 15s native timeout since that timeout isn't reliably honored on the pebble app
const GPS_WATCHDOG_MS = 10000;

/**
 * Decides how long to wait before retrying a weather fetch, or null when no retry
 * should run. A successful fetch never retries, and the attempts are capped.
 */
function weatherRetryDelayMs(resultOk: boolean, attempt: number): number | null {
  if (resultOk || attempt >= WEATHER_RETRY_DELAYS_MS.length) {
    return null;
  }

  return WEATHER_RETRY_DELAYS_MS[attempt];
}

/** Snapshots the stock-relevant settings. */
function stockSettingsSnapshot(): string[] {
  return settingsSnapshot(STOCK_KEYS);
}

/** Reports whether any stock-relevant setting changed between two snapshots. */
function stockSettingsChanged(before: string[] | null, after: string[]): boolean {
  return settingsChanged(STOCK_KEYS, before, after);
}

/**
 * Runs one stock-fetch round: fires a quote per symbol, collects them in display
 * order, and sends the packed strip once the last lands.
 *
 * Built so an unfinished fetch can never get stuck and block every fetch after it. A
 * symbol that throws right away counts as a failed quote, a watchdog force-closes a
 * round whose callback never arrives, and a forced round takes over from one that is
 * still running so the old round's late replies can't reopen it or send twice.
 */
function runStockRound(state: StockState, symbols: string[], force: boolean, deps: StockDeps): void {
  // nothing to fetch: never start a round or an empty list would leave the in-flight
  // flag stuck until the watchdog since no per-symbol callback ever fires
  if (!symbols.length) {
    return;
  }

  // a round already running would spend the quota twice if a second trigger (the ready
  // event plus the watch's own STOCK_REQUEST) landed before it finished. a forced fetch (a
  // settings change that may have swapped symbols) takes over instead of being dropped
  if (state.inFlight && !force) {
    return;
  }

  // every slot starts as a failed quote so the array is never sparse. when the watchdog closes a
  // round early the symbols that never answered still pack as one record each, so the count the
  // watch reads always matches the records behind it
  const results: StockQuote[] = symbols.map(() => stockUtil.status('ERR'));
  let pending = symbols.length;
  const round = ++state.round; // a new round number tells any round still running to stop
  state.inFlight = true;

  // closes the round exactly once: bumping the round number again shuts out a late watchdog
  // or a straggling reply then records when the fetch finished and sends whatever quotes arrived
  function finishRound() {
    if (round !== state.round) {
      return; // a newer forced round took over, or this one already closed
    }
    state.round++;
    clearTimeout(watchdog);
    state.inFlight = false;

    // record the finish time only when at least one quote came back good so a round that failed
    // outright (a network blip) can retry on the next poll instead of freezing the
    // watchlist. record the trading day too so Alpha Vantage knows once it holds today
    const firstGood = results.find((quote) => quote && quote.ok);
    if (firstGood) {
      state.lastFetchMs = deps.now();
      state.lastAsOf = firstGood.asOf || '';
    }
    deps.sendStocks(results);
  }

  // if a provider never calls back (a hung request the xhr timeout somehow misses) the
  // in-flight flag would stay stuck and block every future fetch so force the round closed
  // after a cap. packing tolerates the missing slots as failed quotes
  const watchdog = setTimeout(finishRound, deps.timeoutMs);

  symbols.forEach((symbol, index) => {
    function onQuote(result: any) {
      if (round !== state.round) {
        return; // taken over or already closed
      }
      results[index] = result;
      if (--pending === 0) {
        finishRound();
      }
    }

    // a single symbol that throws right away (say a bad URL from odd input) must not
    // strand the whole round with pending stuck above zero so treat it as a failed quote
    // and let the other symbols finish
    try {
      deps.fetchQuote(symbol, onQuote);
    } catch (err) {
      onQuote(stockUtil.status('ERR'));
    }
  });
}

/**
 * Starts the app: builds the Clay settings page, wires the lifecycle listeners,
 * and fetches weather on demand.
 */
function startPebbleApp(options: StartOptions): void {
  const clayConfig = options.clayConfig;
  const formatCoords = options.formatCoords;

  // required here (not at module load) so the unit specs can use the shared
  // helpers without loading Clay or the per-face message_keys alias
  // eslint-disable-next-line @typescript-eslint/no-require-imports
  const Clay = require('@rebble/clay/src/js/index') as ClayConstructor;
  // eslint-disable-next-line @typescript-eslint/no-require-imports
  const messageKeys = require('message_keys');
  // auto-handling off so we own the send path: Clay's own webviewclosed handler sends the settings
  // through Pebble.sendAppMessage directly, which bypasses queueSend and collides with any other
  // in-flight send (the losing message drops with no retry). we open the config and send the saved
  // settings ourselves below so every send goes through the one queue
  const clay = new Clay(clayConfig, options.customClay, { autoHandleEvents: false });

  // register the location autocomplete (referenced as type "locationsearch" in
  // config.ts) before the settings page is built
  clay.registerComponent(locationComponent);

  // any face-specific custom components (e.g. a layout builder)
  (options.components || []).forEach((component) => clay.registerComponent(component));

  // the defaults declared in config.ts
  const DEFAULTS = collectDefaults(clayConfig);

  let weatherSettingsBeforeConfig: string[] | null = null;
  let stockSettingsBeforeConfig: string[] | null = null;
  let calendarUrlBeforeConfig: string | null = null;

  // fetch state carried across stock rounds and mutated in place by runStockRound. the two
  // throttle stamps come back off the phone because this JS is killed and restarted at will and
  // a gate that forgets when it last fetched is no gate at all
  const savedStock = stockCache.load(localStorage);
  const stockState: StockState = { inFlight: false, round: 0, lastFetchMs: savedStock.lastFetchMs, lastAsOf: savedStock.lastAsOf };

  // the last strip worth showing kept across restarts so a watch with an empty store still has
  // something while the gate is shut. this is not the dedupe cache below and must never be one:
  // it says what the watch could show not what it already has
  let savedStrip: number[] | null = savedStock.strip;

  // last strip pushed to the watch so an unchanged refresh skips the redundant BLE wake
  // reset on ready and on a watch-initiated request so the watch always gets a fresh answer
  let lastCalendarBytes: number[] | null = null;
  let lastStockBytes: number[] | null = null;
  let lastWeatherKey: string | null = null;

  // PebbleKit JS only allows one AppMessage in flight, so concurrent sends collide and the loser is
  // silently dropped. at cold boot the settings round-trip, weather, stocks, and calendar all fire
  // at once, so a fetched reading would drop with no retry and leave the face blank until a lone
  // later send (like a settings change) got through. serialize every send through one queue: one at
  // a time, each retried a few times on a nack before it is given up
  const SEND_RETRIES = 3;
  const SEND_RETRY_MS = 250;
  const SEND_WATCHDOG_MS = 8000;
  const sendQueue: Array<{ dict: any; onOk?: () => void; onFail?: () => void; tries: number }> = [];
  let sending = false;

  function pumpSend(): void {
    if (sending || sendQueue.length === 0) {
      return;
    }
    sending = true;
    const item = sendQueue[0];

    // resolve each send exactly once. a lost ack/nack (neither callback ever fires) would otherwise
    // leave sending true forever and wedge the whole queue, so a watchdog counts as a failure
    let settled = false;
    const settle = (ok: boolean): void => {
      if (settled) {
        return;
      }
      settled = true;
      clearTimeout(watchdog);
      sending = false;

      if (ok) {
        sendQueue.shift();
        if (item.onOk) {
          item.onOk();
        }
        pumpSend();
        return;
      }

      // a nack usually just means the outbox was momentarily busy, so back off and retry the same
      // head a few times before dropping it and moving on so one stuck send can't wedge the queue
      item.tries++;
      if (item.tries >= SEND_RETRIES) {
        sendQueue.shift();
        if (item.onFail) {
          item.onFail();
        }
        pumpSend();
      } else {
        setTimeout(pumpSend, SEND_RETRY_MS);
      }
    };

    const watchdog = setTimeout(() => settle(false), SEND_WATCHDOG_MS);
    Pebble.sendAppMessage(item.dict, () => settle(true), () => settle(false));
  }

  /** Queues an AppMessage so sends never overlap and drop. */
  function queueSend(dict: any, onOk?: () => void, onFail?: () => void): void {
    sendQueue.push({ dict: dict, onOk: onOk, onFail: onFail, tries: 0 });
    pumpSend();
  }

  /** Sends a weather result to the watch. */
  function sendWeather(result: any) {
    const dict: Record<string, any> = {
      [messageKeys.WEATHER_TEMPERATURE]: result.temperature,
      [messageKeys.WEATHER_CONDITIONS]: result.condition,
      [messageKeys.WEATHER_OK]: result.ok ? 1 : 0,
    };

    if (result.location && messageKeys.LOCATION_NAME !== undefined) {
      dict[messageKeys.LOCATION_NAME] = result.location;
    }

    // extra readings only sent for faces that declare the keys
    // a missing value is left out so the watch keeps its placeholder
    EXTRA_WEATHER_FIELDS.forEach(({ key, field }) => {
      const value = result[field];
      if (messageKeys[key] !== undefined && value !== undefined && value !== '') {
        dict[messageKeys[key]] = value;
      }
    });

    // the forecast strips ride as packed byte arrays. only for faces that
    // declare the keys and only when the provider actually supplied a strip
    if (messageKeys.WEATHER_FORECAST_HOURLY !== undefined) {
      const bytes = wire.packForecastHourly(result.forecastHourly);
      if (bytes) {
        dict[messageKeys.WEATHER_FORECAST_HOURLY] = bytes;
      }
    }

    if (messageKeys.WEATHER_FORECAST_DAILY !== undefined) {
      const bytes = wire.packForecastDaily(result.forecastDaily);
      if (bytes) {
        dict[messageKeys.WEATHER_FORECAST_DAILY] = bytes;
      }
    }

    Object.assign(dict, formatCoords(messageKeys, result));

    // a compound dict rather than one strip so compare a serialized signature and
    // skip the send when nothing moved since last time
    const key = JSON.stringify(dict);
    if (key === lastWeatherKey) {
      return;
    }
    lastWeatherKey = key;

    queueSend(
      dict,
      () => {
        console.log('Weather info sent to Pebble successfully!');
      },
      () => {
        console.error('Error sending weather info to Pebble!');
      }
    );
  }

  /**
   * Fetches the weather using GPS or the stored manual coordinates, then
   * forwards the result to the watch. A failed fetch retries a few times.
   */
  function getWeather(attempt?: number) {
    const retry = attempt || 0;
    const config = getConfig();

    const opts: any = {
      provider: String(readValue(config.WEATHER_PROVIDER, DEFAULTS.WEATHER_PROVIDER)),
      key: String(readValue(config.WEATHER_API_KEY, DEFAULTS.WEATHER_API_KEY || '')).trim().slice(0, 64),
      fahrenheit: readBool(config.WEATHER_TEMPERATURE_UNIT, DEFAULTS.WEATHER_TEMPERATURE_UNIT),
      coords: null,
      label: undefined,
      // only faces that declare a forecast key pay to fetch the strips. everyone
      // else skips the extra hourly block and the supplemental provider call
      wantForecast: messageKeys.WEATHER_FORECAST_HOURLY !== undefined || messageKeys.WEATHER_FORECAST_DAILY !== undefined,
    };

    const useGps = readBool(config.LOCATION_USE_GPS, DEFAULTS.LOCATION_USE_GPS);
    const gpsFallback = readBool(config.LOCATION_GPS_FALLBACK, DEFAULTS.LOCATION_GPS_FALLBACK);
    const manual = getManualLocation(config);

    // sends the result on and when a fetch failed schedules a bounded retry so a cold-start miss
    // (gps still warming and network not up yet) recovers without waiting out the poll
    const deliver = (result: any) => {
      sendWeather(result);
      const retryMs = weatherRetryDelayMs(result.ok, retry);
      if (retryMs !== null) {
        setTimeout(() => getWeather(retry + 1), retryMs);
      }
    };

    const fetchFor = (coords: any, label: any) => {
      opts.coords = coords;
      opts.label = label;
      console.log(`Fetching Weather: ${opts.provider} @ ${coords.lat},${coords.lon} (${label})`);

      weather.fetchWeather(opts, request, (result: any) => {
        console.log(`Weather: ${result.condition} ${result.temperature} (ok=${result.ok})`);
        deliver(result);
      });
    };

    const useManual = () => {
      if (manual) {
        return fetchFor(manual.coords, manual.label);
      }
      deliver(weatherUtil.status('No Location'));
    };

    // what to do when gps can't place us: the manual city if the user allowed the fallback
    // otherwise a status the panel can show
    const gpsFail = () => {
      if (gpsFallback) {
        useManual();
      } else {
        deliver(weatherUtil.status('No GPS'));
      }
    };

    // skip gps entirely when the user disabled it
    if (!useGps) {
      return useManual();
    }

    // fall back to the manual location if the phone has no geolocation API
    if (typeof navigator === 'undefined' || !navigator.geolocation) {
      return gpsFail();
    }

    // getCurrentPosition on the pebble app can hang and never call back (the timeout option isn't
    // reliably honored) which would strand a fallback user with nothing sent. the watchdog runs
    // the fallback if neither callback lands and the located flag keeps whichever fires first
    let located = false;
    const watchdog = setTimeout(() => {
      if (located) {
        return;
      }
      located = true;
      gpsFail();
    }, GPS_WATCHDOG_MS);

    navigator.geolocation.getCurrentPosition(
      (pos) => {
        if (located) {
          return;
        }
        located = true;
        clearTimeout(watchdog);
        fetchFor({ lat: pos.coords.latitude, lon: pos.coords.longitude }, 'My Location');
      },
      () => {
        if (located) {
          return;
        }
        located = true;
        clearTimeout(watchdog);
        gpsFail();
      },
      // a cold wake has no fresh fix so accept a position up to 10 min old rather than block on a
      // slow new acquisition that can time out. weather barely moves over that window anyway
      { timeout: 15000, maximumAge: 600000 }
    );
  }

  /** Sends already-packed watchlist bytes to the watch, unless the watch holds them already. */
  function pushStockBytes(bytes: number[] | null) {
    if (!bytes) {
      return;
    }

    if (wire.bytesEqual(bytes, lastStockBytes)) {
      return;
    }
    lastStockBytes = bytes;

    queueSend(
      { [messageKeys.STOCK_STRIP]: bytes },
      () => {
        console.log('Stock info sent to Pebble successfully!');
      },
      () => {
        console.error('Error sending stock info to Pebble!');
      }
    );
  }

  /** Sends the packed watchlist strip to the watch, and keeps it for the next run. */
  function sendStocks(results: any[]) {
    const bytes = wire.packStockStrip(results);
    pushStockBytes(bytes);

    // a round where every quote failed would cache a strip of ERRs and show them tomorrow, so
    // only a real reading is worth keeping. runStockRound stamps the throttle from the same first
    // good quote just before it calls this, so the stamps below are already the new ones
    if (bytes && results.some((quote) => quote && quote.ok)) {
      savedStrip = bytes;
      stockCache.save(localStorage, {
        lastAsOf: stockState.lastAsOf,
        lastFetchMs: stockState.lastFetchMs,
        strip: bytes,
      });
    }
  }

  /**
   * Fetches a quote for each configured symbol, then forwards the packed strip
   * to the watch. Skipped entirely for faces that do not show stocks.
   *
   * @return True when a round started, false when there was nothing to fetch or the
   *   provider's quota gate held it back, so the caller knows no strip is coming.
   */
  function getStocks(force?: boolean): boolean {
    // a face that does not declare the strip key never shows stocks so skip the fetch
    if (messageKeys.STOCK_STRIP === undefined) {
      return false;
    }

    const config = getConfig();
    const provider = String(readValue(config.STOCK_PROVIDER, DEFAULTS.STOCK_PROVIDER || 'finnhub'));
    const key = String(readValue(config.STOCK_API_KEY, DEFAULTS.STOCK_API_KEY || '')).trim().slice(0, 64);
    const symbols = parseSymbols(readValue(config.STOCK_SYMBOLS, DEFAULTS.STOCK_SYMBOLS || ''));

    if (!symbols.length) {
      return false;
    }

    // keep the last strip when a poll lands before the provider's data could have moved
    if (schedule.shouldThrottleStockFetch(provider, force || false, stockState.lastFetchMs, stockState.lastAsOf, Date.now())) {
      return false;
    }

    // fetch every symbol and keep the results in the configured order so the watchlist
    // rows stay put. runStockRound stops a fetch from spending the quota twice and recovers
    // on its own if a round gets stuck
    runStockRound(stockState, symbols, Boolean(force), {
      fetchQuote: (symbol, onQuote) => stock.fetchQuote({ provider, key, symbol }, request, onQuote),
      sendStocks: sendStocks,
      now: Date.now,
      timeoutMs: STOCK_ROUND_TIMEOUT_MS,
    });
    return true;
  }

  /** Reads the current iCal feed URL from the Clay config. */
  function calendarUrl(): string {
    return String(readValue(getConfig().CALENDAR_ICS_URL, DEFAULTS.CALENDAR_ICS_URL || '')).trim();
  }

  /**
   * Fetches the iCal feed, parses it, and sends the packed strip to the watch. Skipped for
   * faces that do not declare the calendar key, or when no URL is set.
   */
  function getCalendar() {
    // a face that does not declare the strip key never shows a calendar so skip the fetch
    if (messageKeys.CALENDAR_STRIP === undefined) {
      return;
    }

    const url = calendarUrl();
    if (!url) {
      return;
    }

    // bust any HTTP cache the phone keeps for this GET: a unique query param makes every poll a
    // fresh URL so an edited event is not hidden behind a stale cached copy. Google ignores the
    // extra param and still serves the feed
    const bustedUrl = url + (url.indexOf('?') === -1 ? '?' : '&') + '_=' + Date.now();

    console.log('Calendar: fetching feed');
    request(bustedUrl, (error, body) => {
      if (error) {
        console.error('Calendar fetch failed: ' + error);
        return;
      }

      const events = ical.parseIcal(body as string);
      console.log('Calendar: parsed ' + events.length + ' upcoming event(s)');
      if (events.length) {
        console.log('Calendar: next is "' + events[0].title + '" at ' + new Date(events[0].startEpoch * 1000).toString());
      }

      const bytes = wire.packCalendarStrip(events);
      if (!bytes) {
        return;
      }

      if (wire.bytesEqual(bytes, lastCalendarBytes)) {
        console.log('Calendar: unchanged, skipping send');
        return;
      }
      lastCalendarBytes = bytes;

      queueSend(
        { [messageKeys.CALENDAR_STRIP]: bytes },
        () => { console.log('Calendar sent to Pebble'); },
        () => { console.error('Error sending calendar to Pebble'); }
      );
    });
  }

  // while the JS is alive the phone drives its own refresh since a suspended JS never answers the
  // watch's poll. calendar every tick with weather and stock less often since they change slowly
  const REFRESH_MS = 5 * 60 * 1000;
  const SLOW_REFRESH_EVERY = 6; // weather/stock every ~30 min
  let refreshTimer: ReturnType<typeof setInterval> | null = null;
  let refreshTick = 0;

  function backgroundRefresh() {
    refreshTick++;
    getCalendar();
    if (refreshTick % SLOW_REFRESH_EVERY === 0) {
      getWeather();
      getStocks();
    }
  }

  // app lifecycle listeners
  Pebble.addEventListener('ready', () => {
    console.log('PebbleKit JS Ready!');
    queueSend({ [messageKeys.SETTINGS_REQUEST]: 1 });

    // clear the dedupe cache so a watch that just rebooted with empty stores gets a fresh send
    lastCalendarBytes = null;
    lastStockBytes = null;
    lastWeatherKey = null;

    getWeather();
    // the gate now outlives a restart so it can hold on the very first fetch of a run. a watch
    // that just rebooted with an empty store would sit blank till the gate opened, which for
    // Alpha Vantage is tomorrow, so hand it the strip off the phone instead
    if (!getStocks()) {
      pushStockBytes(savedStrip);
    }
    getCalendar();

    // the timer dies when the JS is suspended so re-arm it fresh on every ready
    if (refreshTimer) {
      clearInterval(refreshTimer);
    }
    refreshTick = 0;
    refreshTimer = setInterval(backgroundRefresh, REFRESH_MS);
  });

  Pebble.addEventListener('appmessage', (event) => {
    const payload: Record<string, number | string> = event.payload || {};

    // the watch only asks when it needs data so clear the dedupe cache to force a fresh send
    if (payload[messageKeys.WEATHER_REQUEST]) {
      lastWeatherKey = null;
      getWeather();
    }

    if (payload[messageKeys.STOCK_REQUEST]) {
      // the watch's poll timer drives this on every interval, so it goes through the provider
      // quota gate like any other routine fetch. only a settings change forces past it
      const held = lastStockBytes;
      lastStockBytes = null;
      if (!getStocks()) {
        // the gate held the fetch back. the watch asks when it has nothing to show, so push the
        // strip already in hand rather than leaving it blank until the gate opens. after a restart
        // nothing is in hand so fall back to the one off the phone
        pushStockBytes(held || savedStrip);
      }
    }

    if (payload[messageKeys.CALENDAR_REQUEST]) {
      lastCalendarBytes = null;
      getCalendar();
    }

    // the watch's reply to our SETTINGS_REQUEST carries its current settings
    if (messageKeys.CLOCK_DATE_FORMAT in payload || messageKeys.APPEARANCE_THEME in payload) {
      // faces that declare SETTINGS_FRESH get the two-way restore: the watch flags when it booted
      // with no saved settings (wiped by an install or update) so we push our own config back
      // instead of letting its defaults seed over ours. faces without the key keep the old seed
      if (messageKeys.SETTINGS_FRESH !== undefined) {
        const config = getConfig();
        const phoneHasConfig = Object.keys(config).length > 0;
        const watchFresh = payload[messageKeys.SETTINGS_FRESH] === 1;

        if (watchFresh && phoneHasConfig) {
          // restore the watch from our saved config using the same dict a Save would send
          queueSend(clay.getSettings(JSON.stringify(config)));
        } else if (!phoneHasConfig) {
          // nothing saved on the phone yet so recover it from the watch instead
          seedConfigFromWatch(messageKeys, payload, options.seedKeys);
        }
        // both sides have settings: the phone is the source of truth and already correct
      } else {
        seedConfigFromWatch(messageKeys, payload, options.seedKeys);
      }
    }
  });

  // Clay saves the new settings in its own webviewclosed handler which runs
  // before ours so capture the previous values while the page is still open
  Pebble.addEventListener('showConfiguration', () => {
    weatherSettingsBeforeConfig = weatherSettingsSnapshot();
    stockSettingsBeforeConfig = stockSettingsSnapshot();
    calendarUrlBeforeConfig = calendarUrl();

    // Clay's auto-handling is off, so open the config page ourselves
    Pebble.openURL(clay.generateUrl());
  });

  Pebble.addEventListener('webviewclosed', (event) => {
    if (!event || !event.response) {
      return;
    }

    // send the saved settings to the watch through the queue. Clay's auto-handling would send this
    // directly and let it collide with an in-flight send (dropping the whole save with no retry)
    queueSend(clay.getSettings(event.response));

    // only refetch when a weather setting actually changed. the C side already
    // re-requests for those so refetching on every save (theme or vibe) is wasted
    const weatherBefore = weatherSettingsBeforeConfig;
    weatherSettingsBeforeConfig = null;

    if (weatherSettingsChanged(weatherBefore, weatherSettingsSnapshot())) {
      // wrap so the timer id is never passed in as the retry count
      setTimeout(() => getWeather(), 250);
    }

    // stocks refetch on their own settings change the same way
    const stockBefore = stockSettingsBeforeConfig;
    stockSettingsBeforeConfig = null;

    if (stockSettingsChanged(stockBefore, stockSettingsSnapshot())) {
      setTimeout(() => getStocks(true), 250);
    }

    // calendar refetches when the iCal URL changed so a new feed shows without waiting for the
    // next poll
    const calendarBefore = calendarUrlBeforeConfig;
    calendarUrlBeforeConfig = null;

    if (calendarBefore !== calendarUrl()) {
      setTimeout(getCalendar, 250);
    }
  });
}

export default {
  startPebbleApp,
  runStockRound,
  parseSymbols,
  collectDefaults,
  request,
  getConfig,
  readValue,
  readBool,
  validCoord,
  getManualLocation,
  seedConfigFromWatch,
  weatherSettingsSnapshot,
  weatherSettingsChanged,
  weatherRetryDelayMs,
  stockSettingsSnapshot,
  stockSettingsChanged,
  WEATHER_KEYS,
  STOCK_KEYS,
};
