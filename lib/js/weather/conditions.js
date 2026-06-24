/**
 * Single source of truth for the weather-condition vocabulary.
 *
 * Each token is the short condition word the PebbleKit JS producer emits (see
 * util.js wmoToCondition / applyNight / CONDITION_ALIASES) and the C consumer
 * looks up for an icon. The C side (icons_table.h) is generated from this list
 * by tools/generate-conditions.js - edit the vocabulary here, never there.
 *
 * resource is the RESOURCE_ID_ICON_* suffix. FZDZ and FZRN intentionally share
 * WI_SLEET (there is no distinct freezing-drizzle glyph).
 */
'use strict';

module.exports = {
  // shown when a condition isn't recognized (the UNKNOWN token routes here)
  fallback: 'WI_NA',
  conditions: [
    { token: 'CLEAR',       resource: 'WI_CLEAR' },
    { token: 'CLEAR_NIGHT', resource: 'WI_NIGHT_CLEAR' },
    { token: 'CLDY',        resource: 'WI_CLOUDY' },
    { token: 'FOGGY',       resource: 'WI_FOG' },
    { token: 'DRZL',        resource: 'WI_DRIZZLE' },
    { token: 'FZDZ',        resource: 'WI_SLEET' },
    { token: 'RAIN',        resource: 'WI_RAIN' },
    { token: 'FZRN',        resource: 'WI_SLEET' },
    { token: 'SNOW',        resource: 'WI_SNOW' },
    { token: 'SHWR',        resource: 'WI_SHOWERS' },
    { token: 'SNSH',        resource: 'WI_SNOW_WIND' },
    { token: 'STRM',        resource: 'WI_THUNDERSTORM' }
  ]
};
