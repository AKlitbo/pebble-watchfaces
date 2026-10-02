/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see paf/ts/pkjs/app.ts). This face draws the
 * weather rather than spelling it out, and it needs the sunrise and sunset to place the sun
 * on its arc, which is why its appinfo declares the extra weather keys. Everything else is
 * shared.
 */
import app from '../../../paf/ts/pkjs/app';
import weather from '../../../paf/ts/weather/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // weather with no coordinates, since this face never displays them
  features: [weather],
});
