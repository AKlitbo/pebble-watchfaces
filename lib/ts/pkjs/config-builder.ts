/**
 * A generic Clay settings-page builder for a simple face.
 *
 * buildConfig stamps out the common sections (theme, date, weather, location, and
 * so on) from a small per-section description, so a plain face gets a settings page
 * without hand-assembling one. A face whose layout or theme picker outgrows the
 * template can hand-build its own page instead and reuse only defaultDateOptions
 * from here. buildConfig stays as the starting point a copied face would reach for
 * first.
 */

import type { ClayConfigItem } from '../clay/types';

/** The per-section description a face hands buildConfig. Each section is optional.
 * Present means "include it", and its own fields tune it. */
interface ConfigBuilderOptions {
  heading?: string;
  intro?: string;
  theme?: { label?: string; description?: string; options?: Array<{ label: string; value: string | number }> };
  bluetooth?: { description?: string };
  quietTime?: { description?: string };
  hourlyVibe?: { label?: string; description?: string };
  date?: { label?: string; description?: string; default?: string; options?: Array<{ label: string; value: string | number }> };
  steps?: { label?: string; description?: string };
  location?: { gpsDefault?: boolean };
  weather?: unknown;
  temperature?: unknown;
  battery?: { label?: string; description?: string };
  layoutSections?: ClayConfigItem[];
}

/** The stock date-format choices, shared by faces that don't override them. */
const defaultDateOptions = [
  { 'label': '2026.0618 (yyyy.mmdd)', 'value': '%Y.%m%d' },
  { 'label': '2026.06.18 (yyyy.mm.dd)', 'value': '%Y.%m.%d' },
  { 'label': '06.18.2026 (mm.dd.yyyy)', 'value': '%m.%d.%Y' },
  { 'label': '18.06.2026 (dd.mm.yyyy)', 'value': '%d.%m.%Y' },
  { 'label': '2026-06-18 (yyyy-mm-dd)', 'value': '%Y-%m-%d' },
  { 'label': '06-18-2026 (mm-dd-yyyy)', 'value': '%m-%d-%Y' },
  { 'label': '18-06-2026 (dd-mm-yyyy)', 'value': '%d-%m-%Y' },
  { 'label': '2026/06/18 (yyyy/mm/dd)', 'value': '%Y/%m/%d' },
  { 'label': '06/18/2026 (mm/dd/yyyy)', 'value': '%m/%d/%Y' },
  { 'label': '18/06/2026 (dd/mm/yyyy)', 'value': '%d/%m/%Y' },
  { 'label': 'JUN 18 (mmm dd)', 'value': '%b %d' },
  { 'label': 'JUN 18 2026 (mmm dd yyyy)', 'value': '%b %d %Y' },
  { 'label': '18 JUN 2026 (dd mmm yyyy)', 'value': '%d %b %Y' },
  { 'label': 'THU JUN 18 (ddd mmm dd)', 'value': '%a %b %d' },
  { 'label': 'THU 18 JUN (ddd dd mmm)', 'value': '%a %d %b' },
  { 'label': '2026.169 (yyyy.dayofyear)', 'value': '%Y.%j' },
  { 'label': '2026-169 (yyyy-dayofyear)', 'value': '%Y-%j' },
];

/** The buzz patterns the connect and disconnect selects both offer. */
const vibeOptions = [
  { 'label': 'None', 'value': 0 },
  { 'label': 'Short', 'value': 1 },
  { 'label': 'Long', 'value': 2 },
  { 'label': 'Double', 'value': 3 },
];

/**
 * Builds a Clay config array from a per-section description.
 *
 * Top-level fields:
 *   heading, intro        page title and lead paragraph (both optional)
 *
 * Section objects (each section is customised through its own fields):
 *   theme    { label?, description?, options }   options is the theme list (required)
 *   bluetooth { description? }                   always shown
 *   date     { label?, description?, default?, options? }
 *   steps    { label?, description? }
 *
 * Optional sections (present = included, omitted = excluded):
 *   location    { gpsDefault? }
 *   weather     {}                               marker, adds the provider picker to the Weather section
 *   temperature {}                               marker, adds the unit dropdown to the Weather section
 */
