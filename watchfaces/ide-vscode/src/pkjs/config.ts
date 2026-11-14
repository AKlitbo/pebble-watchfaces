/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see paf/ts/pkjs/config-builder.ts), tuned with the
 * editor theme list, a compact date default that fits the terminal panel, and the
 * battery section this face's tab-strip readout needs.
 */
import buildConfig from '../../paf/ts/pkjs/config-builder';

export default buildConfig({
  theme: {
    label: 'Editor Theme',
    description: 'Colour scheme for the editor frame and readouts.',
    options: [
      { label: 'Dark', value: 0 },
      { label: 'Light', value: 1 },
      { label: 'Terminal', value: 2 },
      { label: 'Cyberpunk', value: 3 },
      { label: "Synthwave '84", value: 4 },
      { label: 'Mono (Black & White)', value: 5 },
    ],
  },
  bluetooth: {
    description: 'Show a bluetooth glyph beside the battery. It is lit while the phone is connected and slashed when the link drops.',
  },
  location: { gpsDefault: false },
  weather: {},
  temperature: {},
  hourlyVibe: {},
  date: {
    default: '%b %d',
  },
  steps: {
    label: 'Stats Readout',
    description: 'What the terminal steps line shows: step count, or distance walked.',
  },
  battery: {
    description: 'What the tab-strip battery readout shows: the icon, the percentage, or both.',
  },
});
