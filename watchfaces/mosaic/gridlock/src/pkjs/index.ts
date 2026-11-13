import app from '../../../paf/ts/pkjs/app';
import controlsComponent from './clay/controls-component.g';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import weather from '../../../paf/ts/weather/feature';
import stocks from '../../../paf/ts/stock/feature';
import calendar from '../../../paf/ts/calendar/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  components: [controlsComponent, layoutComponent, themeComponent],
  features: [weather, stocks, calendar],
});
