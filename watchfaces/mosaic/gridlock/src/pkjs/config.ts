import configBuilder from '../../../../../lib/ts/pkjs/config-builder';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';
import vibrantByType from './clay/vibrant.g';
import type { ClayItem } from '../../../core/pkjs/types';

/** A select/toggle option: its label and the value Clay stores. */
type ClayOption = { label: string; value: string | number };

// base list. icon/colour get merged in from module-meta below so the builders read one source
const MODULE_BASE = [
  { label: 'Empty', value: 0, sizes: [] },
  // the 1x4 is the headerless bar and the 2x2 is the headed time and date block. both are module 1.
  // alwaysHeaderless is per panel: only the 1x4 bar has no header so the 2x2 keeps its H toggle
  {
    label: 'Digital Clock',
    value: 1,
    sizes: ['1x4', '2x2'],
    themeRows: [
      { size: '1x4', thumb: 'Digital Clock', alwaysHeaderless: true },
      { size: '2x2', thumb: 'Time + Date' },
      { size: '2x2', thumb: 'Big Hour' },
      { size: '2x2', thumb: 'Big Minutes' },
      { size: '2x4', thumb: 'Big Time' },
    ],
  },
  // Status (the combined battery/bluetooth/quiet panel) borrows Battery's colour set (see
  // theme_alias in C) so it groups under the Battery swatch in the appearance editor
  {
    label: 'Battery',
    value: 2,
    sizes: ['1x2', '2x2'],
    themeLabel: 'Battery',
    themeRows: [
      { size: '1x2', thumb: 'Battery' },
      { size: '2x2', thumb: 'Battery' },
      { size: '1x2', thumb: 'Status' },
    ],
  },
  { label: 'Weather', value: 3, sizes: ['1x2', '2x2'] },
  {
    label: 'Temperature',
    value: 4,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Temperature' },
      { size: '1x2', thumb: 'Feels Like' },
    ],
  },
  {
    label: 'Heart Rate',
    value: 5,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Heart Rate' },
      { size: '2x2', thumb: 'Heart Rate' },
      { size: '2x2', thumb: 'HR Graph' },
    ],
  },
  {
    label: 'Steps',
    value: 6,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Steps' },
      { size: '2x2', thumb: 'Steps' },
      { size: '2x2', thumb: 'Steps Graph' },
    ],
  },
  { label: 'Distance', value: 24, sizes: ['1x2', '2x2'] },
  { label: 'Calories', value: 7, sizes: ['1x2', '2x2'] },
  { label: 'Sleep', value: 8, sizes: ['1x2', '2x2'] },
  { label: 'Activity', value: 9, sizes: ['1x2', '2x2'] },
  { label: 'Humidity', value: 10, sizes: ['1x2'] },
  { label: 'Wind Speed', value: 11, sizes: ['1x2'] },
  { label: 'Sunrise', value: 12, sizes: ['1x2'] },
  { label: 'Sunset', value: 13, sizes: ['1x2'] },
  { label: 'Daylight', value: 14, sizes: ['2x2'], themeHidden: true },
  { label: 'Conditions', value: 15, sizes: ['2x2'] },
  { label: 'Beats', value: 16, sizes: ['1x2', '2x2'] },
  { label: 'Date', value: 17, sizes: ['1x2'] },
  { label: 'Time (1x2)', value: 18, sizes: ['1x2'], themeLabel: 'Time' },
  { label: 'Time Zone 1', value: 19, sizes: ['1x2'], themeLabel: 'Alternate Time' },
  { label: 'Analog Clock', value: 20, sizes: ['2x2'] },
  { label: 'UV', value: 21, sizes: ['1x2', '2x2'], themeLabel: 'UV Index' },
  { label: 'Hi/Low', value: 22, sizes: ['1x2'], themeLabel: 'Temperature Hi/Lo' },
  {
    label: 'Precipitation',
    value: 23,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Precipitation' },
      { size: '1x2', thumb: 'Dew Point' },
    ],
  },
  {
    label: 'Forecast (Hourly)',
    value: 25,
    sizes: ['1x4', '2x4'],
    themeLabel: 'Hourly/Daily Forecast',
    themeRows: [
      { size: '1x4', thumb: 'Forecast (Hourly)' },
      { size: '2x4', thumb: 'Forecast (Hourly)' },
      { size: '1x4', thumb: 'Forecast (4-Day)' },
      { size: '2x4', thumb: 'Forecast (4-Day)' },
    ],
  },
  { label: 'Forecast (4-Day)', value: 26, sizes: ['1x4', '2x4'], themeHidden: true },
  {
    label: 'Sun',
    value: 28,
    sizes: ['1x2', '2x2'],
    themeLabel: 'Day/Night Tracker',
    themeRows: [
      { size: '1x2', thumb: 'Sun' },
      { size: '2x2', thumb: 'Sun' },
      { size: '2x2', thumb: 'Daylight' },
    ],
  },
  // stock (2x2) and the watchlist (2x4) share one colour set but each keeps its own header/border.
  // themeRows lists the appearance editor's per-size rows and which module's screenshot each shows
  {
    label: 'Stock',
    value: 29,
    sizes: ['2x2'],
    themeLabel: 'Stock and Watchlist',
    themeRows: [
      { size: '2x2', thumb: 'Stock' },
      { size: '2x4', thumb: 'Watchlist' },
    ],
  },
  { label: 'Watchlist', value: 30, sizes: ['2x4'], themeHidden: true },
  {
    label: 'Connection',
    value: 31,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Connection' },
      { size: '1x2', thumb: 'Quiet Time' },
    ],
  },
  { label: 'HR Graph', value: 32, sizes: ['2x2'], themeHidden: true },
  { label: 'Moon', value: 33, sizes: ['1x2', '2x2'] },
  { label: 'Quiet Time', value: 34, sizes: ['1x2'], themeHidden: true },
  { label: 'Feels Like', value: 35, sizes: ['1x2'], themeHidden: true },
  { label: 'Pressure', value: 36, sizes: ['1x2'] },
  { label: 'Dew Point', value: 37, sizes: ['1x2'], themeHidden: true },
  { label: 'Steps Graph', value: 39, sizes: ['2x2'], themeHidden: true },
  // the four calendar panels share one colour set (see theme_alias in C): Next Event owns it and
  // the others borrow it so they group under one "Calendar" swatch in the appearance editor
  // themeRows grows to list all four as each panel lands
  {
    label: 'Next Event',
    value: 40,
    sizes: ['2x2', '1x4'],
    themeLabel: 'Calendar',
    themeRows: [
      { size: '2x2', thumb: 'Next Event' },
      { size: '1x4', thumb: 'Next Event' },
      { size: '2x4', thumb: 'Agenda' },
      { size: '2x2', thumb: 'Timeline' },
      { size: '1x4', thumb: 'Timeline' },
      { size: '2x4', thumb: 'Timeline' },
    ],
  },
  { label: 'Agenda', value: 41, sizes: ['2x4'], themeHidden: true },
  { label: 'Timeline', value: 42, sizes: ['1x4', '2x4', '2x2'], themeHidden: true },
  {
    label: 'Week Number',
    value: 43,
    sizes: ['1x2'],
    themeLabel: 'Week Count',
    themeRows: [
      { size: '1x2', thumb: 'Week Number' },
      { size: '1x2', thumb: 'Weeks Left' },
    ],
  },
  {
    label: 'Day of Year',
    value: 44,
    sizes: ['1x2'],
    themeLabel: 'Year Count',
    themeRows: [
      { size: '1x2', thumb: 'Day of Year' },
      { size: '1x2', thumb: 'Days Left' },
    ],
  },
  {
    label: 'Epoch Clock',
    value: 45,
    sizes: ['1x2'],
    themeLabel: 'Time Codes',
    themeRows: [
      { size: '1x2', thumb: 'Epoch Clock' },
      { size: '1x2', thumb: 'Julian Date' },
    ],
  },
  { label: 'Days Left', value: 46, sizes: ['1x2'], themeHidden: true },
  { label: 'Weekday Dots', value: 47, sizes: ['1x2'] },
  // Next Moon borrows the Moon panel's colour set (see theme_alias in C) so it hides
  // behind the Moon swatch in the appearance editor
  { label: 'Next Moon', value: 48, sizes: ['1x2'], themeHidden: true },
  { label: 'Month Grid', value: 49, sizes: ['2x4'] },
  { label: 'Julian Date', value: 50, sizes: ['1x2'], themeHidden: true },
  // Weeks Left shares the Week Number swatch (see theme_alias in C) so it hides under it
  { label: 'Weeks Left', value: 51, sizes: ['1x2'], themeHidden: true },
  { label: 'Status', value: 52, sizes: ['1x2'], themeHidden: true },
  // the big-digit night panels all ride under the Digital Clock's colour (see theme_alias in
  // C) so they hide their own swatch and group under Time in the appearance editor
  { label: 'Big Hour', value: 53, sizes: ['2x2'], themeHidden: true },
  { label: 'Big Minutes', value: 54, sizes: ['2x2'], themeHidden: true },
  { label: 'Big Time', value: 55, sizes: ['2x4'], themeHidden: true },
  // "Weather Now" (module 27) is built in C but intentionally left out of the builder until it
  // is ready to ship so it is not selectable in the layout builder yet
];

