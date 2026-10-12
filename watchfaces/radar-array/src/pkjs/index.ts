/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see paf/ts/pkjs/app.ts). This face opts into weather
 * with coordinates, which coords.ts builds. Everything else is shared.
 */
import app from '../../paf/ts/pkjs/app';
import weather from '../../paf/ts/weather/feature';
import clayConfig from './config';
import { coordFields } from './coords';

app.startPebbleApp({
  clayConfig,
  features: [weather.withCoords(coordFields)],
});
