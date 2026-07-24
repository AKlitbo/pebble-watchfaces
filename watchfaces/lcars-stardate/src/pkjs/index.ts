/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face
 * formats coordinates in LCARS dash style into separate latitude/longitude
 * keys. Everything else is shared.
 */
import app from '../../../../lib/ts/pkjs/app';
import type { WeatherResult } from '../../../../lib/ts/weather/util';
import clayConfig from './config';

/**
 * Formats a decimal coordinate in LCARS dash style.
 * E.g. 33.448376 -> "33-448", -112.074036 -> "-112-074". Missing -> "".
 */
function fmtCoord(v: number | undefined): string {
  if (typeof v !== 'number' || Number.isNaN(v)) {
    return '';
  }

  const prefix = v < 0 ? '-' : '';
  return prefix + Math.abs(v).toFixed(3).replace('.', '-');
}

app.startPebbleApp({
  clayConfig,
  // dash style into two keys. fmtCoord yields '' for a missing coordinate
  formatCoords: (messageKeys: Record<string, number>, result: WeatherResult) => ({
    [messageKeys.LOCATION_LATITUDE]: fmtCoord(result.lat),
    [messageKeys.LOCATION_LONGITUDE]: fmtCoord(result.lon),
  }),
});