function buildConfig(options: ConfigBuilderOptions): ClayConfigItem[] {
  const theme = options.theme || {};
  const bluetooth = options.bluetooth || {};
  const date = options.date || {};
  const steps = options.steps || {};

  const config: ClayConfigItem[] = [
    {
      'type': 'heading',
      'defaultValue': options.heading || 'Watchface Configuration',
    },
    {
      'type': 'text',
      'defaultValue': options.intro || 'Personalize your layout, dial in your weather preferences, and make this watchface your own.',
    },
    {
      'type': 'section',
      'items': [
        {
          'type': 'heading',
          'defaultValue': 'Appearance',
        },
        {
          'type': 'select',
          'messageKey': 'APPEARANCE_THEME',
          'label': theme.label || 'Frame Theme',
          'description': theme.description || 'Colour scheme for the watch frame.',
          'defaultValue': 0,
          'options': theme.options,
        },
        ...(options.quietTime ? [{
          'type': 'toggle',
          'messageKey': 'APPEARANCE_QUIET_TIME_ICON',
          'label': 'Show Quiet Time Icon',
          'description': options.quietTime.description || 'Show a muted-speaker glyph next to bluetooth while Quiet Time is on.',
          'defaultValue': false,
        }] : []),
      ],
    },
  ];

  // face-specific sections (e.g. a layout and goals) sit right after Appearance
  if (options.layoutSections) {
    options.layoutSections.forEach((section) => config.push(section));
  }

  config.push({
    'type': 'section',
    'items': [
      {
        'type': 'heading',
        'defaultValue': 'Bluetooth',
      },
      {
        'type': 'toggle',
        'messageKey': 'CONNECTION_BLUETOOTH_ICON',
        'label': 'Show Connection Icon',
        'description': bluetooth.description || 'Display a bluetooth glyph showing whether the watch is connected to your phone.',
        'defaultValue': true,
      },
      {
        'type': 'select',
        'messageKey': 'CONNECTION_VIBE_CONNECT',
        'label': 'Vibrate on Connect',
        'description': 'Buzz the watch when the phone reconnects.',
        'defaultValue': 0,
        'options': vibeOptions,
      },
      {
        'type': 'select',
        'messageKey': 'CONNECTION_VIBE_DISCONNECT',
        'label': 'Vibrate on Disconnect',
        'description': 'Buzz the watch when the phone disconnects.',
        'defaultValue': 0,
        'options': vibeOptions,
      },
    ],
  });

  const clockItems: ClayConfigItem[] = [
    {
      'type': 'heading',
      'defaultValue': 'Clock',
    },
    {
      'type': 'select',
      'messageKey': 'CLOCK_DATE_FORMAT',
      'label': date.label || 'Date Format',
      'description': date.description || 'How the date line is written.',
      'defaultValue': date.default || '%Y.%m%d',
      'options': date.options || defaultDateOptions,
    },
    {
      'type': 'select',
      'messageKey': 'CLOCK_TIME_FORMAT',
      'label': 'Time Format',
      'description': 'How the main time is shown. .beats is Swatch Internet Time (Biel Mean Time).',
      'defaultValue': 0,
      'options': [
        { 'label': 'System Default', 'value': 0 },
        { 'label': '12-hour', 'value': 1 },
        { 'label': '24-hour', 'value': 2 },
        { 'label': '.beats (Swatch Internet Time)', 'value': 3 },
      ],
    },
  ];

  if (options.hourlyVibe) {
    clockItems.push({
      'type': 'select',
      'messageKey': 'CLOCK_HOURLY_VIBE',
      'label': options.hourlyVibe.label || 'Hourly Vibration',
      'description': options.hourlyVibe.description || 'Buzz at the top of every hour. Silenced automatically during Quiet Time.',
      'defaultValue': 0,
      'options': vibeOptions,
    });
  }

  config.push({
    'type': 'section',
    'items': clockItems,
  });

  const healthItems: ClayConfigItem[] = [
    {
      'type': 'heading',
      'defaultValue': 'Health',
    },
    {
      'type': 'select',
      'messageKey': 'HEALTH_STEPS_MODE',
      'label': steps.label || 'Stats Readout',
      'description': steps.description || 'What the stats slot shows: step count, or distance walked.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Steps', 'value': 0 },
        { 'label': 'Distance (Miles)', 'value': 1 },
        { 'label': 'Distance (Kilometers)', 'value': 2 },
      ],
    },
  ];

  if (options.battery) {
    healthItems.push({
      'type': 'select',
      'messageKey': 'BATTERY_DISPLAY',
      'label': options.battery.label || 'Battery',
      'description': options.battery.description || 'What the battery readout shows.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Icon + Percent', 'value': 0 },
        { 'label': 'Icon Only', 'value': 1 },
        { 'label': 'Percent Only', 'value': 2 },
      ],
    });
  }

  config.push({
    'type': 'section',
    'items': healthItems,
  });

  if (options.location) {
    config.push({
      'type': 'section',
      'items': [
        {
          'type': 'heading',
          'defaultValue': 'Location Settings',
        },
        {
          'type': 'toggle',
          'messageKey': 'LOCATION_USE_GPS',
          'label': 'Enable Phone GPS',
          'description': 'Automatically fetch weather for your current location.',
          'defaultValue': options.location.gpsDefault !== undefined ? options.location.gpsDefault : false,
        },
        {
          'type': 'toggle',
          'messageKey': 'LOCATION_GPS_FALLBACK',
          'label': 'Fallback to Manual Location',
          'description': 'If GPS is disabled or unavailable, use the city typed below.',
          'defaultValue': true,
        },
        {
          'type': 'locationsearch',
          'messageKey': 'LOCATION_NAME',
          'label': 'Manual Location',
          'attributes': {
            'placeholder': 'Search a city, e.g. Phoenix',
          },
        },
      ],
    });
  }

  if (options.weather || options.temperature) {
    const weatherItems: ClayConfigItem[] = [
      {
        'type': 'heading',
        'defaultValue': 'Weather',
      },
    ];

    if (options.temperature) {
      weatherItems.push({
        'type': 'select',
        'messageKey': 'WEATHER_TEMPERATURE_UNIT',
        'label': 'Temperature Unit',
        'defaultValue': 0,
        'options': [
          { 'label': 'Celsius (°C)', 'value': 0 },
          { 'label': 'Fahrenheit (°F)', 'value': 1 },
        ],
      });
    }

    if (options.weather) {
      weatherItems.push(
        {
          'type': 'text',
          'defaultValue': 'Choose where your watch pulls its weather data. Open-Meteo works right out of the box with no setup required.',
        },
        {
          'type': 'select',
          'messageKey': 'WEATHER_PROVIDER',
          'label': 'Data Source',
          'defaultValue': 'openmeteo',
          'options': [
            { 'label': 'Open-Meteo (Free, No Key Required)', 'value': 'openmeteo' },
            { 'label': 'OpenWeatherMap', 'value': 'owm' },
            { 'label': 'WeatherAPI.com', 'value': 'weatherapi' },
          ],
        },
        {
          'type': 'input',
          'messageKey': 'WEATHER_API_KEY',
          'label': 'API Key',
          'description': 'Only required if you selected OpenWeatherMap or WeatherAPI above.',
          'attributes': {
            'placeholder': 'Paste your private API key here...',
            'limit': 64,
          },
        }
      );
    }

    config.push({
      'type': 'section',
      'items': weatherItems,
    });
  }

  config.push({
    'type': 'submit',
    'defaultValue': 'Save & Apply to Watch',
  });

  return config;
}

// attach the date options to the function too so it matches the module.exports shape
// so a consumer can read them off the default export as well as the named one
buildConfig.defaultDateOptions = defaultDateOptions;

export default buildConfig;