const MODULE_OPTIONS = MODULE_BASE.map(function (option) {
  const meta = moduleMeta[option.label];
  const merged: Record<string, unknown> = {};
  Object.assign(merged, option, meta);
  // the VIBRANT colour is generated per module id (vibrant.g.js) and shared with the
  // firmware so the theme editor's Use Vibrant button reads the same colour the watch paints
  const vibrant = (vibrantByType as Record<string, unknown>)[option.value];
  if (vibrant) {
    merged.vibrant = vibrant;
  }
  return merged;
});

/** A Clay select row. The description is optional so a self-evident picker can skip it. */
function select(
  messageKey: string,
  label: string,
  options: ClayOption[],
  def: string | number,
  description?: string
): ClayItem {
  const item: ClayItem = {
    type: 'select',
    messageKey: messageKey,
    label: label,
    defaultValue: def,
    options: options,
  };
  if (description) {
    item.description = description;
  }
  return item;
}

/** A section heading row. */
function heading(text: string): ClayItem {
  return { type: 'heading', defaultValue: text };
}

/** A Clay on/off toggle row. */
function toggle(messageKey: string, label: string, description: string, def: boolean): ClayItem {
  return {
    type: 'toggle',
    messageKey: messageKey,
    label: label,
    description: description,
    defaultValue: def,
  };
}

