/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face shows the
 * weather condition and temperature but no coordinates, so it opts into the plain weather
 * feature. Everything else is shared.
 */
import app from '../../lib/ts/pkjs/app';
import weather from '../../lib/ts/weather/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // weather with no coordinates, since this face never displays them
  features: [weather],
});
