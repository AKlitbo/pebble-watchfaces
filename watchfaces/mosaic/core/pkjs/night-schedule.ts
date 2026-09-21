/**
 * The Clay rows that set when the night layout takes over.
 *
 * Both faces in the family ask the same three questions and store them under the same keys, so
 * the rows are built once here rather than written out twice and drifting.
 *
 * The wording names both ends of the swap on purpose. Naming only sunset reads as a one-way trip
 * into night with no way back. The two clocks double as the fixed schedule and as the fallback
 * whenever the watch has no sun readings, so nobody has to be asked what happens before the
 * weather has arrived.
 */

import type { ClayItem } from './types';

/** How the swap is triggered. The values line up with the watch's own NightSchedMode. */
const MODE_OPTIONS = [
  { label: 'Never', value: 0 },
  { label: 'At Sunset & Sunrise', value: 1 },
  { label: 'At Custom Times', value: 2 },
];

// half-hour steps, matching the slot the watch stores. a free-text time would need its own
// validation and would hit the empty-string problem the layout sentinel exists for
const HALF_HOURS = Array.from({ length: 48 }, (_, slot) => {
  const hour = Math.floor(slot / 2);
  const minute = slot % 2 ? '30' : '00';
  return { label: (hour < 10 ? '0' : '') + hour + ':' + minute, value: slot };
});

/**
 * The three rows, ready to drop into a config section under the layout builder.
 *
 * @return The mode picker and the two clocks, in the order they should appear.
 */
export function nightScheduleItems(): ClayItem[] {
  return [
    {
      type: 'select',
      messageKey: 'LAYOUT_NIGHT_MODE',
      label: 'Swap Day & Night Layouts',
      defaultValue: 0,
      options: MODE_OPTIONS,
      description: 'When the watch swaps between your day and night layouts. At Sunset & Sunrise follows the times your weather provider reports, and falls back to the times below whenever it has none yet.',
    },
    {
      type: 'select',
      messageKey: 'LAYOUT_NIGHT_START',
      label: 'Night Starts',
      defaultValue: 42, // 21:00
      options: HALF_HOURS,
      description: 'When the night layout takes over. Used for custom times, and as the fallback whenever the watch has no sunset reading.',
    },
    {
      type: 'select',
      messageKey: 'LAYOUT_NIGHT_END',
      label: 'Night Ends',
      defaultValue: 14, // 07:00
      options: HALF_HOURS,
      description: 'And when the day layout comes back.',
    },
  ];
}

export default { nightScheduleItems };
