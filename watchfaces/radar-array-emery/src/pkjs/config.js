module.exports = [
  {
    "type": "heading",
    "defaultValue": "Watchface Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Personalize your layout, dial in your weather preferences, and make this watchface your own."
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
        "label": "Frame Theme",
        "description": "Colour scheme for the watch frame.",
        "defaultValue": 0,
        "options": [
          { "label": "Default", "value": 0 },
          { "label": "Rescue", "value": 1 },
          { "label": "Neon", "value": 2 },
          { "label": "Stealth", "value": 3 },
          { "label": "Phosphor", "value": 4 },
          { "label": "Crimson", "value": 5 }
        ]
      }
    ]
  },
  {
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
        "description": "Display a bluetooth glyph showing whether the watch is connected to your phone.",
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
  },
  {
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
        "defaultValue": true
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
  },
  {
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
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "General Preferences"
      },
      {
        "type": "select",
        "messageKey": "DATE_FORMAT",
        "label": "Date Format",
        "description": "How the date line is written.",
        "defaultValue": "%Y.%m%d",
        "options": [
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
        ]
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
      },
      {
        "type": "toggle",
        "messageKey": "TEMPERATURE_UNIT",
        "label": "Display in Fahrenheit (°F)",
        "description": "Leave unchecked to use Celsius (°C).",
        "defaultValue": false
      },
      {
        "type": "select",
        "messageKey": "STEPS_MODE",
        "label": "Range Readout",
        "description": "What the RANGE slot shows: step count, or distance walked.",
        "defaultValue": 0,
        "options": [
          { "label": "Steps", "value": 0 },
          { "label": "Distance (Miles)", "value": 1 },
          { "label": "Distance (Kilometers)", "value": 2 }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save & Apply to Watch"
  }
];