/**
 * Per-readout presentation metadata for the Clay slot builder, keyed by label (matching
 * config.ts OPS_OPTIONS and module-thumbnails.g.js) so a lookup survives a numeric id drift.
 *
 * config.ts merges these onto OPS_OPTIONS so the builder reads them straight from its config. It
 * cannot import this file directly, because Clay turns the builder's initialize into text and
 * runs it in a webview of its own where module scope (and this import) does not exist.
 *
 * Fields per row:
 *   icon        emoji fallback, shown when a readout has no real panel thumbnail.
 *   blockColor  the tint the builder paints behind the icon.
 *   slug        the resources/thumbnails/<slug>-<size>.png filename stem. embed-thumbnails.ts
 *               reads it from here so the thumbnail list has one home, not a second copy.
 *
 * Empty is deliberately absent. It has no panel to photograph, and the builder offers it as a
 * clear action rather than as something to drag.
 */

import type { ModuleMeta } from '../../../../../lib/ts/clay/types';

const moduleMeta: Record<string, ModuleMeta> = {
  'Heart Rate (VITALS)':            { icon: '❤️', blockColor: '#e74c3c', slug: 'heart' },
  'Steps / Distance (TRAVERSAL)':   { icon: '👟', blockColor: '#2ecc71', slug: 'steps' },
  'Battery (POWER)':                { icon: '🔋', blockColor: '#f0a500', slug: 'battery' },
  'Calories (METABOLIC)':           { icon: '🔥', blockColor: '#e67e22', slug: 'calories' },
  'Sleep (REGEN)':                  { icon: '😴', blockColor: '#4a5568', slug: 'sleep' },
  'Active Minutes (EXERTION)':      { icon: '🏃', blockColor: '#1abc9c', slug: 'active' },

  'Moon Phase % (LUNAR)':           { icon: '🌘', blockColor: '#7e57c2', slug: 'moon-pct' },
  'Moon Phase Name (LUNAR)':        { icon: '🌗', blockColor: '#7e57c2', slug: 'moon-phase' },
  'Next Full / New Moon':           { icon: '🌑', blockColor: '#673ab7', slug: 'moon-next' },

  'Sunrise (DAWN)':                 { icon: '🌅', blockColor: '#ffa726', slug: 'sunrise' },
  'Sunset (DUSK)':                  { icon: '🌇', blockColor: '#ef6c00', slug: 'sunset' },
  'Length of Day (DAYLIGHT)':       { icon: '☀️', blockColor: '#fbc02d', slug: 'daylight' },
  'Countdown to Sunrise / Sunset':  { icon: '⏳', blockColor: '#ff8f00', slug: 'sun-next' },

  'Humidity (ATMOS)':               { icon: '💧', blockColor: '#29b6f6', slug: 'humidity' },
  'Wind (AIRFLOW)':                 { icon: '🌬', blockColor: '#4dd0e1', slug: 'wind' },
  'UV Index':                       { icon: '😎', blockColor: '#ab47bc', slug: 'uv' },
  'High / Low Temperature (RANGE)': { icon: '🌡', blockColor: '#42a5f5', slug: 'hilo' },
  'Temperature (THERMAL)':          { icon: '🌡', blockColor: '#42a5f5', slug: 'temp' },
  'Conditions (SKY)':               { icon: '🌤', blockColor: '#2196f3', slug: 'conditions' },

  'Julian Date':                    { icon: '🗓', blockColor: '#90a4ae', slug: 'julian' },
  'Day of Year (SOL)':              { icon: '📅', blockColor: '#90a4ae', slug: 'day-of-year' },
  'Week Number (CYCLE)':            { icon: '🗓', blockColor: '#78909c', slug: 'week' },

  'Epoch Clock':                    { icon: '🕗', blockColor: '#aed581', slug: 'epoch' },
  'Swatch Beats':                   { icon: '🌐', blockColor: '#e91e63', slug: 'beats' },
  'Alternate Time Zone (ZONE 1)':   { icon: '🌍', blockColor: '#8e44ad', slug: 'zone1' },

  // the tall one that fills the whole left column
  'Sensors Block (weather + temperature)': { icon: '🌤', blockColor: '#1e88e5', slug: 'sensors' },
};

export default moduleMeta;
