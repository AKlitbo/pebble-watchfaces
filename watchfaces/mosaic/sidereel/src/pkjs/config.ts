import configBuilder from '../../../paf/ts/pkjs/config-builder';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';
import moduleGroups from '../../../core/pkjs/module-groups';
import vibrantByType from './clay/vibrant.g';
import layoutPresets from '../data/layout-presets.json';
import type { ClayConfigItem } from '../../../paf/ts/clay/types';
import { nightScheduleItems } from '../../../core/pkjs/night-schedule';
import { select, heading, toggle, VIBE_OPTIONS } from '../../../paf/ts/pkjs/config-rows';
import { healthSection } from '../../../core/pkjs/health-section';
import { locationSection, timeZoneItem } from '../../../core/pkjs/location-section';
import { weatherSection } from '../../../core/pkjs/weather-section';
import { hourlyVibeItem } from '../../../core/pkjs/clock-rows';
import { headerFontItem, panelStyleItem } from '../../../core/pkjs/appearance-rows';

// base list. icon/colour get merged in from module-meta below so the builders read one source.
//
// the panels this face carries, each keeping gridlock's value, sizes and theme grouping so the
// two agree on what a number means. the ones this face has no cell for (anything 1x4 or 2x4, and
// the stock, calendar and analog panels) are absent rather than reshaped
const MODULE_BASE = [
  { label: 'Empty', value: 0, sizes: [] },
  { label: 'Time Zone 1', value: 19, sizes: ['1x2'], themeLabel: 'Alternate Time' },
  { label: 'Beats', value: 16, sizes: ['1x2', '2x2'] },
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
  { label: 'Julian Date', value: 50, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Day Progress',
    value: 58,
    sizes: ['1x2', '2x2'],
    themeLabel: 'Time Progress',
    themeRows: [
      { size: '1x2', thumb: 'Day Progress' },
      { size: '2x2', thumb: 'Day Progress' },
      { size: '1x2', thumb: 'Hour Progress' },
      { size: '2x2', thumb: 'Hour Progress' },
    ],
  },
  { label: 'Hour Progress', value: 59, sizes: ['1x2', '2x2'], themeHidden: true },
  { label: 'Date', value: 17, sizes: ['1x2'] },
  { label: 'Weekday Dots', value: 47, sizes: ['1x2'] },
  {
    label: 'Week Number',
    value: 43,
    sizes: ['1x2'],
    themeLabel: 'Week Count',
    themeRows: [
      { size: '1x2', thumb: 'Week Number' },
      { size: '1x2', thumb: 'Weeks Left' },
      { size: '1x2', thumb: 'Week Progress' },
      { size: '2x2', thumb: 'Week Progress' },
    ],
  },
  // Weeks Left shares the Week Number swatch (see theme_alias in C) so it hides under it
  { label: 'Weeks Left', value: 51, sizes: ['1x2'], themeHidden: true },
  { label: 'Week Progress', value: 60, sizes: ['1x2', '2x2'], themeHidden: true },
  {
    label: 'Day of Year',
    value: 44,
    sizes: ['1x2'],
    themeLabel: 'Year Count',
    themeRows: [
      { size: '1x2', thumb: 'Day of Year' },
      { size: '1x2', thumb: 'Days Left' },
      { size: '1x2', thumb: 'Year Progress' },
      { size: '2x2', thumb: 'Year Progress' },
    ],
  },
  { label: 'Days Left', value: 46, sizes: ['1x2'], themeHidden: true },
  // Year Progress and Week Progress borrow the Day of Year and Week Number colour sets (see
  // theme_alias in C) so they hide under the Year Count and Week Count swatches. Hour Progress
  // borrows Day Progress's, so the two share a Time Progress swatch. each panel still gets its own
  // rows because the header and border toggles act on the module a row names, not the one it
  // borrows from
  { label: 'Year Progress', value: 57, sizes: ['1x2', '2x2'], themeHidden: true },
  { label: 'Next Alarm', value: 56, sizes: ['1x2', '2x2'] },
  { label: 'Weather', value: 3, sizes: ['1x2', '2x2'] },
  { label: 'Conditions', value: 15, sizes: ['2x2'] },
  {
    label: 'Temperature',
    value: 4,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Temperature' },
      { size: '1x2', thumb: 'Feels Like' },
    ],
  },
  { label: 'Feels Like', value: 35, sizes: ['1x2'], themeHidden: true },
  { label: 'Hi/Low', value: 22, sizes: ['1x2'], themeLabel: 'Temperature Hi/Lo' },
  { label: 'Humidity', value: 10, sizes: ['1x2'] },
  {
    label: 'Precipitation',
    value: 23,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Precipitation' },
      { size: '1x2', thumb: 'Dew Point' },
    ],
  },
  { label: 'Dew Point', value: 37, sizes: ['1x2'], themeHidden: true },
  { label: 'Wind Speed', value: 11, sizes: ['1x2'] },
  { label: 'Pressure', value: 36, sizes: ['1x2'] },
  { label: 'UV', value: 21, sizes: ['1x2', '2x2'], themeLabel: 'UV Index' },
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
  { label: 'Sunrise', value: 12, sizes: ['1x2'] },
  { label: 'Sunset', value: 13, sizes: ['1x2'] },
  { label: 'Daylight', value: 14, sizes: ['2x2'], themeHidden: true },
  // Next Moon borrows the Moon panel's colour set (see theme_alias in C) so it hides behind the
  // Moon swatch in the appearance editor. it still gets its own row there, because the header and
  // border toggles act on the module a row names
  {
    label: 'Moon',
    value: 33,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Moon' },
      { size: '2x2', thumb: 'Moon' },
      { size: '1x2', thumb: 'Next Moon' },
    ],
  },
  { label: 'Next Moon', value: 48, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Steps',
    value: 6,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Steps' },
      { size: '2x2', thumb: 'Steps' },
      { size: '2x2', thumb: 'Steps Graph' },
      { size: '1x2', thumb: 'Step Goal' },
    ],
  },
  // the goal bar panels borrow their health panel's colour set (see theme_alias in C), so each hides
  // under that panel's swatch. each still gets its own row there, because the header and border
  // toggles act on the module a row names
  { label: 'Step Goal', value: 61, sizes: ['1x2'], themeHidden: true },
  { label: 'Steps Graph', value: 39, sizes: ['2x2'], themeHidden: true },
  {
    label: 'Distance',
    value: 24,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Distance' },
      { size: '2x2', thumb: 'Distance' },
      { size: '1x2', thumb: 'Distance Goal' },
    ],
  },
  { label: 'Distance Goal', value: 62, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Calories',
    value: 7,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Calories' },
      { size: '2x2', thumb: 'Calories' },
      { size: '1x2', thumb: 'Calorie Goal' },
    ],
  },
  { label: 'Calorie Goal', value: 63, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Activity',
    value: 9,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Activity' },
      { size: '2x2', thumb: 'Activity' },
      { size: '1x2', thumb: 'Activity Goal' },
    ],
  },
  { label: 'Activity Goal', value: 65, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Sleep',
    value: 8,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Sleep' },
      { size: '2x2', thumb: 'Sleep' },
      { size: '1x2', thumb: 'Sleep Goal' },
    ],
  },
  { label: 'Sleep Goal', value: 64, sizes: ['1x2'], themeHidden: true },
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
  { label: 'HR Graph', value: 32, sizes: ['2x2'], themeHidden: true },
  // Battery Battery (the watch and phone batteries side by side) borrows Battery's colour set (see
  // theme_alias in C) so it groups under the Battery swatch in the appearance editor, and shows only
  // there, though its phone half takes Phone Battery's colours, since a second row under Phone Battery
  // could not be styled. this face has no Status panel, which gridlock also groups under Battery
  {
    label: 'Battery',
    value: 2,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Battery' },
      { size: '2x2', thumb: 'Battery' },
      { size: '1x2', thumb: 'Battery Battery' },
    ],
  },
  { label: 'Battery Battery', value: 76, sizes: ['1x2'], themeHidden: true },
  // the phone's battery comes from the Pebble app's Phone plugin, so it reads PLUGINS OFF with plugins
  // off. its own rows come first, since the layout builder takes a size's picture from the first match
  {
    label: 'Phone Battery',
    value: 75,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Phone Battery' },
      { size: '2x2', thumb: 'Phone Battery' },
    ],
  },
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

  // the palette group is the family's, shared by every face, so it comes from its own table
  const group = moduleGroups[option.label];

  if (group) {
    merged.group = group;
  }

  return merged;
});

