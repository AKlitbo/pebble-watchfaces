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

module.exports = {
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