const VIBE_OPTIONS: ClayOption[] = [
  { label: 'None', value: 0 },
  { label: 'Short', value: 1 },
  { label: 'Long', value: 2 },
  { label: 'Double', value: 3 },
];

// the Goal Met Vibe picks. the value IS what the watch plays, so the tune data lives here in the
// config, not in the watch binary. a one-letter sentinel is a plain pulse (S/L/D), C means the
// Custom Vibe Pattern box below, and a comma list of milliseconds is a fanfare rhythm the watch
// buzzes straight through. None is the empty string
const GOAL_VIBE_OPTIONS: ClayOption[] = [
  { label: 'None', value: '' },
  { label: 'Short', value: 'S' },
  { label: 'Long', value: 'L' },
  { label: 'Double', value: 'D' },
  { label: 'Fanfare 7', value: '50,50,50,50,50,50,250,150,250,150,250,150,100,100,100,100,600' },
  { label: 'Fanfare 8', value: '50,60,50,60,50,60,300,180,300,180,300,180,110,110,110,110,700' },
  { label: 'Fanfare 10', value: '60,50,60,50,60,50,350,250,350,250,350,250,120,100,120,100,800' },
  { label: 'Fanfare 12', value: '70,60,70,60,70,60,400,200,400,200,400,200,150,100,150,100,900' },
  { label: 'Fanfare 13', value: '40,40,40,40,40,40,200,100,200,100,200,100,80,80,80,80,500' },
  { label: 'Custom', value: 'C' },
];

// the buzz pattern for a calendar reminder. no None here since the mode dropdown above decides
// which reminders fire (values match VIBE_OPTIONS so they land on the same VibeChoice)
const VIBE_TYPE_OPTIONS: ClayOption[] = [
  { label: 'Short', value: 1 },
  { label: 'Long', value: 2 },
  { label: 'Double', value: 3 },
];

