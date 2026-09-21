import configBuilder from '../../../../../lib/ts/pkjs/config-builder';
import resetFaceColors from './clay/reset-face-colors';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';
import vibrantByType from './clay/vibrant.g';
import layoutPresets from '../data/layout-presets.json';
import type { ClayItem } from '../../../core/pkjs/types';
import { nightScheduleItems } from '../../../core/pkjs/night-schedule';
import { select, heading, toggle, VIBE_OPTIONS } from '../../../core/pkjs/config-rows';
import { healthSection } from '../../../core/pkjs/health-section';
import { locationSection } from '../../../core/pkjs/location-section';
import { weatherSection } from '../../../core/pkjs/weather-section';

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
  { label: 'Next Alarm', value: 56, sizes: ['1x2', '2x2'] },
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
        description: 'How every panel is framed. Classic is the original square panel and Rounded softens the four corners.',
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
        defaultValue: 'Build up to five layouts. Panels sit in the two rows above the hour pointer and the two below, since the reel owns the rest of the screen. Day is the one you normally see. Night takes over on the schedule below, and Quiet Time takes over whenever the watch is on Quiet Time.',
      },
      // the three stores come first on purpose. Clay builds each item in order and only attaches
      // it after setting its value, so a store declared after the builder does not exist yet when
      // the builder initialises and goes looking for it.
      //
      // the library holds every layout and which of them has which job. the watch never reads it
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_SLOTS',
        storeClass: 'gl-library',
        defaultValue: '',
      },
      // the night and Quiet Time layouts, which the builder writes from whichever library entry
      // is assigned. these two the watch does read
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_NIGHT',
        storeClass: 'gl-night',
        defaultValue: '0',
      },
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_QUIET',
        storeClass: 'gl-quiet',
        defaultValue: '0',
      },
      {
        type: 'layoutBuilder',
        messageKey: 'LAYOUT',
        defaultValue: layoutPresets.default,
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
      ...nightScheduleItems(),
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
      ], 0, 'The first day of the week for the Weekday Dots panel.'),
    ],
  },
  healthSection(),
  locationSection(),
  weatherSection(),

  {
    type: 'submit',
    defaultValue: 'Save & Apply to Watch',
  },
];

// the code Clay runs inside the config page itself, named beside the rows it wires up so both
// the runtime and the preview tool find it the same way
export const customClay = resetFaceColors;

export default config;
