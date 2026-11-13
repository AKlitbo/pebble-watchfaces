/**
 * Clock rows both faces in the family put in their own Clock sections.
 *
 * Each face lays out its Clock section itself, since the rows around these differ. A row both of
 * them carry under the same key is built once here rather than written out twice and drifting.
 */

import type { ClayConfigItem } from '../../paf/ts/clay/types';
import { select, VIBE_OPTIONS } from '../../paf/ts/pkjs/config-rows';

/**
 * The hourly buzz picker, for a face's Clock section.
 *
 * @return The Hourly Vibration select, off by default.
 */
export function hourlyVibeItem(): ClayConfigItem {
  return select('CLOCK_HOURLY_VIBE', 'Hourly Vibration', VIBE_OPTIONS, 0, 'The watch buzzes on the hour, except during Quiet Time.');
}

export default { hourlyVibeItem };
