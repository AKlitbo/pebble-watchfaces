/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face draws the
 * weather rather than spelling it out, and it needs the sunrise and sunset to place the sun
 * on its arc, which is why its appinfo declares the extra weather keys. Everything else is
 * shared.
 */
import app from '../../../../../lib/ts/pkjs/app';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // this face never displays coordinates so it sends none
  formatCoords: () => ({}),
});