// which event reminders fire. the per-reminder pattern selects sit under this
const CALENDAR_VIBE_MODE_OPTIONS: ClayOption[] = [
  { label: 'None', value: 0 },
  { label: '15 min, 5 min, and Start', value: 1 },
  { label: '5 min, and Start', value: 2 },
  { label: 'Start Only', value: 3 },
];

// the swap runs both ways, so the labels name both ends of it: naming only sunset reads as a
// one-way trip into night with no way back. the two clocks below double as the fixed schedule and
// as the fallback whenever the watch has no sun readings, so nobody has to be asked what happens
// when the weather has not arrived
const NIGHT_MODE_OPTIONS: ClayOption[] = [
  { label: 'Never', value: 0 },
  { label: 'At Sunset & Sunrise', value: 1 },
  { label: 'At Custom Times', value: 2 },
];

// half-hour steps, matching the slot the watch stores. a free-text time would need its own
// validation and would hit the empty-string problem the layout sentinel exists for
const HALF_HOURS: ClayOption[] = Array.from({ length: 48 }, (_, slot) => {
  const hour = Math.floor(slot / 2);
  const minute = slot % 2 ? '30' : '00';
  return { label: (hour < 10 ? '0' : '') + hour + ':' + minute, value: slot };
});

const GOAL_STEPS = [5000, 7500, 10000, 12500, 15000, 20000, 25000].map((n, i) => ({ label: n.toLocaleString(), value: i }));
const GOAL_CALORIES = [500, 1000, 1500, 2000, 2500, 3000].map((n, i) => ({ label: n + ' kcal', value: i }));
const GOAL_SLEEP = [6, 7, 8, 9, 10].map((n, i) => ({ label: n + ' h', value: i }));
const GOAL_ACTIVE = [15, 30, 45, 60, 90].map((n, i) => ({ label: n + ' min', value: i }));
const GOAL_HR = [150, 160, 170, 180, 190, 200].map((n, i) => ({ label: n + ' bpm', value: i }));
const GOAL_DISTANCE = [2, 3, 5, 8, 10, 15, 20].map((n, i) => ({ label: n + ' km', value: i }));

