/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face formats
 * coordinates as a single hemisphere-style readout carried in the latitude key. The
 * longitude key is left empty. Everything else is shared.
 */
import app from '../../../../lib/ts/pkjs/app';
import type { WeatherResult } from '../../../../lib/ts/weather/util';
import clayConfig from './config';

/**
 * Formats one signed coordinate as a magnitude plus hemisphere letter.
 * E.g. 34.0522 -> "34.05N", -118.2437 -> "118.24W". Missing -> "".
 */
function fmtCoord(value: number | undefined, positive: string, negative: string): string {
  if (typeof value !== 'number' || Number.isNaN(value)) {
    return '';
  }

  const hemisphere = value >= 0 ? positive : negative;
  return Math.abs(value).toFixed(2) + hemisphere;
}

/** Builds the combined "lat lon" readout, or a no-fix placeholder. */
function coordString(result: WeatherResult): string {
  if (typeof result.lat === 'number' && typeof result.lon === 'number') {
    return fmtCoord(result.lat, 'N', 'S') + ' ' + fmtCoord(result.lon, 'E', 'W');
  }

  return 'NO LOCK';
}

app.startPebbleApp({
  clayConfig,
  // the combined readout travels in the latitude key
  // longitude is left empty
  formatCoords: (messageKeys: Record<string, number>, result: WeatherResult) => ({
    [messageKeys.LOCATION_LATITUDE]: coordString(result),
    [messageKeys.LOCATION_LONGITUDE]: '',
  }),
});
