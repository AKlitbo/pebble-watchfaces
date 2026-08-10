/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face shows no
 * coordinates, but the location keys still have to be declared and filled, so they carry the
 * plain signed values. Both builder components register here: the layout grid and the panel
 * appearance page.
 */
import app from '../../../../../lib/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import type { WeatherResult } from '../../../../../lib/ts/weather/util';
import clayConfig, { customClay } from './config';

/** One signed coordinate as a plain string, or empty when there is no fix. */
function coord(value: number | undefined): string {
  if (typeof value !== 'number' || Number.isNaN(value)) {
    return '';
  }

  return value.toFixed(4);
}

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent],
  customClay,
  seedKeys: ['WEATHER_TEMPERATURE_UNIT'],
  // colours seed as the numbers Clay's picker wants, not as the strings a select takes
  seedColorKeys: [
    'APPEARANCE_POINTER_COLOR',
    'APPEARANCE_POINTER_INK',
    'APPEARANCE_REEL_COLOR',
    'APPEARANCE_REEL_INK',
  ],
  seedBoolKeys: ['APPEARANCE_FACE_COLORS'],
  formatCoords: (messageKeys: Record<string, number>, result: WeatherResult) => ({
    [messageKeys.LOCATION_LATITUDE]: coord(result.lat),
    [messageKeys.LOCATION_LONGITUDE]: coord(result.lon),
  }),
});
