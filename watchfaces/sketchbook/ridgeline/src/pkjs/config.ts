/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see paf/ts/pkjs/config-builder.ts), tuned with the scene
 * palette list and the wording this face uses for its stats row. Weather matters more here
 * than on most faces, since the condition is what the picture is drawn from.
 */
import buildConfig from '../../../paf/ts/pkjs/config-builder';
import themePreviews from './clay/theme-previews.g';

// the eight palettes, shared by the Scene Palette picker and the night and Quiet Time pickers
const paletteOptions = [
  { label: 'Sketchbook', value: 0 },
  { label: 'Daybreak', value: 1 },
  { label: 'Alpenglow', value: 2 },
  { label: 'Blueprint', value: 3 },
  { label: 'Forest', value: 4 },
  { label: 'Cyberpunk', value: 6 },
  { label: 'Neo Tokyo', value: 7 },
  { label: 'Mono (Black & White)', value: 5 },
];

export default buildConfig({
  heading: 'Ridgeline',
  intro: 'Pick a palette for the mountains and the sky, then set up your weather so the scene knows what it is drawing.',
  theme: {
    label: 'Scene Palette',
    description: 'Colours for the sky, the ridges, and the linework. Each one has a day and a night version, and the face swaps between them at sunrise and sunset.',
    options: paletteOptions,
    previews: themePreviews,
  },
  appearanceItems: [
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_PALETTE_MODE',
      'label': 'Palette Mode',
      'description': 'How the palette above is used. Fixed keeps it. Shuffle rotates through them all. Day / Night / Quiet keeps it by day and swaps to another after dark or while Quiet Time is on.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Fixed', 'value': 0 },
        { 'label': 'Shuffle', 'value': 1 },
        { 'label': 'Day / Night / Quiet', 'value': 2 },
      ],
    },
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_SHUFFLE_INTERVAL',
      'visibleWhen': { 'key': 'APPEARANCE_PALETTE_MODE', 'in': [1] },
      'label': 'Shuffle Every',
      'description': 'How long each palette stays up in Shuffle mode. It has no effect in the other modes.',
      'defaultValue': 2,
      'options': [
        { 'label': '15 Minutes', 'value': 0 },
        { 'label': '30 Minutes', 'value': 1 },
        { 'label': 'Hour', 'value': 2 },
        { 'label': '3 Hours', 'value': 3 },
        { 'label': '6 Hours', 'value': 4 },
        { 'label': 'Day', 'value': 5 },
      ],
    },
    {
      'type': 'toggle',
      'messageKey': 'APPEARANCE_NIGHT_PALETTE_ON',
      'visibleWhen': { 'key': 'APPEARANCE_PALETTE_MODE', 'in': [2] },
      'label': 'Use Night Palette',
      'description': 'In Day / Night / Quiet mode, paint a different palette after dark. Off keeps the daytime one all night.',
      'defaultValue': false,
    },
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_NIGHT_PALETTE',
      'visibleWhen': [{ 'key': 'APPEARANCE_PALETTE_MODE', 'in': [2] }, { 'key': 'APPEARANCE_NIGHT_PALETTE_ON', 'in': [true] }],
      'label': 'Night Palette',
      'defaultValue': 0,
      'options': paletteOptions,
    },
    {
      'type': 'toggle',
      'messageKey': 'APPEARANCE_QUIET_PALETTE_ON',
      'visibleWhen': { 'key': 'APPEARANCE_PALETTE_MODE', 'in': [2] },
      'label': 'Use Quiet Time Palette',
      'description': 'In Day / Night / Quiet mode, paint a different palette while Quiet Time is on. Quiet Time wins over night.',
      'defaultValue': false,
    },
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_QUIET_PALETTE',
      'visibleWhen': [{ 'key': 'APPEARANCE_PALETTE_MODE', 'in': [2] }, { 'key': 'APPEARANCE_QUIET_PALETTE_ON', 'in': [true] }],
      'label': 'Quiet Time Palette',
      'defaultValue': 0,
      'options': paletteOptions,
    },
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_LAYOUT',
      // a round build ships the balanced layout only, so the picker has nothing to offer there.
      // the firmware forces it too, so this is the tidy half of the pair rather than the load
      // bearing one
      'capabilities': ['NOT_PLATFORM_GABBRO'],
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
  // the date line runs through readout_date, so it can end in a .beats reading
  date: { beats: true },
  location: { gpsDefault: true },
  weather: {},
  temperature: {},
  hourlyVibe: {},
  steps: {
    label: 'Tracks Readout',
    description: 'What the tracks in the stats row show: step count, or distance walked.',
    // a round build has no stats row, so there is nothing for this to pick
    capabilities: ['NOT_PLATFORM_GABBRO'],
  },
});
