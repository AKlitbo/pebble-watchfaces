/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see lib/ts/pkjs/config-builder.ts), tuned with the scene
 * palette list and the wording this face uses for its stats row. Weather matters more here
 * than on most faces, since the condition is what the picture is drawn from.
 */
import buildConfig from '../../../../../lib/ts/pkjs/config-builder';

export default buildConfig({
  heading: 'Ridgeline',
  intro: 'Pick a palette for the mountains and the sky, then set up your weather so the scene knows what it is drawing.',
  theme: {
    label: 'Scene Palette',
    description: 'Colours for the sky, the ridges, and the linework. Each one has a day and a night version, and the face swaps between them at sunrise and sunset.',
    options: [
      { label: 'Sketchbook', value: 0 },
      { label: 'Daybreak', value: 1 },
      { label: 'Alpenglow', value: 2 },
      { label: 'Blueprint', value: 3 },
      { label: 'Forest', value: 4 },
      { label: 'Cyberpunk', value: 6 },
      { label: 'Neo Tokyo', value: 7 },
      { label: 'Mono (Black & White)', value: 5 },
    ],
  },
  appearanceItems: [
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_LAYOUT',
      'label': 'Layout',
      'description': 'Controls how much space the clock takes. Larger layouts move the date into the top bar and remove the readouts to give the time more room.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Balanced', 'value': 0 },
        { 'label': 'Time Focused', 'value': 1 },
        { 'label': 'Clock Only', 'value': 2 },
      ],
    },
  ],
  clockItems: [
    {
      'type': 'toggle',
      'messageKey': 'CLOCK_MERIDIEM',
      'label': 'Show AM/PM',
      'description': 'Show the AM/PM marker above the colon. Only applies to a 12-hour clock, since the other formats have no marker to show.',
      'defaultValue': true,
    },
  ],
  quietTime: {
    description: 'Show a muted-speaker glyph next to bluetooth while Quiet Time is on.',
  },
  bluetooth: {
    description: 'Show a bluetooth glyph in the sky - lit when the phone is connected, slashed when it drops.',
  },
  location: { gpsDefault: true },
  weather: {},
  temperature: {},
  hourlyVibe: {},
  steps: {
    label: 'Tracks Readout',
    description: 'What the tracks in the stats row show: step count, or distance walked.',
  },
});
