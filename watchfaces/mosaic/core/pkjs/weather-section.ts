/**
 * The Weather section of the settings page.
 *
 * Both faces in the family offer the same units and the same providers and store them under the
 * same keys, so the rows are built once here rather than written out twice and drifting.
 *
 * Open-Meteo is the default because it needs no key, so a fresh install shows real weather
 * without anyone having to sign up for anything.
 *
 * The Data Source comes first, with its key row straight under it, since the source decides
 * whether the key row shows at all. On a face built with the weather plugin, the framework's page
 * hook puts the select first anyway, so the section reads the same with the plugin or without it.
 */

import type { ClayConfigItem } from '../../paf/ts/clay/types';
import { select, heading, WIND_UNIT_OPTIONS } from '../../paf/ts/pkjs/config-rows';

/**
 * The whole section, ready to drop into the page.
 *
 * @return The Weather section with the provider picker, the key box and the units.
 */
export function weatherSection(): ClayConfigItem {
  return {
    type: 'section',
    items: [
      heading('Weather'),
      select('WEATHER_PROVIDER', 'Data Source', [
        { label: 'Open-Meteo (Free, No Key)', value: 'openmeteo' },
        { label: 'OpenWeatherMap (Needs Key)', value: 'owm' },
        { label: 'WeatherAPI.com (Needs Key)', value: 'weatherapi' },
      ], 'openmeteo', 'Where the phone gets the weather. Open-Meteo needs no key.'),
      {
        type: 'input',
        messageKey: 'WEATHER_API_KEY',
        label: 'API Key',
        description: 'Paste the key from your OpenWeatherMap or WeatherAPI.com account.',
        attributes: {
          placeholder: 'Paste your private API key here...',
          maxlength: 64,
        },
        // only the two providers that take a key show it. that also hides it while the weather
        // comes from a Pebble app plugin, and a hidden row still keeps and saves the key
        visibleWhen: { key: 'WEATHER_PROVIDER', in: ['owm', 'weatherapi'] },
      },
      select('WEATHER_TEMPERATURE_UNIT', 'Temperature Unit', [
        { label: 'Celsius (°C)', value: 0 },
        { label: 'Fahrenheit (°F)', value: 1 },
      ], 0),
      select('WEATHER_WIND_UNIT', 'Wind Speed Unit', WIND_UNIT_OPTIONS, 0),
    ],
  };
}

export default { weatherSection };
