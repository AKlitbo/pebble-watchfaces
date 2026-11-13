import configBuilder from '../../../paf/ts/pkjs/config-builder';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';
import moduleGroups from '../../../core/pkjs/module-groups';
import vibrantByType from './clay/vibrant.g';
import type { ClayOption } from '../../../paf/ts/clay/types';
import { nightScheduleItems } from '../../../core/pkjs/night-schedule';
import { select, heading, VIBE_OPTIONS } from '../../../paf/ts/pkjs/config-rows';
import { healthSection } from '../../../core/pkjs/health-section';
import { locationSection, timeZoneItem } from '../../../core/pkjs/location-section';
import { weatherSection } from '../../../core/pkjs/weather-section';
import { hourlyVibeItem } from '../../../core/pkjs/clock-rows';
import { headerFontItem, panelStyleItem } from '../../../core/pkjs/appearance-rows';

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
  // the big-digit night panels all ride under the Digital Clock's colour (see theme_alias in
  // C) so they hide their own swatch and group under Time in the appearance editor
  { label: 'Big Hour', value: 53, sizes: ['2x2'], themeHidden: true },
  { label: 'Big Minutes', value: 54, sizes: ['2x2'], themeHidden: true },
  { label: 'Big Time', value: 55, sizes: ['2x4'], themeHidden: true },
  { label: 'Time (1x2)', value: 18, sizes: ['1x2'], themeLabel: 'Time' },
  { label: 'Analog Clock', value: 20, sizes: ['2x2'] },
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
    sizes: ['1x2', '2x2', '1x4'],
    themeLabel: 'Time Progress',
    themeRows: [
      { size: '1x2', thumb: 'Day Progress' },
      { size: '2x2', thumb: 'Day Progress' },
      { size: '1x4', thumb: 'Day Progress' },
      { size: '1x2', thumb: 'Hour Progress' },
      { size: '2x2', thumb: 'Hour Progress' },
      { size: '1x4', thumb: 'Hour Progress' },
    ],
  },
  { label: 'Hour Progress', value: 59, sizes: ['1x2', '2x2', '1x4'], themeHidden: true },
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
      { size: '1x4', thumb: 'Week Progress' },
    ],
  },
  // Weeks Left shares the Week Number swatch (see theme_alias in C) so it hides under it
  { label: 'Weeks Left', value: 51, sizes: ['1x2'], themeHidden: true },
  { label: 'Week Progress', value: 60, sizes: ['1x2', '2x2', '1x4'], themeHidden: true },
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
      { size: '1x4', thumb: 'Year Progress' },
    ],
  },
  { label: 'Days Left', value: 46, sizes: ['1x2'], themeHidden: true },
  // Year Progress and Week Progress borrow the Day of Year and Week Number colour sets (see
  // theme_alias in C) so they hide under the Year Count and Week Count swatches. Hour Progress
  // borrows Day Progress's, so the two share a Time Progress swatch. each panel still gets its own
  // rows because the header and border toggles act on the module a row names, not the one it
  // borrows from
  { label: 'Year Progress', value: 57, sizes: ['1x2', '2x2', '1x4'], themeHidden: true },
  { label: 'Month Grid', value: 49, sizes: ['2x4'] },
  { label: 'Next Alarm', value: 56, sizes: ['1x2', '2x2'] },
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
  { label: 'Timeline', value: 42, sizes: ['1x4', '2x4', '2x2'], themeHidden: true },
  { label: 'Agenda', value: 41, sizes: ['2x4'], themeHidden: true },
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
      { size: '1x4', thumb: 'Step Goal' },
    ],
  },
  // the goal bar panels borrow their health panel's colour set (see theme_alias in C), so each hides
  // under that panel's swatch. each still gets its own row there, because the header and border
  // toggles act on the module a row names
  { label: 'Step Goal', value: 61, sizes: ['1x2', '1x4'], themeHidden: true },
  { label: 'Steps Graph', value: 39, sizes: ['2x2'], themeHidden: true },
  {
    label: 'Distance',
    value: 24,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Distance' },
      { size: '2x2', thumb: 'Distance' },
      { size: '1x2', thumb: 'Distance Goal' },
      { size: '1x4', thumb: 'Distance Goal' },
    ],
  },
  { label: 'Distance Goal', value: 62, sizes: ['1x2', '1x4'], themeHidden: true },
  {
    label: 'Calories',
    value: 7,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Calories' },
      { size: '2x2', thumb: 'Calories' },
      { size: '1x2', thumb: 'Calorie Goal' },
      { size: '1x4', thumb: 'Calorie Goal' },
    ],
  },
  { label: 'Calorie Goal', value: 63, sizes: ['1x2', '1x4'], themeHidden: true },
  {
    label: 'Activity',
    value: 9,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Activity' },
      { size: '2x2', thumb: 'Activity' },
      { size: '1x2', thumb: 'Activity Goal' },
      { size: '1x4', thumb: 'Activity Goal' },
    ],
  },
  { label: 'Activity Goal', value: 65, sizes: ['1x2', '1x4'], themeHidden: true },
  {
    label: 'Sleep',
    value: 8,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Sleep' },
      { size: '2x2', thumb: 'Sleep' },
      { size: '1x2', thumb: 'Sleep Goal' },
      { size: '1x4', thumb: 'Sleep Goal' },
      { size: '1x2', thumb: 'Night HRV' },
    ],
  },
  { label: 'Night HRV', value: 69, sizes: ['1x2'], themeHidden: true },
  { label: 'Sleep Goal', value: 64, sizes: ['1x2', '1x4'], themeHidden: true },
  {
    label: 'Heart Rate',
    value: 5,
    sizes: ['1x2', '2x2'],
    themeRows: [
      { size: '1x2', thumb: 'Heart Rate' },
      { size: '2x2', thumb: 'Heart Rate' },
      { size: '2x2', thumb: 'HR Graph' },
      { size: '2x4', thumb: 'BPM Today' },
      { size: '1x2', thumb: 'HRV' },
      { size: '1x2', thumb: 'HRV Trend' },
      { size: '2x2', thumb: 'Heart' },
      { size: '2x2', thumb: 'HRV History' },
      { size: '2x2', thumb: 'Still vs Walking' },
      { size: '2x4', thumb: 'HRV Tuning' },
      { size: '2x4', thumb: 'HRV Today' },
    ],
  },
  { label: 'HR Graph', value: 32, sizes: ['2x2'], themeHidden: true },
  { label: 'BPM Today', value: 74, sizes: ['1x4', '2x4'], themeHidden: true },
  { label: 'HRV', value: 66, sizes: ['1x2'], themeHidden: true },
  { label: 'HRV Trend', value: 67, sizes: ['1x2'], themeHidden: true },
  { label: 'Heart', value: 68, sizes: ['2x2'], themeHidden: true },
  { label: 'HRV History', value: 70, sizes: ['2x2'], themeHidden: true },
  { label: 'Still vs Walking', value: 71, sizes: ['2x2'], themeHidden: true },
  { label: 'HRV Today', value: 72, sizes: ['1x4', '2x4'], themeHidden: true },
  { label: 'HRV Tuning', value: 73, sizes: ['2x4'], themeHidden: true },
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
  { label: 'Status', value: 52, sizes: ['1x2'], themeHidden: true },
  {
    label: 'Connection',
    value: 31,
    sizes: ['1x2'],
    themeRows: [
      { size: '1x2', thumb: 'Connection' },
      { size: '1x2', thumb: 'Quiet Time' },
    ],
  },
  { label: 'Quiet Time', value: 34, sizes: ['1x2'], themeHidden: true },
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

  // the palette group is the family's, shared by every face, so it comes from its own table
  const group = moduleGroups[option.label];

  if (group) {
    merged.group = group;
  }

  return merged;
});

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
  { label: '5 min and Start', value: 2 },
  { label: 'Start Only', value: 3 },
];

