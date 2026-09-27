/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see lib/ts/pkjs/app.ts). This face shows no
 * coordinates, so it opts into the plain weather feature. The page's custom components register
 * here: the layout grid, the panel appearance page, and the hidden stores that keep the layout
 * library and the night and Quiet Time layouts. Clay stops building the page at the first item
 * whose component is missing, so everything below it never shows.
 */
import app from '../../../lib/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import hiddenStoreComponent from '../../../lib/ts/clay/hidden-store-component';
import weather from '../../../lib/ts/weather/feature';
import clayConfig, { customClay } from './config';

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent, hiddenStoreComponent],
  customClay,
  features: [weather],
});
