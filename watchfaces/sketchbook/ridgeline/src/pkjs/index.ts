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
import sketchbookChrome from '../../../core/pkjs/clay/chrome-component';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  // the paper-and-ink restyle of the settings page, shared by every Sketchbook face
  components: [sketchbookChrome],
  // weather with no coordinates, since this face never displays them
  features: [weather],
});