/** A Clay colour swatch row. Sunlight is off so the grid shows the colours as the watch paints them. */
// the face colours and their reset only do anything with Set Face Colours Yourself on
const FACE_COLORS_ON = { key: 'APPEARANCE_FACE_COLORS', in: [true] };

function color(messageKey: string, label: string, description: string, def: number): ClayConfigItem {
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
      heading('Sidereel Settings'),
      {
        type: 'text',
        defaultValue: 'A reel of minutes read by an hour pointer. Pick a theme and build your layouts first. The sections below set up what your panels show.',
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
        description: 'Vibrant colours each panel by what it shows and gives the pointer its red. Custom uses the panel colours you set in the editor below.',
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
        'Paint the reel and the hour pointer with four colours of your own, whichever theme is picked. Off, each theme keeps its own.', false),
      { ...color('APPEARANCE_POINTER_COLOR', 'Pointer Colour',
        'The hour pointer on the left.', 0xFF0000), visibleWhen: FACE_COLORS_ON },
      { ...color('APPEARANCE_POINTER_INK', 'Pointer Text Colour',
        'The hour and the status glyphs sitting on the pointer.', 0xFFFFFF), visibleWhen: FACE_COLORS_ON },
      { ...color('APPEARANCE_REEL_COLOR', 'Reel Colour',
        'The reel of minutes on the right, and the run of sprocket holes beside it.', 0xFFFFFF), visibleWhen: FACE_COLORS_ON },
      { ...color('APPEARANCE_REEL_INK', 'Reel Text Colour',
        'The minutes on the reel.', 0x000000), visibleWhen: FACE_COLORS_ON },
      {
        type: 'resetButton',
        id: 'resetFaceColors',
        defaultValue: 'Reset Face Colours',
        description: 'Puts the four colours above back to their starting values.',
        resetKeys: ['APPEARANCE_POINTER_COLOR', 'APPEARANCE_POINTER_INK', 'APPEARANCE_REEL_COLOR', 'APPEARANCE_REEL_INK'],
        visibleWhen: FACE_COLORS_ON,
      },
      {
        type: 'themeBuilder',
        messageKey: 'APPEARANCE_CUSTOM_COLORS',
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
      headerFontItem(),
      panelStyleItem(),
      // this face draws its own status glyphs in the hour pointer rather than carrying gridlock's
      // Quiet Time panel, so the toggle for one lives here beside the rest of the look
      toggle('APPEARANCE_QUIET_TIME_ICON', 'Show Quiet Time Icon',
        'Show a muted-speaker glyph in the hour pointer while Quiet Time is on.', false),
      toggle('APPEARANCE_REDUCE_MOTION', 'Reduce Motion',
        'Show each new minute straight away rather than scrolling the reel to it.', false),
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
      heading('Bluetooth'),
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
      heading('Clock'),
      // .beats is left out of the list gridlock offers: the reel reads minutes, so a beat count
      // has nowhere to land on this face
      select('CLOCK_TIME_FORMAT', 'Time Format', [
        { label: 'System Default', value: 0 },
        { label: '12-hour (08:30)', value: 1 },
        { label: '12-hour (8:30)', value: 4 },
        { label: '24-hour (20:30)', value: 2 },
      ], 0),
      select('CLOCK_DATE_FORMAT', 'Date Format', configBuilder.defaultDateOptions, '%b %d'),
      hourlyVibeItem(),
      select('CLOCK_WEEK_START', 'Week Starts On', [
        { label: 'Sunday', value: 0 },
        { label: 'Monday', value: 1 },
      ], 0, 'The first day of the week for the Weekday Dots panel.'),
      timeZoneItem(),
    ],
  },
  healthSection(),
  weatherSection(),
  // straight under Weather, since every row in it only feeds the weather
  locationSection(),

  {
    type: 'submit',
    defaultValue: 'Save & Apply to Watch',
  },
];

export default config;
