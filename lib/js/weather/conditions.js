/**
 * Single source of truth for the weather-condition vocabulary.
 *
 * Each token is the short condition word the PebbleKit JS producer emits (see
 * util.js wmoToCondition / applyNight / CONDITION_ALIASES) and the C consumer
 * looks up for an icon and a label. The C side (icons_table.h, labels_table.h)
 * is generated from this list by tools/generate-conditions.js. Edit the
 * vocabulary here, never there.
 *
 * Fields per row:
 *   token         stable wire/icon key. NEVER reword it. Saved settings, the
 *                 icon table and in-flight messages all depend on its spelling.
 *   resource      day icon (RESOURCE_ID_ICON_<resource>).
 *   nightResource optional night icon. When present the JS emits "<token>_NIGHT"
 *                 after dark and the generator adds a "<token>_NIGHT" branch to
 *                 both tables (the night row reuses the day labels).
 *   labelShort    compact display for tiny faces / small slots. Seeded to the
 *                 token text (exactly what small faces show) so you can reword
 *                 it freely later without touching the token.
 *   labelLong     full readable display for roomy faces.
 *
 * FZDZ and FZRN intentionally share WI_SLEET / WI_NIGHT_SLEET (there is no
 * distinct freezing-drizzle glyph).
 */
'use strict';

// wire code for a condition the watch doesn't recognise. the C wx_resource_for_code
// sends this to the WI_NA fallback glyph. real conditions ride as their array index
const UNKNOWN_CODE = 255;

// high bit OR-ed onto an hourly forecast code to mark a night hour, so the watch loads the
// night glyph for that column. real indices are 0-11 so this bit is free. the C side mirrors
// it as WX_FORECAST_NIGHT_BIT in ui/weather/icons.h. only the hourly strip sets it (the daily
// strip is whole-day and stays on day glyphs)
const FORECAST_NIGHT_BIT = 0x80;

const vocabulary = {
  // shown when a condition isn't recognized (the UNKNOWN token routes here)
  fallback: { resource: 'WI_NA', labelShort: 'UNKNOWN', labelLong: 'Unknown' },
  conditions: [
    { token: 'CLEAR', resource: 'WI_CLEAR',         nightResource: 'WI_NIGHT_CLEAR',         labelShort: 'CLEAR', labelLong: 'Clear' },
    { token: 'PCLDY', resource: 'WI_PARTLY_CLOUDY', nightResource: 'WI_NIGHT_PARTLY_CLOUDY', labelShort: 'PCLDY', labelLong: 'Partly Cloudy' },
    { token: 'CLDY',  resource: 'WI_CLOUDY',        nightResource: 'WI_NIGHT_CLOUDY',        labelShort: 'CLDY',  labelLong: 'Cloudy' },
    { token: 'FOGGY', resource: 'WI_FOG',           nightResource: 'WI_NIGHT_FOG',           labelShort: 'FOGGY', labelLong: 'Fog' },
    { token: 'DRZL',  resource: 'WI_DRIZZLE',       nightResource: 'WI_NIGHT_DRIZZLE',       labelShort: 'DRZL',  labelLong: 'Drizzle' },
    { token: 'FZDZ',  resource: 'WI_SLEET',         nightResource: 'WI_NIGHT_SLEET',         labelShort: 'FZDZ',  labelLong: 'Freezing Drizzle' },
    { token: 'RAIN',  resource: 'WI_RAIN',          nightResource: 'WI_NIGHT_RAIN',          labelShort: 'RAIN',  labelLong: 'Rain' },
    { token: 'FZRN',  resource: 'WI_SLEET',         nightResource: 'WI_NIGHT_SLEET',         labelShort: 'FZRN',  labelLong: 'Freezing Rain' },
    { token: 'SNOW',  resource: 'WI_SNOW',          nightResource: 'WI_NIGHT_SNOW',          labelShort: 'SNOW',  labelLong: 'Snow' },
    { token: 'SHWR',  resource: 'WI_SHOWERS',       nightResource: 'WI_NIGHT_SHOWERS',       labelShort: 'SHWR',  labelLong: 'Showers' },
    { token: 'SNSH',  resource: 'WI_SNOW_WIND',     nightResource: 'WI_NIGHT_SNOW_WIND',     labelShort: 'SNSH',  labelLong: 'Snow Showers' },
    { token: 'STRM',  resource: 'WI_THUNDERSTORM',  nightResource: 'WI_NIGHT_THUNDERSTORM',  labelShort: 'STRM',  labelLong: 'Thunderstorm' }
  ]
};

/**
 * Turns a condition token into its compact wire code for the forecast row.
 *
 * The forecast columns can't each carry a token string, so a condition rides as
 * a single byte: its index in the conditions array. The night marker is dropped
 * first (the forecast shows day glyphs) and anything unrecognized becomes
 * UNKNOWN_CODE so the watch draws WI_NA.
 *
 * @param {string} token A condition token like "RAIN" or "RAIN_NIGHT".
 * @return {number} The array index, or UNKNOWN_CODE when the token is unknown.
 */
function codeFor(token) {
  if (!token) {
    return UNKNOWN_CODE;
  }

  const base = String(token).replace(/_NIGHT$/, '');
  const index = vocabulary.conditions.findIndex((entry) => entry.token === base);
  return index < 0 ? UNKNOWN_CODE : index;
}

module.exports = vocabulary;
module.exports.UNKNOWN_CODE = UNKNOWN_CODE;
module.exports.FORECAST_NIGHT_BIT = FORECAST_NIGHT_BIT;
module.exports.codeFor = codeFor;
