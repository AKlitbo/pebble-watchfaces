/**
 * Shared PebbleKit JS bootstrap for the weather watchfaces.
 *
 * Reads Clay settings, picks the location (GPS or saved city), fetches weather
 * through the chosen provider, and sends it to the watch. The faces only differ
 * in how they format coordinates, so each passes its own formatCoords hook.
 */
'use strict';

const weather = require('../weather/weather');
const { status } = require('../weather/util');
const stock = require('../stock/stock');
const locationComponent = require('../clay/location-component');

const WEATHER_KEYS = ['WEATHER_PROVIDER', 'API_KEY', 'TEMPERATURE_UNIT', 'USE_GPS', 'GPS_FALLBACK', 'LOCATION_NAME'];

// the stock settings that mean a refetch is worth it after the config closes
const STOCK_KEYS = ['STOCK_PROVIDER', 'STOCK_API_KEY', 'STOCK_SYMBOLS'];

// how many tickers the watchlist strip can carry. matches STOCK_MAX_SLOTS on the watch
const STOCK_MAX_SLOTS = 4;

// a ticker is upper letters plus dot and dash (BRK.B, some intl suffixes) up to 12 long
const STOCK_SYMBOL_RE = /^[A-Z.\-]{1,12}$/;

/** @const {!Array<{key: string, field: string}>} Extra weather readings, mapping a
 *  message key to its field on the provider result. */
const EXTRA_WEATHER_FIELDS = [
  { key: 'HUMIDITY', field: 'humidity' },
  { key: 'WIND_SPEED', field: 'windKmh' },
  { key: 'WIND_DIR', field: 'windDir' },
  { key: 'SUNRISE', field: 'sunrise' },
  { key: 'SUNSET', field: 'sunset' },
  { key: 'UV_INDEX', field: 'uvIndex' },
  { key: 'PRECIPITATION', field: 'precip' },
  { key: 'FEELS_LIKE', field: 'feelsLike' },
  { key: 'PRESSURE', field: 'pressure' },
  { key: 'CLOUD', field: 'cloud' },
  { key: 'WIND_GUST', field: 'windGustKmh' },
  { key: 'DEW_POINT', field: 'dewPoint' },
  { key: 'TEMP_MAX', field: 'tempMax' },
  { key: 'TEMP_MIN', field: 'tempMin' },
  { key: 'PRECIP_CHANCE', field: 'precipChance' },
  { key: 'PRECIP_TOTAL', field: 'precipTotal' },
  { key: 'UV_MAX', field: 'uvMax' }
];

// no-reading sentinel a forecast temperature ships as. matches the C
// weather_store WEATHER_NO_TEMP so the watch can tell a real reading from a gap
const FORECAST_NO_TEMP = -1000;

/**
 * Encodes a signed value as two little-endian bytes.
 *
 * A missing reading rides as the FORECAST_NO_TEMP sentinel so the watch draws a
 * placeholder instead of a phantom zero.
 *
 * @param {?number} value A temperature, or null/undefined for no reading.
 * @return {!Array<number>} [low, high]
 */
function int16Bytes(value) {
  const reading = (value === null || value === undefined) ? FORECAST_NO_TEMP : value;
  const wrapped = reading < 0 ? reading + 0x10000 : reading;
  return [wrapped & 0xff, (wrapped >> 8) & 0xff];
}

/**
 * Packs the hourly forecast strip into the wire bytes the watch decodes.
 *
 * Layout: [count][baseHour][stepHours] then per column [code][tempLow][tempHigh].
 *
 * @param {?Object} hourly {baseHour, stepHours, cols:[{code, temp}]} or null.
 * @return {?Array<number>} The byte array, or null when there is nothing to send.
 */
function packForecastHourly(hourly) {
  if (!hourly || !hourly.cols || !hourly.cols.length) {
    return null;
  }

  const bytes = [hourly.cols.length, hourly.baseHour & 0xff, hourly.stepHours & 0xff];
  hourly.cols.forEach((col) => {
    bytes.push(col.code & 0xff, ...int16Bytes(col.temp));
  });
  return bytes;
}

