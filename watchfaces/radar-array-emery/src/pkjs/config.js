const buildConfig = require('../../../../lib/js/pkjs/config-builder');

module.exports = buildConfig({
  theme: {
    label: "Frame Theme",
    description: "Colour scheme for the watch frame.",
    options: [
      { "label": "Default", "value": 0 },
      { "label": "Rescue", "value": 1 },
      { "label": "Neon", "value": 2 },
      { "label": "Stealth", "value": 3 },
      { "label": "Phosphor", "value": 4 },
      { "label": "Crimson", "value": 5 },
      { "label": "Mono", "value": 6 }
    ]
  },
  location: { gpsDefault: true },
  weather: {},
  temperature: {},
  steps: {
    label: "Range Readout",
    description: "What the RANGE slot shows: step count, or distance walked."
  }
});
