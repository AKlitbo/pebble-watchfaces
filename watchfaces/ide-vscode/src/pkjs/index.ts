/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face shows the
 * weather condition and temperature but no coordinates, so formatCoords sends no location
 * keys. Everything else is shared.
 */
import app from '../../../../lib/ts/pkjs/app';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // this face never displays coordinates so it sends none
  formatCoords: () => ({}),
});
