/**
 * PebbleKit JS entry point.
 *
 * Thin wrapper over the shared bootstrap (see paf/ts/pkjs/app.ts). This face shows no
 * coordinates, so it opts into the plain weather feature, and the phone feature brings the phone's
 * battery for the two phone battery panels. The page's custom components register here: the look
 * every control shares, the layout grid, the panel appearance page, and the reset button. The
 * framework registers the hidden stores that keep the layout library and the night and
 * Quiet Time layouts. Clay stops building the page at the first item whose component is missing, so
 * everything below it never shows.
 */
import app from '../../../paf/ts/pkjs/app';
import controlsComponent from './clay/controls-component.g';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import resetButtonComponent from './clay/reset-button-component';
import weather from '../../../paf/ts/weather/feature';
import phone from '../../../paf/ts/phone/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  components: [controlsComponent, layoutComponent, themeComponent, resetButtonComponent],
  features: [weather, phone],
});
