import app from '../../../paf/ts/pkjs/app';
import controlsComponent from './clay/controls-component.g';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import weather from '../../../paf/ts/weather/feature';
import stocks from '../../../paf/ts/stock/feature';
import calendar from '../../../paf/ts/calendar/feature';
import phone from '../../../paf/ts/phone/feature';
import music from '../../../paf/ts/music/feature';
import layoutPluginSources from '../../../core/pkjs/layout-plugin-sources';
import clayConfig from './config';

app.startPebbleApp({
  clayConfig,
  components: [controlsComponent, layoutComponent, themeComponent],
  // hands the layout builder the sources on the Pebble app plugin, for its warning about a panel with no plugin
  customClay: layoutPluginSources,
  // the art fills the 2x4 Now Playing panel's square, its 83px tile less 2px all round. the 1x4 draws
  // it shrunk, in music_now_playing.c
  features: [weather, stocks, calendar, phone, music.withArtwork({ w: 79, h: 79 })],
});
