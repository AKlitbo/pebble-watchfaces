/**
 * The coordinates Radar Array sends the watch.
 *
 * The latitude key carries the whole readout as one string, such as "34.05N 118.24W", and the
 * longitude key carries the longitude alone. The watch shows only the latitude key, but it only
 * saves a fix for the next launch when both keys hold a number.
 */
import type { WeatherResult } from '../../paf/ts/weather/util';

/**
 * Formats one signed coordinate as a magnitude plus hemisphere letter.
 * E.g. 34.0522 -> "34.05N", -118.2437 -> "118.24W".
 */
function fmtCoord(value: number, positive: string, negative: string): string {
  const hemisphere = value >= 0 ? positive : negative;

  return Math.abs(value).toFixed(2) + hemisphere;
}

/** A fix needs both halves as real numbers, so half a reply is never shown or saved as one. */
function hasFix(result: WeatherResult): result is WeatherResult & { lat: number; lon: number } {
  return Number.isFinite(result.lat) && Number.isFinite(result.lon);
}

/**
 * Builds the two location keys for a weather reply.
 *
 * @param messageKeys The face's message key ids.
 * @param result The weather reply, carrying the coordinates when the phone found them.
 * @return The latitude key with the whole readout and the longitude key with the longitude alone.
 */
export function coordFields(messageKeys: Record<string, number>, result: WeatherResult): Record<number, string> {
  if (!hasFix(result)) {
    // no number in either key, so the watch keeps its last good fix
    return { [messageKeys.LOCATION_LATITUDE]: 'NO LOCK', [messageKeys.LOCATION_LONGITUDE]: '' };
  }

  const longitude = fmtCoord(result.lon, 'E', 'W');

  return {
    // the watch shows the latitude key, so the whole readout travels in it
    [messageKeys.LOCATION_LATITUDE]: fmtCoord(result.lat, 'N', 'S') + ' ' + longitude,
    // the longitude also goes in its own key, so the watch has a real pair to save
    [messageKeys.LOCATION_LONGITUDE]: longitude,
  };
}