/**
 * Packs the 7-day forecast strip into the wire bytes the watch decodes.
 *
 * Layout: [count][baseWeekday] then per column [code][maxLow][maxHigh][minLow][minHigh].
 *
 * @param {?Object} daily {baseWeekday, cols:[{code, tempMax, tempMin}]} or null.
 * @return {?Array<number>} The byte array, or null when there is nothing to send.
 */
function packForecastDaily(daily) {
  if (!daily || !daily.cols || !daily.cols.length) {
    return null;
  }

  const bytes = [daily.cols.length, daily.baseWeekday & 0xff];
  daily.cols.forEach((col) => {
    bytes.push(col.code & 0xff, ...int16Bytes(col.tempMax), ...int16Bytes(col.tempMin));
  });
  return bytes;
}

/**
 * Clamps a number to the signed range of a little-endian field, rounded first.
 *
 * A price or percent that overflows its field would wrap to a wild wrong value,
 * so pin it to the range instead.
 *
 * @param {number} value The value to fit.
 * @param {number} bits The field width in bits (16 or 32).
 * @return {number} The rounded, clamped integer.
 */
function clampInt(value, bits) {
  const limit = Math.pow(2, bits - 1);
  let rounded = Math.round(Number(value) || 0);
  if (rounded > limit - 1) {
    rounded = limit - 1;
  }
  if (rounded < -limit) {
    rounded = -limit;
  }
  return rounded;
}

/**
 * Encodes a signed integer as little-endian bytes.
 *
 * Uses division rather than bit shifts so a 32-bit value does not trip JS's
 * signed shift operators.
 *
 * @param {number} value The already-clamped integer.
 * @param {number} byteCount How many bytes to emit.
 * @return {!Array<number>} The bytes, least significant first.
 */
function leBytes(value, byteCount) {
  let wrapped = value < 0 ? value + Math.pow(2, byteCount * 8) : value;
  const out = [];
  for (let i = 0; i < byteCount; i++) {
    out.push(wrapped % 256);
    wrapped = Math.floor(wrapped / 256);
  }
  return out;
}

/**
 * Splits the comma list of tickers into clean uppercase symbols.
 *
 * A junk or over-long entry is dropped so the watch never gets a bad symbol, and
 * the list is capped at the strip size.
 *
 * @param {*} raw The STOCK_SYMBOLS setting, like "AAPL, msft ,tsla".
 * @return {!Array<string>} The valid symbols, in order.
 */
function parseSymbols(raw) {
  return String(raw || '')
    .split(',')
    .map((symbol) => symbol.trim().toUpperCase())
    .filter((symbol) => STOCK_SYMBOL_RE.test(symbol))
    .slice(0, STOCK_MAX_SLOTS);
}

// alpha vantage is end-of-day and its free tier is capped daily, so a poll landing inside
// the hour just burns quota for data that has not moved. hold its fetches to hourly
const AV_MIN_FETCH_INTERVAL_MS = 60 * 60 * 1000;

/**
 * Whether a stock poll should be skipped to spare the Alpha Vantage quota.
 *
 * The watch's poll interval does not apply to Alpha Vantage. When it is the chosen source and
 * a poll lands inside the hour we skip the fetch and let the watch keep the last strip. Any
 * other provider always fetches, and a forced fetch (a settings change may have swapped the
 * symbols or provider) always goes through too.
 *
 * @param {string} provider The chosen data source.
 * @param {boolean} force True to bypass the throttle after a settings change.
 * @param {number} lastFetchMs When the last fetch round finished, in epoch ms (0 for never).
 * @param {number} now The current time in epoch ms.
 * @return {boolean} True to skip this poll.
 */
function shouldThrottleStockFetch(provider, force, lastFetchMs, now) {
  return !force && provider === 'alphavantage'
    && now - lastFetchMs < AV_MIN_FETCH_INTERVAL_MS;
}

/**
 * Packs the watchlist quotes into the wire bytes the watch decodes.
 *
 * Layout: [count] then per slot [ok][price int32 LE cents][pct int16 LE
 * hundredths][symLen][sym bytes]. A failed slot carries its short status text in
 * the symbol field with ok = 0 so the watch can show "RATE LIMIT" and friends.
 *
 * @param {?Array<!Object>} results One quote result per configured symbol.
 * @return {?Array<number>} The byte array, or null when there is nothing to send.
 */
