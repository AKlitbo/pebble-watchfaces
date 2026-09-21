/**
 * The Location section of the settings page.
 *
 * Both faces in the family ask the same three questions and store them under the same keys, so
 * the rows are built once here rather than written out twice and drifting.
 *
 * The alternate time zone lives here rather than with the clock settings because it is answered
 * the same way as the manual location, by searching for a place.
 */

import type { ClayItem } from './types';
import { heading, toggle } from './config-rows';

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Location section with the GPS toggles and the two place searches.
 */
export function locationSection(): ClayItem {
  return {
    type: 'section',
    items: [
      heading('Location Settings'),
      toggle('LOCATION_USE_GPS', 'Enable Phone GPS', 'Automatically fetch weather for your current location.', true),
      toggle('LOCATION_GPS_FALLBACK', 'Fallback to Manual Location', 'If GPS is disabled or unavailable, use the city typed below.', true),
      {
        type: 'locationsearch',
        messageKey: 'LOCATION_NAME',
        label: 'Manual Location',
        attributes: {
          placeholder: 'Search a city, e.g. Phoenix',
        },
      },
      {
        type: 'locationsearch',
        messageKey: 'CLOCK_TIMEZONE_1',
        label: 'Alternate Time Zone',
        description: "Sets the local time displayed by the 'Time Zone 1' module in your layout. Search a city, a zone name such as Europe/London, or type UTC or an offset like UTC+05:30.",
        attributes: {
          placeholder: 'e.g. Phoenix, UTC, or Europe/London',
        },
      },
    ],
  };
}

export default { locationSection };
