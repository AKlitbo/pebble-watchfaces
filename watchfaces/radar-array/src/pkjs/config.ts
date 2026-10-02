/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see paf/ts/pkjs/config-builder.ts), tuned with the
 * radar theme list, GPS on by default, and the RANGE wording this face uses for its
 * stats slot.
 */
import buildConfig from '../../paf/ts/pkjs/config-builder';

export default buildConfig({
  // the framework's default intro leaves weather out, and this face has it
  intro: 'Personalize your layout, dial in your weather preferences, and make this watchface your own.',
  theme: {
    label: 'Frame Theme',
    description: 'Colour scheme for the watch frame.',
    options: [
      { label: 'Default', value: 0 },
      { label: 'Rescue', value: 1 },
      { label: 'Neon', value: 2 },
      { label: 'Stealth', value: 3 },
      { label: 'Phosphor', value: 4 },
      { label: 'Crimson', value: 5 },
      { label: 'Mono', value: 6 },
    ],
  },
  location: { gpsDefault: true },
  weather: {},
  temperature: {},
  hourlyVibe: {},
  steps: {
    label: 'Range Readout',
    description: 'What the RANGE slot shows: step count, or distance walked.',
  },
});