function packStockStrip(results) {
  if (!results || !results.length) {
    return null;
  }

  const slots = results.slice(0, STOCK_MAX_SLOTS);
  const bytes = [slots.length];

  slots.forEach((result) => {
    const ok = result && result.ok ? 1 : 0;
    const priceCents = clampInt(Math.round((result && result.price || 0) * 100), 32);
    const pctHundredths = clampInt(Math.round((result && result.changePercent || 0) * 100), 16);
    // a good slot carries its ticker, a failed one carries its status text so the
    // watch has something to show. cap to what the store's symbol buffer holds
    const label = String((result && (result.ok ? result.symbol : result.status)) || '').toUpperCase().slice(0, 11);

    bytes.push(ok);
    bytes.push(...leBytes(priceCents, 4));
    bytes.push(...leBytes(pctHundredths, 2));
    bytes.push(label.length);
    for (let i = 0; i < label.length; i++) {
      bytes.push(label.charCodeAt(i) & 0x7f);
    }
  });

  return bytes;
}

/**
 * Collects {messageKey: defaultValue} from the Clay config so the same
 * defaults apply whether or not the user has opened the settings page yet.
 * @param {!Array<!Object>} items
 * @return {!Object}
 */
function collectDefaults(items) {
  return items.reduce((defaults, item) => {
    if (item.messageKey && item.defaultValue !== undefined) {
      defaults[item.messageKey] = item.defaultValue;
    }
    if (item.items) {
      Object.assign(defaults, collectDefaults(item.items));
    }
    return defaults;
  }, {});
}

/**
 * Performs an HTTP GET.
 * @param {string} url
 * @param {function(?string, string=)} callback callback(error, responseText)
 */
function request(url, callback) {
  const xhr = new XMLHttpRequest();

  xhr.onload = () => {
    // 2xx is success. otherwise flag an error but still pass the body so a
    // provider can read its own error JSON
    if (xhr.status >= 200 && xhr.status < 300) {
      callback(null, xhr.responseText);
    } else {
      callback('http ' + xhr.status, xhr.responseText);
    }
  };

  xhr.onerror = () => {
    callback('network error');
  };

  xhr.ontimeout = () => {
    callback('timeout');
  };

  xhr.timeout = 15000;

  xhr.open('GET', url);
  xhr.send();
}

/**
 * Reads the persisted Clay settings from localStorage.
 * @return {!Object}
 */
function getConfig() {
  try {
    return JSON.parse(localStorage.getItem('clay-settings')) || {};
  } catch (error) {
    return {};
  }
}

/**
 * Unwraps a Clay value, applying a fallback for empty values.
 * @param {*} value
 * @param {*} fallback
 * @return {*}
 */
function readValue(value, fallback) {
  let result = value;

  if (result && typeof result === 'object' && 'value' in result) {
    result = result.value;
  }

  if (result === undefined || result === null || result === '') {
    return fallback;
  }

  return result;
}

/**
 * Reads a boolean Clay setting, applying a fallback when it is unset.
 * @param {*} value
 * @param {boolean} fallback
 * @return {boolean}
 */
function readBool(value, fallback) {
  const result = readValue(value, fallback);
  return result === true || result === 'true' || result === 1 || result === '1';
}

/**
 * Reports whether a coordinate pair is within the valid geographic range.
 * @param {*} lat
 * @param {*} lon
 * @return {boolean}
 */
function validCoord(lat, lon) {
  return typeof lat === 'number' && lat >= -90 && lat <= 90 &&
    typeof lon === 'number' && lon >= -180 && lon <= 180;
}

/**
 * Reads the manual location the user picked in settings.
 * @param {!Object} config
 * @return {?{coords: {lat: number, lon: number}, label: string}}
 */
function getManualLocation(config) {
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
        label: parsed.label || ''
      };
    }
  } catch (error) {
    // fall through on parsing error
  }

  return null;
}

// the watch payload is already sanitized C-side but guard each copy anyway so
// a malformed round-trip can't seed junk into the Clay store
const isString = (value) => typeof value === 'string';
const isEnum = (value) => typeof value === 'string' || typeof value === 'number';
const asBool = (value) => value === 1;

