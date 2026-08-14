/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see lib/ts/pkjs/config-builder.ts), tuned with the
 * LCARS theme list and the two ops slots this face lets you fill.
 */
import buildConfig from '../../../../lib/ts/pkjs/config-builder';
import moduleThumbnails from './clay/module-thumbnails.g';
import moduleMeta from './clay/module-meta';

// what either right-hand slot can show. the value is the C OpsId, so these numbers are the
// contract with src/c/ops/ops.h and are only ever appended to, never reordered
const OPS_OPTIONS = [
  { label: 'Heart Rate (VITALS)', value: 0 },
  { label: 'Steps / Distance (TRAVERSAL)', value: 1 },
  { label: 'Battery (POWER)', value: 2 },
  { label: 'Calories (METABOLIC)', value: 3 },
  { label: 'Sleep (REGEN)', value: 4 },
  { label: 'Active Minutes (EXERTION)', value: 5 },
  { label: 'Moon Phase % (LUNAR)', value: 6 },
  { label: 'Moon Phase Name (LUNAR)', value: 7 },
  { label: 'Next Full / New Moon', value: 8 },
  { label: 'Sunrise (DAWN)', value: 9 },
  { label: 'Sunset (DUSK)', value: 10 },
  { label: 'Length of Day (DAYLIGHT)', value: 11 },
  { label: 'Countdown to Sunrise / Sunset', value: 12 },
  { label: 'Humidity (ATMOS)', value: 13 },
  { label: 'Wind (AIRFLOW)', value: 14 },
  { label: 'UV Index', value: 15 },
  { label: 'High / Low Temperature (RANGE)', value: 16 },
  { label: 'Julian Date', value: 17 },
  { label: 'Day of Year (SOL)', value: 18 },
  { label: 'Week Number (CYCLE)', value: 19 },
  { label: 'Epoch Clock', value: 24 },
  { label: 'Swatch Beats', value: 25 },
  { label: 'Alternate Time Zone (ZONE 1)', value: 26 },
  { label: 'Temperature (THERMAL)', value: 21 },
  { label: 'Conditions (SKY)', value: 22 },
  { label: 'Empty', value: 20 },
];

// the weather block fills both rows of its column so it is only offered where a
// column starts. picking it anywhere else would leave half a block and the watch
// reads it as Empty there
const SENSORS_BLOCK = { label: 'Sensors Block (weather + temperature)', value: 23 };

// what the builder drags. every readout plus the block with the emoji and tint
// from module-meta riding along
// the builder cannot import that file itself. Clay turns its initialize into text
// and runs it in a webview with no module scope. so the merge happens here and
// rides on the page item
const BUILDER_OPTIONS = [SENSORS_BLOCK, ...OPS_OPTIONS].map(function (option) {
  const merged: Record<string, unknown> = {};
  Object.assign(merged, option, moduleMeta[option.label]);
  return merged;
});

export default buildConfig({
  theme: {
    label: 'Frame Theme',
    description: 'LCARS color scheme for the watch frame.',
    options: [
      { label: 'Classic', value: 0 },
      { label: 'Nemesis Blue', value: 3 },
      { label: 'Classic Mono', value: 4 },
      { label: 'Voyager', value: 7 },
      { label: 'Voyager Mono', value: 8 },
      { label: 'Lower Decks', value: 1 },
      { label: 'Lower Decks Mono', value: 5 },
      { label: 'Lower Decks PADD', value: 2 },
      { label: 'Lower Decks PADD Mono', value: 6 },
    ],
  },
  layoutSections: [
    {
      'type': 'section',
      'items': [
        {
          'type': 'heading',
          'defaultValue': 'Ops Panels',
        },
        {
          'type': 'text',
          'defaultValue': 'The four panels across the bottom of the face. The sun and weather readouts need a weather source set up below.',
        },
        // the three panels the builder does not own outright. a Clay component
        // carries one message key so the other three ride hidden stores it writes
        // to by class. the watch reads all four the same way as plain enum settings
        {
          'type': 'hiddenStore',
          'messageKey': 'APPEARANCE_SLOT_LEFT_BOTTOM',
          'storeClass': 'sb-lb',
          'defaultValue': '20',
        },
        {
          'type': 'hiddenStore',
          'messageKey': 'APPEARANCE_SLOT_RIGHT_TOP',
          'storeClass': 'sb-rt',
          'defaultValue': '0',
        },
        {
          'type': 'hiddenStore',
          'messageKey': 'APPEARANCE_SLOT_RIGHT_BOTTOM',
          'storeClass': 'sb-rb',
          'defaultValue': '1',
        },
        {
          'type': 'slotBuilder',
          'messageKey': 'APPEARANCE_SLOT_LEFT_TOP',
          'defaultValue': '23',
          'moduleOptions': BUILDER_OPTIONS,
          'moduleThumbnails': moduleThumbnails,
        },
      ],
    },
  ],
  location: { gpsDefault: false, timeZone: true },
  weather: {},
  temperature: {},
  quietTime: {},
  hourlyVibe: {},
  steps: {
    label: 'Steps Readout',
    description: 'Whether a panel set to Steps counts steps or shows the distance walked.',
  },
});
