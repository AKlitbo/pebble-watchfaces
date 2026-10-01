import app from '../../../paf/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import hiddenStoreComponent from '../../../paf/ts/clay/hidden-store-component';
import weather from '../../../paf/ts/weather/feature';
import stocks from '../../../paf/ts/stock/feature';
import calendar from '../../../paf/ts/calendar/feature';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent, hiddenStoreComponent],
  features: [weather, stocks, calendar],
});
