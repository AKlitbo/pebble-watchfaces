/**
 * The Weather Location section of the settings page, and the alternate time zone row.
 *
 * Both faces in the family ask the same questions and store them under the same keys, so the rows
 * are built once here rather than written out twice and drifting.
 *
 * Every row in the section only feeds a weather fetch. The time zone row is searched the same way
 * as the manual location, so it is built here too, but each face puts it in its Clock section,
 * since the clock reads it whatever the weather source.
 */

import type { ClayConfigItem } from '../../paf/ts/clay/types';
import { heading, toggle } from '../../paf/ts/pkjs/config-rows';

// the heading and every row only mean something for a weather fetch, so they all hide while the
// weather comes from a Pebble app plugin, and the card folds away with them. a face built without
// a plugin define ignores the mark
const HIDES = { pluginSourceHides: 'WEATHER_PROVIDER' };

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Weather Location section with the GPS toggles and the manual city.
 */
export function locationSection(): ClayConfigItem {
  return {
    type: 'section',
    items: [
      { ...heading('Weather Location'), ...HIDES },
      { ...toggle('LOCATION_USE_GPS', 'Use Phone GPS', 'The phone looks up where you are for the weather.', true), ...HIDES },
      {
        ...toggle('LOCATION_GPS_FALLBACK', 'Fall Back to Manual Location', 'When GPS cannot find you, the phone uses the city below.', true),
        ...HIDES,
        visibleWhen: { key: 'LOCATION_USE_GPS', in: [true] },
      },
      {
        type: 'locationsearch',
        messageKey: 'LOCATION_NAME',
        label: 'Manual Location',
        description: 'The city for the weather when GPS is off or cannot find you.',
        ...HIDES,
        attributes: {
          placeholder: 'Search a city, e.g. Phoenix',
        },
      },
    ],
  };
}

/**
 * The alternate time zone row, for a face's Clock section.
 *
 * @return The time zone search that sets the Time Zone 1 panel.
 */
export function timeZoneItem(): ClayConfigItem {
  return {
    type: 'locationsearch',
    messageKey: 'CLOCK_TIMEZONE_1',
    timeZone: true,
    label: 'Alternate Time Zone',
    description: 'The time on the Time Zone 1 panel. Search a city or a zone such as Europe/London, or type UTC or an offset like UTC+05:30.',
    attributes: {
      placeholder: 'e.g. Phoenix, UTC, or Europe/London',
    },
  };
}

export default { locationSection, timeZoneItem };
