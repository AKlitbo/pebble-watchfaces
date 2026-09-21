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

import type { ClayItem, ClayOption } from './types';
import { select, heading } from './config-rows';

// the Goal Met Vibe picks. the value IS what the watch plays, so the tune data lives here in the
// config, not in the watch binary. a one-letter sentinel is a plain pulse (S/L/D), C means the
// Custom Vibe Pattern box below, and a comma list of milliseconds is a fanfare rhythm the watch
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
const GOAL_DISTANCE = [2, 3, 5, 8, 10, 15, 20].map((n, i) => ({ label: n + ' km', value: i }));

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Health section with its goals, units and the goal-met buzz.
 */
export function healthSection(): ClayItem {
  return {
    type: 'section',
    items: [
      heading('Health Configuration'),
      select('HEALTH_GOAL_ACTIVE', 'Activity Goal', GOAL_ACTIVE, 1),
      select('HEALTH_GOAL_CALORIES', 'Calorie Goal', GOAL_CALORIES, 3),
      select('HEALTH_GOAL_HR', 'Heart-rate Limit', GOAL_HR, 3),
      select('HEALTH_GOAL_SLEEP', 'Sleep Goal', GOAL_SLEEP, 2),
      select('HEALTH_GOAL_STEPS', 'Step Goal', GOAL_STEPS, 2),
      select('HEALTH_STEPS_MODE', 'Steps Unit', [
        { label: 'Steps', value: 0 },
        { label: 'Miles', value: 1 },
        { label: 'Kilometers', value: 2 },
      ], 0, "What the Steps panel shows: your step count, or the distance you've walked today, in miles or kilometers."),
      select('HEALTH_GOAL_DISTANCE', 'Distance Goal', GOAL_DISTANCE, 2),
      select('HEALTH_DISTANCE_UNIT', 'Distance Unit', [
        { label: 'Miles', value: 1 },
        { label: 'Kilometers', value: 0 },
      ], 0, 'The unit for the standalone Distance panel, kept apart so it can differ from the Steps panel.'),
      select('HEALTH_GOAL_VIBE', 'Goal Met Vibration', GOAL_VIBE_OPTIONS, '', 'Buzz the first time you reach a daily goal (steps, calories, distance, or active minutes). The fanfares are little celebration rhythms. Pick Custom to use your own pattern below.'),
      {
        type: 'input',
        messageKey: 'HEALTH_GOAL_VIBE_CUSTOM',
        label: 'Custom Vibe Pattern',
        description: 'Your own rhythm as comma-separated on and off times in milliseconds, starting with a buzz. Used only when Goal Met Vibration is set to Custom.',
        attributes: {
          placeholder: '100,80,100,80,300',
          limit: 120,
        },
      },
    ],
  };
}

export default { healthSection };
