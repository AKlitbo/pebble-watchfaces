/**
 * The Location section of the settings page.
 *
 * Both faces in the family ask the same three questions and store them under the same keys, so
 * the rows are built once here rather than written out twice and drifting.
 *
 * The alternate time zone lives here rather than with the clock settings because it is answered
 * the same way as the manual location, by searching for a place.
 */

import type { ClayConfigItem } from '../../paf/ts/clay/types';
import { heading, toggle } from '../../paf/ts/pkjs/config-rows';

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Location section with the GPS toggles and the two place searches.
 */
export function locationSection(): ClayConfigItem {
  return {
    type: 'section',
    items: [
      heading('Location Settings'),
      // the GPS toggles and the manual city only feed a weather fetch, so they hide while the
      // weather comes from a Pebble app plugin. a face built without a plugin define ignores the mark.
      // the heading stays unmarked since the time zone below still sits under it
      { ...toggle('LOCATION_USE_GPS', 'Enable Phone GPS', 'Automatically fetch weather for your current location.', true), pluginSourceHides: 'WEATHER_PROVIDER' },
      { ...toggle('LOCATION_GPS_FALLBACK', 'Fallback to Manual Location', 'If GPS is on but cannot find you, use the city picked below.', true), pluginSourceHides: 'WEATHER_PROVIDER' },
      {
        type: 'locationsearch',
        messageKey: 'LOCATION_NAME',
        label: 'Manual Location',
        pluginSourceHides: 'WEATHER_PROVIDER',
        attributes: {
          placeholder: 'Search a city, e.g. Phoenix',
        },
      },
      // the time zone stays whatever the weather source, since the clock reads it
      {
        type: 'locationsearch',
        messageKey: 'CLOCK_TIMEZONE_1',
        timeZone: true,
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