// the sections follow how a wearer sets the face up. build the layout, give it a look, set the
// clock, then make each kind of panel work. the sources that need a key, a link, or symbols come
// ahead of Health, whose defaults work untouched
const config = [
  // intro lives in its own section like every other heading so it picks up the same
  // .section > .component padding. at the top level it misses that and the title bar sits cramped
  {
    type: 'section',
    items: [
      heading('Gridlock Settings'),
      {
        type: 'text',
        defaultValue: 'Build your layouts and pick a theme first. The sections below set up what your panels show.',
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
        defaultValue: 'Build up to five layouts, then pick which one the watch shows by day, at night, and during Quiet Time. Night follows the schedule below.',
      },
      // the three invisible stores come first on purpose. Clay builds each item in order and only
      // attaches it after setting its value, so a store declared after the builder does not exist
      // yet when the builder initialises and goes looking for it.
      //
      // invisible store for the layout library: the five layouts and which one has each job. lives
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
      // the Quiet Time layout, written the same way and read the same way. its presence is also
      // what tells the builder to offer a Quiet Time row at all
      {
        type: 'hiddenStore',
        messageKey: 'LAYOUT_QUIET',
        storeClass: 'gl-quiet',
        defaultValue: '0',
      },
      {
        type: 'layoutBuilder',
        messageKey: 'LAYOUT',
        defaultValue: '2,0,0,2,2;12,0,2,2,1;13,1,2,2,1;1,2,0,4,1;3,3,0,2,2;6,3,2,2,2',
        moduleOptions: MODULE_OPTIONS,
        moduleThumbnails: moduleThumbnails,
      },
      ...nightScheduleItems(),
    ],
  },
  // --- Appearance ---
  // it sits below Layout, which is safe under Clay's build order since the theme editor reads none
  // of the layout stores
  {
    type: 'section',
    items: [
      heading('Appearance'),
      {
        type: 'select',
        messageKey: 'APPEARANCE_THEME',
        label: 'Theme',
        description: 'Vibrant colours each panel by what it shows. Custom uses the colours you set in the editor below.',
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
      headerFontItem(),
      panelStyleItem(),
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
      ], 0, 'The dial on the Analog Clock panel.'),
    ],
  },
  // --- Clock ---
  {
    type: 'section',
    items: [
      heading('Clock'),
      select('CLOCK_TIME_FORMAT', 'Time Format', [
        { label: 'System Default', value: 0 },
        { label: '12-hour (08:30)', value: 1 },
        { label: '12-hour (8:30)', value: 4 },
        { label: '24-hour (20:30)', value: 2 },
      ], 0),
      select('CLOCK_DATE_FORMAT', 'Date Format', configBuilder.defaultDateOptions, '%a %b %d'),
      select('CLOCK_WEEK_START', 'Week Starts On', [
        { label: 'Sunday', value: 0 },
        { label: 'Monday', value: 1 },
      ], 0, 'The first day of the week on the Month Grid, Weekday Dots, and Week Progress panels.'),
      timeZoneItem(),
      hourlyVibeItem(),
    ],
  },
  weatherSection(),
  // straight under Weather, so the plugin choice folds this card away right below the select that hides it
  locationSection(),
  // --- Calendar ---
  {
    type: 'section',
    items: [
      heading('Calendar'),
      // the lead line holds for either source, so the Calendar Source select the plugin define adds
      // goes straight under it. the iCal line, the link, and the refresh interval only mean
      // something for an iCal link, so they hide while the events come from a Pebble app plugin
      {
        type: 'text',
        defaultValue: 'Add a Next Event, Agenda, or Timeline panel to your layout to see your events.',
      },
      {
        type: 'text',
        defaultValue: 'Paste your private iCal link below.',
        pluginSourceHides: 'CALENDAR_SOURCE',
      },
      {
        type: 'input',
        messageKey: 'CALENDAR_ICS_URL',
        label: 'iCal Link',
        pluginSourceHides: 'CALENDAR_SOURCE',
        description: "Your calendar's private iCal (.ics) address. Google Calendar has it under Settings and sharing, Integrate calendar, Secret address in iCal format. In iCloud, share the calendar as Public, and in Outlook, publish it. Anyone with the link can read your calendar.",
        attributes: {
          placeholder: 'https://.../calendar.ics',
          maxlength: 512,
        },
      },
      {
        ...select('CALENDAR_POLL', 'Refresh Interval', [
          { label: '5 min', value: 0 },
          { label: '10 min', value: 1 },
          { label: '15 min', value: 2 },
          { label: '20 min', value: 3 },
          { label: '30 min', value: 4 },
          { label: '1 hour', value: 5 },
        ], 4, 'How often the phone downloads your calendar. More often uses more battery.'),
        pluginSourceHides: 'CALENDAR_SOURCE',
      },
      select('CALENDAR_VIBE_MODE', 'Event Reminders', CALENDAR_VIBE_MODE_OPTIONS, 0, 'The watch buzzes before a timed event while a calendar panel is on your layout, and stays quiet during Quiet Time.'),
      // each pattern shows only under the reminder modes that fire it
      { ...select('CALENDAR_VIBE_15MIN', '15-Minute Reminder', VIBE_TYPE_OPTIONS, 1), visibleWhen: { key: 'CALENDAR_VIBE_MODE', in: [1] } },
      { ...select('CALENDAR_VIBE_5MIN', '5-Minute Reminder', VIBE_TYPE_OPTIONS, 3), visibleWhen: { key: 'CALENDAR_VIBE_MODE', in: [1, 2] } },
      { ...select('CALENDAR_VIBE_START', 'Start Reminder', VIBE_TYPE_OPTIONS, 2), visibleWhen: { key: 'CALENDAR_VIBE_MODE', in: [1, 2, 3] } },
    ],
  },
  // --- Stocks ---
  {
    type: 'section',
    items: [
      heading('Stocks'),
      {
        type: 'text',
        defaultValue: 'Add a Stock or Watchlist panel to your layout, then enter your symbols below.',
      },
      select('STOCK_PROVIDER', 'Data Source', [
        { label: 'Finnhub (Real-Time, Free Key)', value: 'finnhub' },
        { label: 'Yahoo (Real-Time, No Key)', value: 'yahoo' },
        { label: 'Twelve Data (Global, Free Key)', value: 'twelvedata' },
        { label: 'Alpha Vantage (End-of-Day, Free Key)', value: 'alphavantage' },
      ], 'finnhub', "Finnhub is real-time for US stocks. Yahoo needs no key, but it is an unofficial feed that can break without warning. Twelve Data adds markets outside the US. Alpha Vantage only fetches the day's close."),
      {
        type: 'input',
        messageKey: 'STOCK_API_KEY',
        label: 'API Key',
        description: 'Paste the free key from the source you picked.',
        attributes: {
          placeholder: 'Paste your private API key here...',
          maxlength: 64,
        },
        visibleWhen: { key: 'STOCK_PROVIDER', in: ['finnhub', 'twelvedata', 'alphavantage'] },
      },
      {
        type: 'input',
        messageKey: 'STOCK_SYMBOLS',
        label: 'Symbols',
        description: 'One symbol for the Stock panel, or up to four separated by commas for the Watchlist.',
        attributes: {
          placeholder: 'AAPL, MSFT, TSLA, NVDA',
          maxlength: 64,
        },
      },
      select('STOCK_POLL', 'Refresh Interval', [
        { label: '1 min', value: 0 },
        { label: '5 min', value: 1 },
        { label: '10 min', value: 2 },
        { label: '15 min', value: 3 },
        { label: '20 min', value: 4 },
        { label: '30 min', value: 5 },
      ], 5, 'How often the phone fetches new quotes. More often uses more battery. Twelve Data waits at least 15 minutes and slows down after hours, and Alpha Vantage fetches the close once a day.'),
    ],
  },
  healthSection({ hrv: true }),
  // --- Bluetooth ---
  {
    type: 'section',
    items: [
      heading('Bluetooth'),
      select('CONNECTION_VIBE_CONNECT', 'Vibrate on Connect', VIBE_OPTIONS, 0),
      select('CONNECTION_VIBE_DISCONNECT', 'Vibrate on Disconnect', VIBE_OPTIONS, 0),
    ],
  },

  {
    type: 'submit',
    defaultValue: 'Save & Apply to Watch',
  },
];

export default config;
