/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see lib/ts/pkjs/config-builder.ts), tuned with the
 * LCARS theme list and the TRAVERSAL wording this face uses for its stats slot.
 */
import buildConfig from '../../../../lib/ts/pkjs/config-builder';

export default buildConfig({
  theme: {
    label: 'Frame Theme',
    description: 'LCARS color scheme for the watch frame.',
    options: [
      { label: 'Classic', value: 0 },
      { label: 'Nemesis Blue', value: 3 },
      { label: 'Classic Mono', value: 4 },
      { label: 'Lower Decks', value: 1 },
      { label: 'Lower Decks Mono', value: 5 },
      { label: 'Lower Decks PADD', value: 2 },
      { label: 'Lower Decks PADD Mono', value: 6 },
    ],
  },
  location: { gpsDefault: false },
  weather: {},
  temperature: {},
  steps: {
    label: 'Traversal Readout',
    description: 'What the TRAVERSAL slot shows: step count, or distance walked.',
  },
});
