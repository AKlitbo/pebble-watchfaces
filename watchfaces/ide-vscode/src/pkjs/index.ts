/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see paf/ts/pkjs/app.ts). This face shows the
 * weather condition and temperature but no coordinates, so it opts into the plain weather
 * feature. Everything else is shared.
 */
import app from '../../paf/ts/pkjs/app';
import weather from '../../paf/ts/weather/feature';
import ideChrome from './clay/chrome-component';
import fieldHelp from './clay/field-help';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // the 95-style restyle of the settings page, and each theme picker's help moved above its picture
  components: [ideChrome],
  customClay: fieldHelp,
  // weather with no coordinates, since this face never displays them
  features: [weather],
});
