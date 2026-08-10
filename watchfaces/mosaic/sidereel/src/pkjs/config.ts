import configBuilder from '../../../../../lib/ts/pkjs/config-builder';
import resetFaceColors from './clay/reset-face-colors';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';
import vibrantByType from './clay/vibrant.g';
import layoutPresets from '../data/layout-presets.json';
import type { ClayItem } from '../../../core/pkjs/types';

/** A select/toggle option: its label and the value Clay stores. */
type ClayOption = { label: string; value: string | number };

// base list. icon/colour get merged in from module-meta below so the builders read one source.
//
// the panels this face carries, each keeping gridlock's value, sizes and theme grouping so the
// two agree on what a number means. the ones this face has no cell for (anything 1x4 or 2x4, and
// the stock, calendar and analog panels) are absent rather than reshaped
const MODULE_BASE = [
  { label: 'Empty', value: 0, sizes: [] },
  // gridlock groups Status under Battery's swatch. this face has no Status panel, so Battery is
  // left with its own two rows
  {
    label: 'Battery',
    value: 2,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Battery' },
      { size: '2x2', thumb: 'Battery' },
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
  { label: 'Time Zone 1', value: 19, sizes: ['1x2'], themeLabel: 'Alternate Time' },
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
  { label: 'HR Graph', value: 32, sizes: ['2x2'], themeHidden: true },
  { label: 'Moon', value: 33, sizes: ['1x2', '2x2'] },
  { label: 'Feels Like', value: 35, sizes: ['1x2'], themeHidden: true },
  { label: 'Pressure', value: 36, sizes: ['1x2'] },
  { label: 'Dew Point', value: 37, sizes: ['1x2'], themeHidden: true },
  { label: 'Steps Graph', value: 39, sizes: ['2x2'], themeHidden: true },
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
  { label: 'Julian Date', value: 50, sizes: ['1x2'], themeHidden: true },
  // Weeks Left shares the Week Number swatch (see theme_alias in C) so it hides under it
  { label: 'Weeks Left', value: 51, sizes: ['1x2'], themeHidden: true },
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

/** A Clay colour swatch row. Sunlight is off so the grid shows the colours as the watch paints them. */
function color(messageKey: string, label: string, description: string, def: number): ClayItem {
  return {
    type: 'color',
    messageKey: messageKey,
    label: label,
    description: description,
    defaultValue: def,
    sunlight: false,
  };
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

const GOAL_STEPS = [5000, 7500, 10000, 12500, 15000, 20000, 25000].map((n, i) => ({ label: n.toLocaleString(), value: i }));
const GOAL_CALORIES = [500, 1000, 1500, 2000, 2500, 3000].map((n, i) => ({ label: n + ' kcal', value: i }));
const GOAL_SLEEP = [6, 7, 8, 9, 10].map((n, i) => ({ label: n + ' h', value: i }));
const GOAL_ACTIVE = [15, 30, 45, 60, 90].map((n, i) => ({ label: n + ' min', value: i }));
const GOAL_HR = [150, 160, 170, 180, 190, 200].map((n, i) => ({ label: n + ' bpm', value: i }));
const GOAL_DISTANCE = [2, 3, 5, 8, 10, 15, 20].map((n, i) => ({ label: n + ' km', value: i }));

const config = [
  // intro lives in its own section like every other heading so it picks up the same
  // .section > .component padding. at the top level it misses that and the title bar sits cramped
  {
    type: 'section',
    items: [
      heading('Sidereel Configuration'),
      {
        type: 'text',
        defaultValue: 'A reel of minutes read by an hour pointer. Drag panels onto the left of the face, pick a theme, and dial in your weather and goals.',
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
        description: 'Mono (White on Black) is the classic look and Mono (Black on White) flips it. Vibrant colours each panel by what it shows and gives the pointer its red. Custom lets you pick the colours yourself below.',
        defaultValue: 0,
        options: [
          { label: 'Mono (White on Black)', value: 0 },
          { label: 'Mono (Black on White)', value: 3 },
          { label: 'Vibrant', value: 1 },
          { label: 'Custom', value: 2 },
        ],
      },
      // the four below paint over whichever theme is running rather than belonging to Custom, so
      // a mono face can carry a red pointer. the switch is what keeps the presets intact until
      // you ask for it, since there is no one default that suits all four themes
      toggle('APPEARANCE_FACE_COLORS', 'Set Face Colours Yourself',
        'Paint the reel and the hour pointer with the four colours below, whichever theme is picked. Off, each theme keeps its own.', false),
      color('APPEARANCE_POINTER_COLOR', 'Pointer Colour',
        'The hour pointer on the left.', 0xFF0000),
      color('APPEARANCE_POINTER_INK', 'Pointer Text Colour',
        'The hour and the status glyphs sitting on the pointer.', 0xFFFFFF),
      color('APPEARANCE_REEL_COLOR', 'Reel Colour',
        'The reel of minutes on the right, and the run of sprocket holes beside it.', 0xFFFFFF),
      color('APPEARANCE_REEL_INK', 'Reel Text Colour',
        'The minutes on the reel.', 0x000000),
      {
        type: 'button',
        id: 'resetFaceColors',
        defaultValue: 'Reset Face Colours',
        description: 'Puts the four colours above back to their starting values.',
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
      // this face draws its own status glyphs in the hour pointer rather than carrying gridlock's
      // Quiet Time panel, so the toggle for one lives here beside the rest of the look
      toggle('APPEARANCE_QUIET_TIME_ICON', 'Show Quiet Time Icon',
        'Show a muted-speaker glyph in the hour pointer while Quiet Time is on.', false),
    ],
  },
  // --- Layout ---
  {
    type: 'section',
    items: [
      heading('Layout'),
      {
        type: 'text',
        defaultValue: 'Drag panels in from below to place them, or drag a placed one to move or remove it. The reel and the hour pointer own the rest of the screen, so panels sit in the two rows above the pointer and the two below.',
      },
      {
        type: 'layoutBuilder',
        messageKey: 'LAYOUT',
        defaultValue: layoutPresets.default,
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
    ],
  },
  // --- Bluetooth ---
  {
    type: 'section',
    items: [
      heading('Bluetooth Settings'),
      // the other half of the pointer's status pair, kept with the connection settings it reports on
      toggle('CONNECTION_BLUETOOTH_ICON', 'Show Connection Icon',
        'Show a bluetooth glyph in the hour pointer, lit while the phone is connected and slashed when it drops.', true),
      select('CONNECTION_VIBE_CONNECT', 'Vibrate on Connect', VIBE_OPTIONS, 0),
      select('CONNECTION_VIBE_DISCONNECT', 'Vibrate on Disconnect', VIBE_OPTIONS, 0),
    ],
  },
  // --- Clock ---
  {
    type: 'section',
    items: [
      heading('Clock Configuration'),
      // .beats is left out of the list gridlock offers: the reel reads minutes, so a beat count
      // has nowhere to land on this face
      select('CLOCK_TIME_FORMAT', 'Time Format', [
        { label: 'System Default', value: 0 },
        { label: '12-hour (08:30)', value: 1 },
        { label: '12-hour (8:30)', value: 4 },
        { label: '24-hour (20:30)', value: 2 },
      ], 0),
      select('CLOCK_DATE_FORMAT', 'Date Format', configBuilder.defaultDateOptions, '%b %d'),
      select('CLOCK_HOURLY_VIBE', 'Hourly Vibration', VIBE_OPTIONS, 0, 'Buzz at the top of every hour. Silenced automatically during Quiet Time.'),
      select('CLOCK_WEEK_START', 'Week Starts On', [
        { label: 'Sunday', value: 0 },
        { label: 'Monday', value: 1 },
      ], 0, 'The first day of the week for the Week Number and Weekday Dots panels.'),
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
        description: "Sets the local time displayed by the 'Time Zone 1' module in your layout.",
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

  {
    type: 'submit',
    defaultValue: 'Save & Apply to Watch',
  },
];

// the code Clay runs inside the config page itself, named beside the rows it wires up so both
// the runtime and the preview tool find it the same way
export const customClay = resetFaceColors;

export default config;