// settings we copy from the watch payload into the Clay store
// accept is an optional type guard and coerce an optional transform (default is copy as-is)
const SEED_FIELDS = [
  { key: 'TEMPERATURE_UNIT', coerce: asBool },
  { key: 'DATE_FORMAT', accept: isString },
  { key: 'THEME', accept: isEnum, coerce: String },
  { key: 'STEPS_MODE', accept: isEnum, coerce: String },
  { key: 'TIME_FORMAT', accept: isEnum, coerce: String },
  { key: 'BLUETOOTH_ICON', coerce: asBool },
  { key: 'BLUETOOTH_VIBE_CONNECT', accept: isEnum, coerce: String },
  { key: 'BLUETOOTH_VIBE_DISCONNECT', accept: isEnum, coerce: String }
];

/**
 * Seeds the Clay store from the watch's current settings so the config opens
 * with the real values instead of defaults. The watch persist is the source of
 * truth, since the phone's clay-settings can be empty or stale after an update.
 * @param {!Object} messageKeys The face's generated message-key map.
 * @param {!Object} payload
 * @param {!Array<string>=} seedKeys Extra face-specific select keys to seed as strings.
 */
function seedConfigFromWatch(messageKeys, payload, seedKeys) {
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

  // extra face keys (like Modular's layout rows + goals) are all selects so they
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
 * Snapshots the weather-relevant settings so a later save can be diffed against
 * them. Each value is JSON-encoded so objects compare by content.
 * @return {!Array<*>}
 */
function weatherSettingsSnapshot() {
  const config = getConfig();
  return WEATHER_KEYS.map((key) => JSON.stringify(config[key]));
}

/**
 * Reports whether any weather-relevant setting changed between two snapshots.
 * @param {?Array<*>} before Snapshot from when the config page opened.
 * @param {!Array<*>} after Snapshot taken after the save was persisted.
 * @return {boolean}
 */
function weatherSettingsChanged(before, after) {
  // no snapshot means the page never reported opening so refetch to be safe
  if (!before) {
    return true;
  }

  return WEATHER_KEYS.some((key, index) => after[index] !== before[index]);
}

/**
 * Snapshots the stock-relevant settings so a later save can be diffed against
 * them. Each value is JSON-encoded so objects compare by content.
 * @return {!Array<*>}
 */
function stockSettingsSnapshot() {
  const config = getConfig();
  return STOCK_KEYS.map((key) => JSON.stringify(config[key]));
}

/**
 * Reports whether any stock-relevant setting changed between two snapshots.
 * @param {?Array<*>} before Snapshot from when the config page opened.
 * @param {!Array<*>} after Snapshot taken after the save was persisted.
 * @return {boolean}
 */
function stockSettingsChanged(before, after) {
  // no snapshot means the page never reported opening so refetch to be safe
  if (!before) {
    return true;
  }

  return STOCK_KEYS.some((key, index) => after[index] !== before[index]);
}

/**
 * Starts the app: builds the Clay settings page, wires the lifecycle listeners,
 * and fetches weather on demand.
 * @param {!Object} options
 * @param {!Array} options.clayConfig The face's Clay config.
 * @param {function} options.formatCoords (messageKeys, result) -> the coordinate
 *     keys for this face's AppMessage dict.
 */
function startPebbleApp(options) {
  const clayConfig = options.clayConfig;
  const formatCoords = options.formatCoords;

  // required here (not at module load) so the unit specs can use the shared
  // helpers without loading Clay or the per-face message_keys alias
  const Clay = require('@rebble/clay/src/js/index');
  const messageKeys = require('message_keys');
  const clay = new Clay(clayConfig, options.customClay);

  // register the location autocomplete (referenced as type "locationsearch" in
  // config.js) before the settings page is built
  clay.registerComponent(locationComponent);

  // any face-specific custom components (e.g. Modular's layout builder)
  (options.components || []).forEach((component) => clay.registerComponent(component));

  /** @const {!Object} The defaults declared in config.js. */
  const DEFAULTS = collectDefaults(clayConfig);

  /** @type {?Array<*>} Weather settings captured when the config page opened. */
  let weatherSettingsBeforeConfig = null;

  /** @type {?Array<*>} Stock settings captured when the config page opened. */
  let stockSettingsBeforeConfig = null;

  /** @type {number} When the last stock fetch round finished, for the Alpha Vantage throttle. */
  let lastStockFetchMs = 0;

  /**
   * Sends a weather result to the watch. The coordinate keys come from the
   * face's formatCoords hook.
   * @param {!Object} result
   */
  function sendWeather(result) {
    const dict = {
      [messageKeys.TEMPERATURE]: result.temperature,
      [messageKeys.CONDITIONS]: result.condition,
      [messageKeys.WEATHER_OK]: result.ok ? 1 : 0
    };

    if (result.location && messageKeys.LOCATION_NAME !== undefined) {
      dict[messageKeys.LOCATION_NAME] = result.location;
    }

    // extra readings only sent for faces that declare the keys (like Modular)
    // a missing value is left out so the watch keeps its placeholder
    EXTRA_WEATHER_FIELDS.forEach(({ key, field }) => {
      const value = result[field];
      if (messageKeys[key] !== undefined && value !== undefined && value !== '') {
        dict[messageKeys[key]] = value;
      }
    });

    // the forecast strips ride as packed byte arrays. only for faces that
    // declare the keys and only when the provider actually supplied a strip
    if (messageKeys.FORECAST_HOURLY !== undefined) {
      const bytes = packForecastHourly(result.forecastHourly);
      if (bytes) {
        dict[messageKeys.FORECAST_HOURLY] = bytes;
      }
    }

    if (messageKeys.FORECAST_DAILY !== undefined) {
      const bytes = packForecastDaily(result.forecastDaily);
      if (bytes) {
        dict[messageKeys.FORECAST_DAILY] = bytes;
      }
    }

    Object.assign(dict, formatCoords(messageKeys, result));

    Pebble.sendAppMessage(
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
   * forwards the result to the watch.
   */
  function getWeather() {
    const config = getConfig();

    const opts = {
      provider: String(readValue(config.WEATHER_PROVIDER, DEFAULTS.WEATHER_PROVIDER)),
      key: String(readValue(config.API_KEY, DEFAULTS.API_KEY || '')).trim().slice(0, 64),
      fahrenheit: readBool(config.TEMPERATURE_UNIT, DEFAULTS.TEMPERATURE_UNIT),
      coords: null,
      label: undefined,
    };

    const useGps = readBool(config.USE_GPS, DEFAULTS.USE_GPS);
    const gpsFallback = readBool(config.GPS_FALLBACK, DEFAULTS.GPS_FALLBACK);
    const manual = getManualLocation(config);

    const fetchFor = (coords, label) => {
      opts.coords = coords;
      opts.label = label;
      console.log(`Fetching Weather: ${opts.provider} @ ${coords.lat},${coords.lon} (${label})`);

      weather.fetchWeather(opts, request, (result) => {
        console.log(`Weather: ${result.condition} ${result.temperature} (ok=${result.ok})`);
        sendWeather(result);
      });
    };

    const useManual = () => {
      if (manual) {
        return fetchFor(manual.coords, manual.label);
      }
      sendWeather(status('No Location'));
    };

    // skip gps entirely when the user disabled it
    if (!useGps) {
      return useManual();
    }

    // fall back to the manual location if the phone has no geolocation API
    if (typeof navigator === 'undefined' || !navigator.geolocation) {
      if (gpsFallback) {
        return useManual();
      } else {
        return sendWeather(status('No GPS'));
      }
    }

    navigator.geolocation.getCurrentPosition(
      (pos) => {
        fetchFor({ lat: pos.coords.latitude, lon: pos.coords.longitude }, 'My Location');
      },
      () => {
        if (gpsFallback) {
          useManual();
        } else {
          sendWeather(status('No GPS'));
        }
      },
      { timeout: 15000, maximumAge: 60000 }
    );
  }

  /**
   * Sends the packed watchlist strip to the watch. No-op for faces that do not
   * declare the STOCK_STRIP key.
   * @param {!Array<!Object>} results One quote result per configured symbol.
   */
  function sendStocks(results) {
    const bytes = packStockStrip(results);
    if (!bytes) {
      return;
    }

    Pebble.sendAppMessage(
      { [messageKeys.STOCK_STRIP]: bytes },
      () => {
        console.log('Stock info sent to Pebble successfully!');
      },
      () => {
        console.error('Error sending stock info to Pebble!');
      }
    );
  }

  /**
   * Fetches a quote for each configured symbol, then forwards the packed strip
   * to the watch. Skipped entirely for faces that do not show stocks.
   *
   * @param {boolean=} force True to fetch even when the Alpha Vantage throttle would
   * otherwise hold it back (a settings change may have swapped the symbols or provider).
   */
  function getStocks(force) {
    // a face that does not declare the strip key never shows stocks so skip the fetch
    if (messageKeys.STOCK_STRIP === undefined) {
      return;
    }

    const config = getConfig();
    const provider = String(readValue(config.STOCK_PROVIDER, DEFAULTS.STOCK_PROVIDER || 'finnhub'));
    const key = String(readValue(config.STOCK_API_KEY, DEFAULTS.STOCK_API_KEY || '')).trim().slice(0, 64);
    const symbols = parseSymbols(readValue(config.STOCK_SYMBOLS, DEFAULTS.STOCK_SYMBOLS || ''));

    if (!symbols.length) {
      return;
    }

    // let alpha vantage keep the last strip when a poll lands too soon so its quota lasts
    if (shouldThrottleStockFetch(provider, force, lastStockFetchMs, Date.now())) {
      return;
    }

    // fetch every symbol and keep the results in the configured order so the
    // watchlist rows stay put. send once the last one lands
    const results = new Array(symbols.length);
    let pending = symbols.length;

    symbols.forEach((symbol, index) => {
      stock.fetchQuote({ provider, key, symbol }, request, (result) => {
        results[index] = result;
        if (--pending === 0) {
          // mark the round done so the throttle measures from a completed fetch, not a
          // failed one. a failed round leaves the stamp be so the next poll can retry
          lastStockFetchMs = Date.now();
          sendStocks(results);
        }
      });
    });
  }

  // app lifecycle listeners
  Pebble.addEventListener('ready', () => {
    console.log('PebbleKit JS Ready!');
    Pebble.sendAppMessage({ [messageKeys.REQUEST_SETTINGS]: 1 });
    getWeather();
    getStocks();
  });

  Pebble.addEventListener('appmessage', (event) => {
    const payload = event.payload || {};

    if (payload[messageKeys.REQUEST_WEATHER]) {
      getWeather();
    }

    if (payload[messageKeys.REQUEST_STOCK]) {
      getStocks();
    }

    // the watch's reply to our REQUEST_SETTINGS carries the display settings
    if (messageKeys.DATE_FORMAT in payload || messageKeys.THEME in payload) {
      seedConfigFromWatch(messageKeys, payload, options.seedKeys);
    }
  });

  // Clay saves the new settings in its own webviewclosed handler which runs
  // before ours so capture the previous values while the page is still open
  Pebble.addEventListener('showConfiguration', () => {
    weatherSettingsBeforeConfig = weatherSettingsSnapshot();
    stockSettingsBeforeConfig = stockSettingsSnapshot();
  });

  Pebble.addEventListener('webviewclosed', (event) => {
    if (!event || !event.response) {
      return;
    }

    // only refetch when a weather setting actually changed. the C side already
    // re-requests for those so refetching on every save (theme or vibe) is wasted
    const weatherBefore = weatherSettingsBeforeConfig;
    weatherSettingsBeforeConfig = null;

    if (weatherSettingsChanged(weatherBefore, weatherSettingsSnapshot())) {
      setTimeout(getWeather, 250);
    }

    // stocks refetch on their own settings change the same way
    const stockBefore = stockSettingsBeforeConfig;
    stockSettingsBeforeConfig = null;

    if (stockSettingsChanged(stockBefore, stockSettingsSnapshot())) {
      setTimeout(() => getStocks(true), 250);
    }
  });
}

module.exports = {
  startPebbleApp,
  packForecastHourly,
  packForecastDaily,
  packStockStrip,
  parseSymbols,
  shouldThrottleStockFetch,
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
  stockSettingsSnapshot,
  stockSettingsChanged,
  WEATHER_KEYS,
  STOCK_KEYS
};