const config = [
  // intro lives in its own section like every other heading so it picks up the same
  // .section > .component padding. at the top level it missed that and the title bar sat cramped
  {
    type: 'section',
    items: [
      heading('Gridlock Configuration'),
      {
        type: 'text',
        defaultValue: 'Compose your layout, pick a theme, and dial in your weather and goals.',
      },
    ],
  },
  // --- Appearance ---
  {
    type: 'section',
    items: [
      heading('Appearance'),
      {
        type: 'select',
        messageKey: 'APPEARANCE_THEME',
        label: 'Theme',
        description: 'Mono (White on Black) is the classic look and Mono (Black on White) flips it. Vibrant colours each panel by what it shows. Custom lets you pick the colours yourself below.',
        defaultValue: 0,
        options: [
          { label: 'Mono (White on Black)', value: 0 },
          { label: 'Mono (Black on White)', value: 3 },
          { label: 'Vibrant', value: 1 },
          { label: 'Custom', value: 2 },
        ],
      },
      {
        type: 'themeBuilder',
        messageKey: 'APPEARANCE_CUSTOM_COLORS',
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
      {
        type: 'select',
        messageKey: 'APPEARANCE_HEADER_FONT',
        label: 'Header Font',
        description: "The font used for every panel's header label. Share Tech Mono is the default.",
        defaultValue: 0,
        options: [
          { label: 'Share Tech Mono (Default)', value: 0 },
          { label: 'LECO', value: 1 },
          { label: 'Press Start 2P', value: 2 },
          { label: 'Pixelify Sans', value: 3 },
          { label: 'Aldrich', value: 4 },
          { label: 'Kode Mono', value: 5 },
          { label: 'Electrolize', value: 6 },
          { label: 'Quantico', value: 7 },
        ],
      },
      {
        type: 'select',
        messageKey: 'APPEARANCE_PANEL_STYLE',
        label: 'Panel Style',
        description: 'How every panel is framed . Classic is the original square panel and Rounded softens the four corners.',
        defaultValue: 0,
        options: [
          { label: 'Classic (Default)', value: 0 },
          { label: 'Rounded', value: 1 },
        ],
      },
    ],
  },
  // --- Layout ---
  {
    type: 'section',
    items: [
      heading('Layout'),
      {
        type: 'text',
        defaultValue: 'Build up to four layouts and pick which two the watch uses. Tap a number to edit that layout, then drag panels in from below to place them, or drag a placed one to move or remove it. Every edit is kept as you go. Day is the layout you normally see; Night takes over on the schedule you set underneath.',
      },
      // the two invisible stores come first on purpose. Clay builds each item in order and only
      // attaches it after setting its value, so a store declared after the builder does not exist
      // yet when the builder initialises and goes looking for it.
      //
      // invisible store for the layout library: the four grids and which two are in use. lives
      // here so its value saves and restores with the rest of the settings, since the config
      // webview will not let it keep its own localStorage. the watch never reads this key
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_SLOTS',
        storeClass: 'gl-library',
        defaultValue: '',
      },
      // and the night layout itself, which the builder writes from whichever library entry is
      // assigned. this one the watch does read
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_NIGHT',
        storeClass: 'gl-night',
        defaultValue: '0',
      },
      {
        type: 'layoutBuilder',
        messageKey: 'LAYOUT',
        defaultValue: '2,0,0,2,2;12,0,2,2,1;13,1,2,2,1;1,2,0,4,1;3,3,0,2,2;6,3,2,2,2',
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
      select('LAYOUT_NIGHT_MODE', 'Swap Day & Night Layouts', NIGHT_MODE_OPTIONS, 0,
        'When the watch swaps between your two layouts. At Sunset & Sunrise follows the times your weather provider reports, and falls back to the times below whenever it has none yet.'),
      select('LAYOUT_NIGHT_START', 'Night Starts', HALF_HOURS, 42,
        'When the night layout takes over. Used for custom times, and as the fallback whenever the watch has no sunset reading.'),
      select('LAYOUT_NIGHT_END', 'Night Ends', HALF_HOURS, 14,
        'And when the day layout comes back.'),
    ],
  },
  // --- Bluetooth ---
  {
    type: 'section',
    items: [
      heading('Bluetooth Settings'),
      select('CONNECTION_VIBE_CONNECT', 'Vibrate on Connect', VIBE_OPTIONS, 0),
      select('CONNECTION_VIBE_DISCONNECT', 'Vibrate on Disconnect', VIBE_OPTIONS, 0),
    ],
  },
  // --- Clock ---
  {
    type: 'section',
    items: [
      heading('Clock Configuration'),
      select('CLOCK_TIME_FORMAT', 'Time Format', [
        { label: 'System Default', value: 0 },
        { label: '12-hour (08:30)', value: 1 },
        { label: '12-hour (8:30)', value: 4 },
        { label: '24-hour (20:30)', value: 2 },
      ], 0),
      select('CLOCK_DATE_FORMAT', 'Date Format', configBuilder.defaultDateOptions, '%a %b %d'),
      select('CLOCK_HOURLY_VIBE', 'Hourly Vibration', VIBE_OPTIONS, 0, 'Buzz at the top of every hour. Silenced automatically during Quiet Time.'),
      select('CLOCK_WEEK_START', 'Week Starts On', [
        { label: 'Sunday', value: 0 },
        { label: 'Monday', value: 1 },
      ], 0, 'The first day of the week for the Month Grid and Weekday Dots panels.'),
      select('APPEARANCE_ANALOG_STYLE', 'Analog Clock Style', [
        { label: 'Classic (Round)', value: 0 },
        { label: 'Square Frame', value: 1 },
        { label: 'Starburst', value: 2 },
        { label: 'Perimeter Track', value: 3 },
        { label: 'Segmented Bezel', value: 4 },
        { label: 'Railroad', value: 5 },
        { label: 'Dots', value: 6 },
        { label: 'Roman', value: 7 },
        { label: 'Grid', value: 8 },
      ], 0, 'The dial an Analog Clock panel draws. Add an Analog Clock to your layout to see it.'),
    ],
  },
  // --- Health ---
  {
    type: 'section',
    items: [
      heading('Health Configuration'),
      select('HEALTH_GOAL_ACTIVE', 'Activity Goal', GOAL_ACTIVE, 1),
      select('HEALTH_GOAL_CALORIES', 'Calorie Goal', GOAL_CALORIES, 3),
      select('HEALTH_GOAL_HR', 'Heart-rate Limit', GOAL_HR, 3),
      select('HEALTH_GOAL_SLEEP', 'Sleep Goal', GOAL_SLEEP, 2),
      select('HEALTH_GOAL_STEPS', 'Step Goal', GOAL_STEPS, 2),
      select('HEALTH_STEPS_MODE', 'Steps Unit', [
        { label: 'Steps', value: 0 },
        { label: 'Miles', value: 1 },
        { label: 'Kilometers', value: 2 },
      ], 0, "What the Steps panel shows: your step count, or the distance you've walked today, in miles or kilometers."),
      select('HEALTH_GOAL_DISTANCE', 'Distance Goal', GOAL_DISTANCE, 2),
      select('HEALTH_DISTANCE_UNIT', 'Distance Unit', [
        { label: 'Miles', value: 1 },
        { label: 'Kilometers', value: 0 },
      ], 0, 'The unit for the standalone Distance panel, kept apart so it can differ from the Steps panel.'),
      select('HEALTH_GOAL_VIBE', 'Goal Met Vibration', GOAL_VIBE_OPTIONS, '', 'Buzz the first time you reach a daily goal (steps, calories, distance, or active minutes). The fanfares are little celebration rhythms. Pick Custom to use your own pattern below.'),
      {
        type: 'input',
        messageKey: 'HEALTH_GOAL_VIBE_CUSTOM',
        label: 'Custom Vibe Pattern',
        description: 'Your own rhythm as comma-separated on and off times in milliseconds, starting with a buzz. Used only when Goal Met Vibration is set to Custom. Example: 100,80,100,80,300',
        attributes: {
          placeholder: '100,80,100,80,300',
          limit: 120,
        },
      },
    ],
  },
  // --- Location & Time Zone ---
  {
    type: 'section',
    items: [
      heading('Location Settings'),
      toggle('LOCATION_USE_GPS', 'Enable Phone GPS', 'Automatically fetch weather for your current location.', true),
      toggle('LOCATION_GPS_FALLBACK', 'Fallback to Manual Location', 'If GPS is disabled or unavailable, use the city typed below.', true),
      {
        type: 'locationsearch',
        messageKey: 'LOCATION_NAME',
        label: 'Manual Location',
        attributes: {
          placeholder: 'Search a city, e.g. Phoenix',
        },
      },
      {
        type: 'locationsearch',
        messageKey: 'CLOCK_TIMEZONE_1',
        label: 'Alternate Time Zone',
        description: "Sets the local time displayed by the 'Alternate Time' module in your layout.",
        attributes: {
          placeholder: 'Search a city, e.g. Phoenix',
        },
      },
    ],
  },
  // --- Weather ---
  {
    type: 'section',
    items: [
      heading('Weather Preferences'),
      select('WEATHER_TEMPERATURE_UNIT', 'Temperature Unit', [
        { label: 'Celsius (°C)', value: 0 },
        { label: 'Fahrenheit (°F)', value: 1 },
      ], 0),
      select('WEATHER_WIND_UNIT', 'Wind Speed Unit', [
        { label: 'Kilometers/hour (km/h)', value: 0 },
        { label: 'Miles/hour (mph)', value: 1 },
        { label: 'Knots (kts)', value: 2 },
        { label: 'Meters/second (m/s)', value: 3 },
      ], 0),
      select('WEATHER_PROVIDER', 'Data Source', [
        { label: 'Open-Meteo (Free, No Key Required)', value: 'openmeteo' },
        { label: 'OpenWeatherMap', value: 'owm' },
        { label: 'WeatherAPI.com', value: 'weatherapi' },
      ], 'openmeteo', 'Choose where your watch pulls its weather data. Open-Meteo works right out of the box with no setup required.'),
      {
        type: 'input',
        messageKey: 'WEATHER_API_KEY',
        label: 'API Key',
        description: 'Only required if you selected OpenWeatherMap or WeatherAPI above.',
        attributes: {
          placeholder: 'Paste your private API key here...',
          limit: 64,
        },
      },
    ],
  },
  // --- Calendar ---
  {
    type: 'section',
    items: [
      heading('Calendar'),
      {
        type: 'text',
        defaultValue: 'Add a Calendar panel (Agenda, Countdown, or Free/Busy) to your layout, then paste your private iCal link below.',
      },
      {
        type: 'input',
        messageKey: 'CALENDAR_ICS_URL',
        label: 'iCal URL',
        description: 'Paste any calendar\'s private iCal (.ics) address. Google Calendar: Settings and sharing, Integrate calendar, "Secret address in iCal format". Apple iCloud: share a calendar as Public. Outlook: publish the calendar. Treat the link like a password: anyone with it can read your calendar.',
        attributes: {
          placeholder: 'https://.../calendar.ics',
          limit: 512,
        },
      },
      select('CALENDAR_POLL', 'Refresh Interval', [
        { label: '5 min', value: 0 },
        { label: '10 min', value: 1 },
        { label: '15 min', value: 2 },
        { label: '20 min', value: 3 },
        { label: '30 min', value: 4 },
        { label: '1 hour', value: 5 },
      ], 4, 'How often the watch pulls a fresh copy of your calendar feed. A tighter interval uses more battery.'),
      select('CALENDAR_VIBE_MODE', 'Event Reminders', CALENDAR_VIBE_MODE_OPTIONS, 0, 'Buzz before a timed event starts. Silenced during Quiet Time, and only when a Calendar panel is on your layout. Pick the pattern for each reminder below.'),
      select('CALENDAR_VIBE_15MIN', 'Reminder: 15m Vibration', VIBE_TYPE_OPTIONS, 1),
      select('CALENDAR_VIBE_5MIN', 'Reminder: 5m Vibration', VIBE_TYPE_OPTIONS, 3),
      select('CALENDAR_VIBE_START', 'Reminder: Start Vibration', VIBE_TYPE_OPTIONS, 2),
    ],
  },
  // --- Stocks ---
  {
    type: 'section',
    items: [
      heading('Stock Preferences'),
      {
        type: 'text',
        defaultValue: 'Add a Stock or Watchlist panel to your layout, then set your ticker(s) below. Most sources need a free API key (Yahoo does not).',
      },
      select('STOCK_PROVIDER', 'Data Source', [
        { label: 'Finnhub (Real-Time, Free Key)', value: 'finnhub' },
        { label: 'Yahoo (Real-Time, No Key)', value: 'yahoo' },
        { label: 'Twelve Data (Global, Free Key)', value: 'twelvedata' },
        { label: 'Alpha Vantage (End-of-Day, Free Key)', value: 'alphavantage' },
      ], 'finnhub', 'Finnhub gives real-time US quotes and is the one to use for a live watchlist. Yahoo is real-time too and needs no key, with the widest market coverage, but it is an unofficial feed that can change without notice. Twelve Data adds global markets on a free key and refreshes on your interval while the market is open. Alpha Vantage is end-of-day only, so it grabs the day\'s close once after the closing bell.'),
      {
        type: 'input',
        messageKey: 'STOCK_API_KEY',
        label: 'API Key',
        description: 'Grab a free key from your chosen provider and paste it here. Yahoo needs no key, so leave this blank when you pick it.',
        attributes: {
          placeholder: 'Paste your private API key here...',
          limit: 64,
        },
      },
      {
        type: 'input',
        messageKey: 'STOCK_SYMBOLS',
        label: 'Tickers',
        description: 'One symbol for the 2x2 Stock panel, or up to four comma-separated for the Watchlist (e.g. AAPL, MSFT, TSLA, NVDA).',
        attributes: {
          placeholder: 'AAPL, MSFT, TSLA, NVDA',
          limit: 64,
        },
      },
      select('STOCK_POLL', 'Refresh Interval', [
        { label: '1 min', value: 0 },
        { label: '5 min', value: 1 },
        { label: '10 min', value: 2 },
        { label: '15 min', value: 3 },
        { label: '20 min', value: 4 },
        { label: '30 min', value: 5 },
      ], 5, 'How often Finnhub and Yahoo pull fresh quotes. A tighter interval drains your battery faster. Twelve Data follows this too while the market is open (down to every 15 minutes to protect its daily cap), then eases off after hours. Alpha Vantage ignores it and just grabs the close once a day after the market shuts.'),
    ],
  },

  {
    type: 'submit',
    defaultValue: 'Save & Apply to Watch',
  },
];

export default config;
