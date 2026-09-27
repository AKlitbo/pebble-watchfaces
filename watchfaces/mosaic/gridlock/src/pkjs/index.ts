import app from '../../../lib/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import hiddenStoreComponent from '../../../lib/ts/clay/hidden-store-component';
import weather from '../../../lib/ts/weather/feature';
import stocks from '../../../lib/ts/stock/feature';
import calendar from '../../../lib/ts/calendar/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent, hiddenStoreComponent],
  features: [weather, stocks, calendar],
});
