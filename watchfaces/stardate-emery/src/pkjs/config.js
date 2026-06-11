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
        "description": "LCARS color scheme for the watch frame.",
        "defaultValue": 0,
        "options": [
          { "label": "Classic", "value": 0 },
          { "label": "Lower Decks", "value": 1 },
          { "label": "Lower Decks PADD", "value": 2 },
          { "label": "Nemesis Blue", "value": 3 }
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
        "defaultValue": false
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
          { "label": "2026.0612  (yyyy.mmdd)", "value": "%Y.%m%d" },
          { "label": "2026-06-12  (yyyy-mm-dd)", "value": "%Y-%m-%d" },
          { "label": "FRI JUN 12", "value": "%a %b %d" },
          { "label": "JUN 12", "value": "%b %d" },
          { "label": "06/12/2026  (mm/dd/yyyy)", "value": "%m/%d/%Y" },
          { "label": "12.06.2026  (dd.mm.yyyy)", "value": "%d.%m.%Y" }
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
        "messageKey": "TRAVERSAL_MODE",
        "label": "Traversal Readout",
        "description": "What the TRAVERSAL slot shows: step count, or distance walked.",
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