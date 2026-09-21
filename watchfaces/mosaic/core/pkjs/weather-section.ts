/**
 * The Weather section of the settings page.
 *
 * Both faces in the family offer the same units and the same providers and store them under the
 * same keys, so the rows are built once here rather than written out twice and drifting.
 *
 * Open-Meteo is the default because it needs no key, so a fresh install shows real weather
 * without anyone having to sign up for anything.
 */

import type { ClayItem } from './types';
import { select, heading } from './config-rows';

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Weather section with its units, the provider picker and the key box.
 */
export function weatherSection(): ClayItem {
  return {
    type: 'section',
    items: [
      heading('Weather Preferences'),
      select('WEATHER_TEMPERATURE_UNIT', 'Temperature Unit', [
        { label: 'Celsius (°C)', value: 0 },
        { label: 'Fahrenheit (°F)', value: 1 },
      ], 0),
      select('WEATHER_WIND_UNIT', 'Wind Speed Unit', [
        { label: 'Kilometers/hour (km/h)', value: 0 },
        { label: 'Miles/hour (mph)', value: 1 },
        { label: 'Knots (kts)', value: 2 },
        { label: 'Meters/second (m/s)', value: 3 },
      ], 0),
      select('WEATHER_PROVIDER', 'Data Source', [
        { label: 'Open-Meteo (Free, No Key Required)', value: 'openmeteo' },
        { label: 'OpenWeatherMap', value: 'owm' },
        { label: 'WeatherAPI.com', value: 'weatherapi' },
      ], 'openmeteo', 'Choose where your watch pulls its weather data. Open-Meteo works right out of the box with no setup required.'),
      {
        type: 'input',
        messageKey: 'WEATHER_API_KEY',
        label: 'API Key',
        description: 'Only required if you selected OpenWeatherMap or WeatherAPI above.',
        attributes: {
          placeholder: 'Paste your private API key here...',
          limit: 64,
        },
      },
    ],
  };
}

export default { weatherSection };
