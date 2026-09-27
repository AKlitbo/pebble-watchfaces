/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face shows no
 * coordinates, so it opts into the plain weather feature. Both builder components register
 * here: the layout grid and the panel appearance page.
 */
import app from '../../../lib/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import weather from '../../../lib/ts/weather/feature';
import clayConfig, { customClay } from './config';

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent],
  customClay,
  features: [weather],
});
