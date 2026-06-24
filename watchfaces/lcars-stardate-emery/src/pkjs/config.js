const buildConfig = require('../../../../lib/js/pkjs/config-builder');

module.exports = buildConfig({
  theme: {
    label: "Frame Theme",
    description: "LCARS color scheme for the watch frame.",
    options: [
      { "label": "Classic", "value": 0 },
      { "label": "Lower Decks", "value": 1 },
      { "label": "Lower Decks PADD", "value": 2 },
      { "label": "Nemesis Blue", "value": 3 }
    ]
  },
  location: { gpsDefault: false },
  weather: {},
  temperature: {},
  steps: {
    label: "Traversal Readout",
    description: "What the TRAVERSAL slot shows: step count, or distance walked."
  }
});
