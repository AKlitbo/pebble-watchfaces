import app from '../../../../lib/ts/pkjs/app';
import layoutComponent from './clay/layout-component.g';
import themeComponent from './clay/theme-component.g';
import hiddenStoreComponent from '../../../../lib/ts/clay/hidden-store-component';
import clayConfig from './config';

// the select settings seeded from the watch payload as their "0"/"1" string form so the config
// page opens on the right choice. WEATHER_TEMPERATURE_UNIT belongs here (not in app.ts SEED_FIELDS)
// because it is a select and not a bool
const seedKeys = [
  'LAYOUT',
  'LAYOUT_NIGHT',
  'LAYOUT_NIGHT_MODE',
  'LAYOUT_NIGHT_START',
  'LAYOUT_NIGHT_END',
  'HEALTH_GOAL_STEPS',
  'HEALTH_GOAL_CALORIES',
  'HEALTH_GOAL_SLEEP',
  'HEALTH_GOAL_ACTIVE',
  'HEALTH_GOAL_HR',
  'HEALTH_GOAL_DISTANCE',
  'HEALTH_DISTANCE_UNIT',
  'HEALTH_GOAL_VIBE',
  'HEALTH_GOAL_VIBE_CUSTOM',
  'WEATHER_WIND_UNIT',
  'WEATHER_TEMPERATURE_UNIT',
  'APPEARANCE_CUSTOM_COLORS',
];

app.startPebbleApp({
  clayConfig,
  components: [layoutComponent, themeComponent, hiddenStoreComponent],
  seedKeys,
  formatCoords: () => ({}),
});
