/**
 * The Health section of the settings page.
 *
 * Both faces in the family ask for the same goals, the same units and the same celebration buzz,
 * and store them under the same keys, so the rows are built once here rather than written out
 * twice and drifting.
 *
 * The goal tables send an index rather than the number itself. The watch keeps the real figures,
 * so a picker only has to agree with it on the order.
 */

import type { ClayConfigItem, ClayOption } from '../../paf/ts/clay/types';
import { select, heading, subheading } from '../../paf/ts/pkjs/config-rows';

// the Goal Met Vibration picks. the value IS what the watch plays, so the tune data lives here in
// the config, not in the watch binary. a one-letter marker is a plain pulse (S/L/D), C means the
// Custom Vibration Pattern box below, and a comma list of milliseconds is a fanfare rhythm the watch
// buzzes straight through. None is the empty string
const GOAL_VIBE_OPTIONS: ClayOption[] = [
  { label: 'None', value: '' },
  { label: 'Short', value: 'S' },
  { label: 'Long', value: 'L' },
  { label: 'Double', value: 'D' },
  { label: 'Fanfare 7', value: '50,50,50,50,50,50,250,150,250,150,250,150,100,100,100,100,600' },
  { label: 'Fanfare 8', value: '50,60,50,60,50,60,300,180,300,180,300,180,110,110,110,110,700' },
  { label: 'Fanfare 10', value: '60,50,60,50,60,50,350,250,350,250,350,250,120,100,120,100,800' },
  { label: 'Fanfare 12', value: '70,60,70,60,70,60,400,200,400,200,400,200,150,100,150,100,900' },
  { label: 'Fanfare 13', value: '40,40,40,40,40,40,200,100,200,100,200,100,80,80,80,80,500' },
  { label: 'Custom', value: 'C' },
];

const GOAL_STEPS = [5000, 7500, 10000, 12500, 15000, 20000, 25000].map((n, i) => ({ label: n.toLocaleString(), value: i }));
const GOAL_CALORIES = [500, 1000, 1500, 2000, 2500, 3000].map((n, i) => ({ label: n + ' kcal', value: i }));
const GOAL_SLEEP = [6, 7, 8, 9, 10].map((n, i) => ({ label: n + ' h', value: i }));
const GOAL_ACTIVE = [15, 30, 45, 60, 90].map((n, i) => ({ label: n + ' min', value: i }));
const GOAL_HR = [150, 160, 170, 180, 190, 200].map((n, i) => ({ label: n + ' bpm', value: i }));
// the watch keeps the goals in km, so the miles beside each one are only there to read by
const GOAL_DISTANCE = [
  '2 km (1.2 mi)',
  '3 km (1.9 mi)',
  '5 km (3.1 mi)',
  '8 km (5 mi)',
  '10 km (6.2 mi)',
  '15 km (9.3 mi)',
  '20 km (12.4 mi)',
].map((label, i) => ({ label: label, value: i }));

const HRV_STILL = [30, 40, 50, 60, 80, 100, 150, 200].map((n, i) => ({ label: String(n), value: i }));
const HRV_WALK = [5, 10, 15, 20, 30].map((n, i) => ({ label: n + ' steps', value: i }));
/**
 * The gaps between heartbeats each HRV reading leaves out at its start, in the order the page sends
 * them. The page sends the index and the watch keeps the counts, so its table has to match this one.
 */
const HRV_SETTLE_GAPS = [3, 5, 8, 10, 12, 15, 20];

/** The index the settle choice starts on, which is 10 gaps, the framework's own default. */
const HRV_SETTLE_DEFAULT = 3;

const HRV_SETTLE = HRV_SETTLE_GAPS.map((n, i) => ({ label: n + ' gaps', value: i }));

/** The HRV settings under their own subheading, for a face that has HRV panels. */
function hrvItems(): ClayConfigItem[] {
  return [
    subheading('HRV'),
    select('HEALTH_HRV_COUNT', 'HRV Readings', [
      { label: 'Not While Walking', value: 0 },
      { label: 'Only When Still', value: 1 },
      { label: 'Any Time', value: 2 },
    ], 0, 'Which readings the HRV panels count. Readings taken while walking run high, so Not While Walking keeps to resting ones.'),
    select('HEALTH_HRV_STILL', 'Still Up To', HRV_STILL, 3, "The most your wrist can move during a reading for it to count as still. The HRV Tuning panel shows each reading's movement, so you can set this from your own numbers."),
    select('HEALTH_HRV_WALK', 'Walking From', HRV_WALK, 1, "How many steps during a reading make it a walk, since wrist movement alone looks the same for typing and walking. The HRV Tuning panel shows each reading's steps."),
    select('HEALTH_HRV_SETTLE', 'Skip at Start', HRV_SETTLE, HRV_SETTLE_DEFAULT, "How many gaps between heartbeats each reading leaves out at the start while the sensor settles. The HRV Tuning panel's KEPT column shows the share of gaps each reading kept."),
  ];
}

/**
 * The whole section, ready to drop into the page.
 *
 * @param options What the face has. `hrv` adds the HRV settings for a face with HRV panels.
 * @return The Health section with its goals, units and the goal-met buzz.
 */
export function healthSection(options: { hrv?: boolean } = {}): ClayConfigItem {
  return {
    type: 'section',
    items: [
      heading('Health'),
      // steps lead since that is the goal most people set, and each unit sits beside its goal
      subheading('Activity'),
      select('HEALTH_GOAL_STEPS', 'Step Goal', GOAL_STEPS, 2),
      select('HEALTH_STEPS_MODE', 'Steps Panel Shows', [
        { label: 'Steps', value: 0 },
        { label: 'Miles', value: 1 },
        { label: 'Kilometres', value: 2 },
      ], 0, 'Your step count, or the distance you walked today.'),
      select('HEALTH_GOAL_DISTANCE', 'Distance Goal', GOAL_DISTANCE, 2),
      select('HEALTH_DISTANCE_UNIT', 'Distance Unit', [
        { label: 'Kilometres', value: 0 },
        { label: 'Miles', value: 1 },
      ], 0, 'The unit on the Distance and Distance Goal panels.'),
      select('HEALTH_GOAL_CALORIES', 'Calorie Goal', GOAL_CALORIES, 3),
      select('HEALTH_GOAL_ACTIVE', 'Activity Goal', GOAL_ACTIVE, 1),
      subheading('Sleep'),
      select('HEALTH_GOAL_SLEEP', 'Sleep Goal', GOAL_SLEEP, 2),
      subheading('Heart'),
      select('HEALTH_GOAL_HR', 'Heart Rate Limit', GOAL_HR, 3, 'The top of the bar on the 2x2 Heart Rate panel.'),
      ...(options.hrv ? hrvItems() : []),
      subheading('Goal Alerts'),
      select('HEALTH_GOAL_VIBE', 'Goal Met Vibration', GOAL_VIBE_OPTIONS, '', 'The watch buzzes the first time each day you reach your step, distance, calorie, or activity goal. The fanfares are longer rhythms, and Custom plays your own.'),
      {
        type: 'input',
        messageKey: 'HEALTH_GOAL_VIBE_CUSTOM',
        label: 'Custom Vibration Pattern',
        description: 'On and off times in milliseconds, separated by commas, starting with a buzz.',
        attributes: {
          placeholder: '100,80,100,80,300',
          maxlength: 120,
        },
        visibleWhen: { key: 'HEALTH_GOAL_VIBE', in: ['C'] },
      },
    ],
  };
}

export default { healthSection };
