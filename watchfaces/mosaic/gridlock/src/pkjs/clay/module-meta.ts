/**
 * Shared per-module presentation metadata for the Clay builders, keyed by label (matching
 * config.ts MODULE_OPTIONS and module-thumbnails.g.js) so a lookup survives a numeric id drift.
 *
 * config.ts merges these onto MODULE_OPTIONS so both builders read them straight from their
 * config. They cannot import this file directly: Clay serialises their initialize and runs it
 * in an isolated webview where module scope (and this import) does not exist.
 *
 * Fields per row:
 *   icon        emoji fallback, shown when a module has no real panel thumbnail.
 *   blockColor  the block tint the layout builder paints behind the icon.
 *   slug        the resources/thumbnails/<slug>-<size>.png filename stem. generate-thumbnails.ts
 *               reads it from here so the thumbnail list has one home, not a second copy.
 *
 * The VIBRANT colour is not here: it lives in vibrant.g.js keyed by module id and
 * config.ts merges it onto MODULE_OPTIONS, so the colour has one home shared with the firmware.
 */

import type { ModuleMeta } from '../../../../core/pkjs/types';

const moduleMeta: Record<string, ModuleMeta> = {
  'Digital Clock': { icon: '⏰', blockColor: '#b0bec5', slug: 'clock' },
  'Battery':       { icon: '🔋', blockColor: '#f0a500', slug: 'battery' },
  'Weather':       { icon: '🌤', blockColor: '#2196f3', slug: 'weather' },
  'Temperature':   { icon: '🌡', blockColor: '#42a5f5', slug: 'temp' },
  'Heart Rate':    { icon: '❤️', blockColor: '#e74c3c', slug: 'heartrate' },
  'Steps':         { icon: '👟', blockColor: '#2ecc71', slug: 'steps' },
  'Distance':      { icon: '🛣', blockColor: '#16a085', slug: 'distance' },
  'Calories':      { icon: '🔥', blockColor: '#e67e22', slug: 'calories' },
  'Sleep':         { icon: '😴', blockColor: '#4a5568', slug: 'sleep' },
  'Activity':      { icon: '🏃', blockColor: '#1abc9c', slug: 'activity' },
  'Humidity':      { icon: '💧', blockColor: '#00bcd4', slug: 'humidity' },
  'Wind Speed':    { icon: '💨', blockColor: '#78909c', slug: 'wind' },
  'Sunrise':       { icon: '🌅', blockColor: '#ff9800', slug: 'sunrise' },
  'Sunset':        { icon: '🌇', blockColor: '#e64a19', slug: 'sunset' },
  'Daylight':      { icon: '☀️', blockColor: '#f1c40f', slug: 'daylight' },
  'Sun':           { icon: '🌞', blockColor: '#f1c40f', slug: 'sun' },
  'Conditions':    { icon: '☁️', blockColor: '#3498db', slug: 'conditions' },
  'Beats':         { icon: '🌐', blockColor: '#e91e63', slug: 'beats' },
  'Date':          { icon: '📅', blockColor: '#9c27b0', slug: 'date' },
  'Time (1x2)':    { icon: '⌚', blockColor: '#b0bec5', slug: 'time' },
  'Time Zone 1':   { icon: '🌍', blockColor: '#8e44ad', slug: 'tz1' },
  'Analog Clock':  { icon: '🕘', blockColor: '#00aaff', slug: 'analog' },
  'UV':            { icon: '☀️', blockColor: '#ff9800', slug: 'uv' },
  'Hi/Low':        { icon: '🌡', blockColor: '#ff7043', slug: 'hilo' },
  'Precipitation': { icon: '🌧', blockColor: '#29b6f6', slug: 'precip' },
  'Forecast (Hourly)': { icon: '🕐', blockColor: '#2196f3', slug: 'forecast-hourly' },
  'Forecast (4-Day)':  { icon: '📆', blockColor: '#2196f3', slug: 'forecast-daily' },
  'Connection':    { icon: '🔗', blockColor: '#607d8b', slug: 'connection' },
  'HR Graph':      { icon: '💓', blockColor: '#e74c3c', slug: 'hrgraph' },
  'Steps Graph':   { icon: '📈', blockColor: '#2ecc71', slug: 'stepsgraph' },
  'Moon':          { icon: '🌙', blockColor: '#5c6bc0', slug: 'moon' },
  'Quiet Time':    { icon: '🔕', blockColor: '#607d8b', slug: 'quiet' },
  'Feels Like':    { icon: '🌡', blockColor: '#ff7043', slug: 'feels' },
  'Pressure':      { icon: '🧭', blockColor: '#78909c', slug: 'pressure' },
  'Dew Point':     { icon: '💧', blockColor: '#00bcd4', slug: 'dew' },
  'Time + Date':   { icon: '🕰', blockColor: '#b0bec5', slug: 'datetime' },
  'Stock':         { icon: '📈', blockColor: '#00c853', slug: 'stock' },
  'Watchlist':     { icon: '📊', blockColor: '#00c853', slug: 'watchlist' },
  'Next Event':    { icon: '🗓️', blockColor: '#ab47bc', slug: 'next-event' },
  'Agenda':        { icon: '🗒️', blockColor: '#ab47bc', slug: 'agenda' },
  'Timeline':      { icon: '🟪', blockColor: '#ab47bc', slug: 'timeline' },
  'Week Number':   { icon: '🔢', blockColor: '#0288d1', slug: 'weeknum' },
  'Day of Year':   { icon: '📆', blockColor: '#00897b', slug: 'dayofyear' },
  'Epoch Clock':   { icon: '🕗', blockColor: '#aed581', slug: 'epoch' },
  'Days Left':     { icon: '⏳', blockColor: '#ffb74d', slug: 'daysleft' },
  'Weekday Dots':  { icon: '⚪', blockColor: '#90a4ae', slug: 'weekday-dots' },
  'Next Moon':     { icon: '🌕', blockColor: '#5c6bc0', slug: 'next-moon' },
  'Month Grid':    { icon: '📅', blockColor: '#8e44ad', slug: 'month' },
  'Julian Date':   { icon: '🔭', blockColor: '#3d5afe', slug: 'julian' },
  'Weeks Left':    { icon: '🔢', blockColor: '#0288d1', slug: 'weeksleft' },
  'Status':        { icon: '📶', blockColor: '#f0a500', slug: 'status' },
  'Big Hour':      { icon: '🕛', blockColor: '#b0bec5', slug: 'big-hour' },
  'Big Minutes':   { icon: '🕧', blockColor: '#b0bec5', slug: 'big-min' },
  'Big Time':      { icon: '🌙', blockColor: '#b0bec5', slug: 'big-time' },
};

export default moduleMeta;
