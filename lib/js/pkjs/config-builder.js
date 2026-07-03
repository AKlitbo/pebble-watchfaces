'use strict';

const defaultDateOptions = [
  { "label": "2026.0618 (yyyy.mmdd)", "value": "%Y.%m%d" },
  { "label": "2026.06.18 (yyyy.mm.dd)", "value": "%Y.%m.%d" },
  { "label": "06.18.2026 (mm.dd.yyyy)", "value": "%m.%d.%Y" },
  { "label": "18.06.2026 (dd.mm.yyyy)", "value": "%d.%m.%Y" },
  { "label": "2026-06-18 (yyyy-mm-dd)", "value": "%Y-%m-%d" },
  { "label": "06-18-2026 (mm-dd-yyyy)", "value": "%m-%d-%Y" },
  { "label": "18-06-2026 (dd-mm-yyyy)", "value": "%d-%m-%Y" },
  { "label": "2026/06/18 (yyyy/mm/dd)", "value": "%Y/%m/%d" },
  { "label": "06/18/2026 (mm/dd/yyyy)", "value": "%m/%d/%Y" },
  { "label": "18/06/2026 (dd/mm/yyyy)", "value": "%d/%m/%Y" },
  { "label": "JUN 18 (mmm dd)", "value": "%b %d" },
  { "label": "JUN 18 2026 (mmm dd yyyy)", "value": "%b %d %Y" },
  { "label": "18 JUN 2026 (dd mmm yyyy)", "value": "%d %b %Y" },
  { "label": "THU JUN 18 (ddd mmm dd)", "value": "%a %b %d" },
  { "label": "THU 18 JUN (ddd dd mmm)", "value": "%a %d %b" },
  { "label": "2026-169 (yyyy.dayofyear)", "value": "%Y.%j" },
  { "label": "2026-169 (yyyy-dayofyear)", "value": "%Y-%j" }
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
 *   weather     {}                               marker, no fields to override yet
 *   temperature {}                               marker, adds the °F toggle to preferences
 */
module.exports = function buildConfig(options) {
  const theme = options.theme || {};
  const bluetooth = options.bluetooth || {};
  const date = options.date || {};
  const steps = options.steps || {};

  const config = [
    {
      "type": "heading",
      "defaultValue": options.heading || "Watchface Configuration"
    },
    {
      "type": "text",
      "defaultValue": options.intro || "Personalize your layout, dial in your weather preferences, and make this watchface your own."
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Appearance"
        },
        {
          "type": "select",
          "messageKey": "THEME",
          "label": theme.label || "Frame Theme",
          "description": theme.description || "Colour scheme for the watch frame.",
          "defaultValue": 0,
          "options": theme.options
        }
      ]
    }
  ];

  // face-specific sections (e.g. Modular's layout + goals) sit right after Appearance
  if (options.layoutSections) {
    options.layoutSections.forEach((section) => config.push(section));
  }

  config.push({
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bluetooth"
      },
      {
        "type": "toggle",
        "messageKey": "BLUETOOTH_ICON",
        "label": "Show Connection Icon",
        "description": bluetooth.description || "Display a bluetooth glyph showing whether the watch is connected to your phone.",
        "defaultValue": true
      },
      {
        "type": "select",
        "messageKey": "BLUETOOTH_VIBE_CONNECT",
        "label": "Vibrate on Connect",
        "description": "Buzz the watch when the phone reconnects.",
        "defaultValue": 0,
        "options": [
          { "label": "None", "value": 0 },
          { "label": "Short", "value": 1 },
          { "label": "Long", "value": 2 },
          { "label": "Double", "value": 3 }
        ]
      },
      {
        "type": "select",
        "messageKey": "BLUETOOTH_VIBE_DISCONNECT",
        "label": "Vibrate on Disconnect",
        "description": "Buzz the watch when the phone disconnects.",
        "defaultValue": 0,
        "options": [
          { "label": "None", "value": 0 },
          { "label": "Short", "value": 1 },
          { "label": "Long", "value": 2 },
          { "label": "Double", "value": 3 }
        ]
      }
    ]
  });

  if (options.location) {
    config.push({
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Location Settings"
        },
        {
          "type": "toggle",
          "messageKey": "USE_GPS",
          "label": "Enable Phone GPS",
          "description": "Automatically fetch weather for your current location.",
          "defaultValue": options.location.gpsDefault !== undefined ? options.location.gpsDefault : false
        },
        {
          "type": "toggle",
          "messageKey": "GPS_FALLBACK",
          "label": "Fallback to Manual Location",
          "description": "If GPS is disabled or unavailable, use the city typed below.",
          "defaultValue": true
        },
        {
          "type": "locationsearch",
          "messageKey": "LOCATION_NAME",
          "label": "Manual Location",
          "attributes": {
            "placeholder": "Search a city, e.g. Phoenix"
          }
        }
      ]
    });
  }

  if (options.weather) {
    config.push({
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Weather Provider"
        },
        {
          "type": "text",
          "defaultValue": "Choose where your watch pulls its weather data. Open-Meteo works right out of the box with no setup required."
        },
        {
          "type": "select",
          "messageKey": "WEATHER_PROVIDER",
          "label": "Data Source",
          "defaultValue": "openmeteo",
          "options": [
            { "label": "Open-Meteo (Free, No Key Required)", "value": "openmeteo" },
            { "label": "OpenWeatherMap", "value": "owm" },
            { "label": "WeatherAPI.com", "value": "weatherapi" }
          ]
        },
        {
          "type": "input",
          "messageKey": "API_KEY",
          "label": "API Key",
          "description": "Only required if you selected OpenWeatherMap or WeatherAPI above.",
          "attributes": {
            "placeholder": "Paste your private API key here...",
            "limit": 64
          }
        }
      ]
    });
  }

  const prefsItems = [
    {
      "type": "heading",
      "defaultValue": "General Preferences"
    },
    {
      "type": "select",
      "messageKey": "DATE_FORMAT",
      "label": date.label || "Date Format",
      "description": date.description || "How the date line is written.",
      "defaultValue": date.default || "%Y.%m%d",
      "options": date.options || defaultDateOptions
    },
    {
      "type": "select",
      "messageKey": "TIME_FORMAT",
      "label": "Time Format",
      "description": "How the main time is shown. .beats is Swatch Internet Time (Biel Mean Time).",
      "defaultValue": 0,
      "options": [
        { "label": "System Default", "value": 0 },
        { "label": "12-hour", "value": 1 },
        { "label": "24-hour", "value": 2 },
        { "label": ".beats (Swatch Internet Time)", "value": 3 }
      ]
    }
  ];

  if (options.temperature) {
    prefsItems.push({
      "type": "toggle",
      "messageKey": "TEMPERATURE_UNIT",
      "label": "Display in Fahrenheit (°F)",
      "description": "Leave unchecked to use Celsius (°C).",
      "defaultValue": false
    });
  }

  prefsItems.push({
    "type": "select",
    "messageKey": "STEPS_MODE",
    "label": steps.label || "Stats Readout",
    "description": steps.description || "What the stats slot shows: step count, or distance walked.",
    "defaultValue": 0,
    "options": [
      { "label": "Steps", "value": 0 },
      { "label": "Distance (Miles)", "value": 1 },
      { "label": "Distance (Kilometers)", "value": 2 }
    ]
  });

  if (options.battery) {
    prefsItems.push({
      "type": "select",
      "messageKey": "BATTERY_DISPLAY",
      "label": options.battery.label || "Battery",
      "description": options.battery.description || "What the battery readout shows.",
      "defaultValue": 0,
      "options": [
        { "label": "Icon + Percent", "value": 0 },
        { "label": "Icon Only", "value": 1 },
        { "label": "Percent Only", "value": 2 }
      ]
    });
  }

  config.push({
    "type": "section",
    "items": prefsItems
  });

  config.push({
    "type": "submit",
    "defaultValue": "Save & Apply to Watch"
  });

  return config;
};

module.exports.defaultDateOptions = defaultDateOptions;
