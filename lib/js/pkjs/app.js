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
const locationComponent = require('../clay/location-component');

const WEATHER_KEYS = ['WEATHER_PROVIDER', 'API_KEY', 'TEMPERATURE_UNIT', 'USE_GPS', 'GPS_FALLBACK', 'LOCATION_NAME'];

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

  // app lifecycle listeners
  Pebble.addEventListener('ready', () => {
    console.log('PebbleKit JS Ready!');
    Pebble.sendAppMessage({ [messageKeys.REQUEST_SETTINGS]: 1 });
    getWeather();
  });

  Pebble.addEventListener('appmessage', (event) => {
    const payload = event.payload || {};

    if (payload[messageKeys.REQUEST_WEATHER]) {
      getWeather();
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
  });

  Pebble.addEventListener('webviewclosed', (event) => {
    if (!event || !event.response) {
      return;
    }

    // only refetch when a weather setting actually changed. the C side already
    // re-requests for those so refetching on every save (theme or vibe) is wasted
    const before = weatherSettingsBeforeConfig;
    weatherSettingsBeforeConfig = null;

    if (weatherSettingsChanged(before, weatherSettingsSnapshot())) {
      setTimeout(getWeather, 250);
    }
  });
}

module.exports = {
  startPebbleApp,
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
  WEATHER_KEYS
};
