/**
 * Specs for the shared weather helpers.
 *
 * These are pure functions that turn upstream data into what the watchface
 * renders. A wrong threshold, alias, or truncation shows the user the wrong
 * weather word or breaks the layout, so they are tested exhaustively.
 */

import { describe, test, expect, vi } from 'vitest';
import util from './util.js';
import conditions from './conditions.js';

describe('wmoToCondition', () => {
  /** A wrong code boundary shows the wrong sky word (e.g. snow reported as rain). */
  test.each([
    [0, 'CLEAR'],
    [1, 'PCLDY'],
    [2, 'PCLDY'],
    [3, 'CLDY'],
    [48, 'FOGGY'],
    [51, 'DRZL'],
    [55, 'DRZL'],
    [57, 'FZDZ'],
    [61, 'RAIN'],
    [65, 'RAIN'],
    [67, 'FZRN'],
    [71, 'SNOW'],
    [75, 'SNOW'],
    [77, 'SNOW'],
    [80, 'SHWR'],
    [82, 'SHWR'],
    [86, 'SNSH'],
    [95, 'STRM'],
    [99, 'STRM']
  ])('maps WMO code %i to %s', (code, expected) => {
    const result = util.wmoToCondition(code);

    expect(result).toBe(expected);
  });

  /** An out-of-range code must fall back to a safe word, never render undefined. */
  test('maps a code above the known range to Unknown', () => {
    const result = util.wmoToCondition(100);

    expect(result).toBe('UNKNOWN');
  });
});

describe('condition vocabulary single source of truth', () => {
  /**
   * Every token the JS producer can emit must exist in conditions.js, which
   * generates the C icon table. A token the C side lacks renders WI_NA silently,
   * so this guards the two sides from drifting (UNKNOWN is the deliberate fallback).
   */
  test('every producible token (day and night) is in conditions.js (or the UNKNOWN fallback)', () => {
    // mirror the C table: every row contributes its day token plus its
    // "<token>_NIGHT" form when it has a night glyph
    const known = new Set();
    for (const entry of conditions.conditions) {
      known.add(entry.token);
      if (entry.nightResource) {
        known.add(`${entry.token}_NIGHT`);
      }
    }

    const emitted = new Set();
    for (let code = 0; code <= 99; code++) {
      const day = util.wmoToCondition(code);
      emitted.add(day);
      // the night promotion of every producible day code must also resolve
      emitted.add(util.applyNight(day, false));
    }

    const unmapped = [...emitted].filter((token) => token !== 'UNKNOWN' && !known.has(token));

    expect(unmapped).toEqual([]);
  });
});

describe('applyNight', () => {
  /** A clear sky at night must become its night variant so the watch shows a moon, not a sun. */
  test('promotes Clear to Clear Night when it is not day', () => {
    const result = util.applyNight('Clear', false);

    expect(result).toBe('CLEAR_NIGHT');
  });

  /** A clear sky by day must stay Clear so the watch shows the sun. */
  test('leaves Clear unchanged during the day', () => {
    const result = util.applyNight('Clear', true);

    expect(result).toBe('CLEAR');
  });

  /** Every condition ships a night glyph, so after dark each must promote. A
   *  rainy or cloudy night that kept the daytime icon was the bug this prevents. */
  test.each([
    ['Partly cloudy', 'PCLDY_NIGHT'],
    ['Cloudy', 'CLDY_NIGHT'],
    ['Rain', 'RAIN_NIGHT'],
    ['Snow', 'SNOW_NIGHT'],
    ['Thunderstorm', 'STRM_NIGHT']
  ])('promotes %s to %s after dark', (condition, expected) => {
    const result = util.applyNight(condition, false);

    expect(result).toBe(expected);
  });

  /** By day the same conditions must resolve to their plain token (no night form). */
  test.each([
    ['Partly cloudy', 'PCLDY'],
    ['Cloudy', 'CLDY'],
    ['Rain', 'RAIN'],
    ['Snow', 'SNOW']
  ])('leaves %s as its day token during the day', (condition, expected) => {
    const result = util.applyNight(condition, true);

    expect(result).toBe(expected);
  });

  /** An unrecognized phrase has no night glyph, so it must fall back to UNKNOWN.
   *  Never an X_NIGHT token the C table can't resolve. */
  test('returns UNKNOWN for an unrecognized condition even at night', () => {
    const result = util.applyNight('Sharknado', false);

    expect(result).toBe('UNKNOWN');
  });
});

describe('shorten', () => {
  /** A verbose, un-aliased phrase must map to UNKNOWN, never a truncated fragment.
   *  A partial word the C icon table can't match would render the wrong/NA icon. */
  test('maps an unrecognized phrase to Unknown instead of truncating', () => {
    const result = util.shorten('Moderate or heavy freezing rain');

    expect(result).toBe('UNKNOWN');
  });

  /** An 11-char string sits exactly at the limit and must pass through untouched. */
  test('leaves an 11-char string unchanged', () => {
    const result = util.shorten('CLEAR_NIGHT');

    expect(result).toBe('CLEAR_NIGHT');
  });

  /** Without aliasing, verbose provider phrases overflow the watch field. */
  test.each([
    ['Sunny', 'CLEAR'],
    ['Partly cloudy', 'PCLDY'],
    ['Overcast', 'CLDY'],
    ['Cloudy', 'CLDY'],
    ['Patchy rain nearby', 'RAIN'],
    ['Thunderstorm', 'STRM'],
    ['Light snow', 'SNOW'],
    ['Mist', 'FOGGY']
  ])('aliases %s to %s', (input, expected) => {
    const result = util.shorten(input);

    expect(result).toBe(expected);
  });

  /** A night-promoted token must still be recognized as known, or the second
   *  shorten() pass in ok() would downgrade "RAIN_NIGHT" to UNKNOWN and drop the moon. */
  test('treats a night-promoted token as a known token', () => {
    const result = util.shorten('RAIN_NIGHT');

    expect(result).toBe('RAIN_NIGHT');
  });

  /** A null/empty condition must render a safe word, never blank or "undefined". */
  test.each([[''], [null], [undefined]])('returns Unknown for falsy input (%s)', (input) => {
    const result = util.shorten(input);

    expect(result).toBe('UNKNOWN');
  });
});

describe('degToCompass', () => {
  /** A wrong sector boundary labels the wind from the opposite quarter. */
  test.each([
    [0, 'N'],
    [22.5, 'NNE'],
    [45, 'NE'],
    [90, 'E'],
    [180, 'S'],
    [270, 'W'],
    [315, 'NW'],
    [360, 'N']
  ])('maps %i degrees to %s', (degrees, expected) => {
    const result = util.degToCompass(degrees);

    expect(result).toBe(expected);
  });

  /** A bearing past 360 must wrap, never index past the compass table. */
  test('wraps a bearing above 360 degrees', () => {
    const result = util.degToCompass(450);

    expect(result).toBe('E');
  });

  /** A missing bearing must yield '' so the watch shows no direction, not "undefined". */
  test.each([[undefined], [null], ['']])('returns empty for a non-numeric bearing (%s)', (input) => {
    const result = util.degToCompass(input);

    expect(result).toBe('');
  });
});

describe('hmFromIso', () => {
  /** The clock portion of a local ISO timestamp must be read out verbatim. */
  test('extracts HH:MM from an ISO timestamp', () => {
    const result = util.hmFromIso('2026-06-26T06:30');

    expect(result).toBe('06:30');
  });

  /** A malformed or missing timestamp must yield '' so the watch shows a placeholder. */
  test.each([['nope'], [''], [null], [undefined]])('returns empty for an unusable timestamp (%s)', (input) => {
    const result = util.hmFromIso(input);

    expect(result).toBe('');
  });
});

describe('hmFromUnix', () => {
  /** A UTC unix time plus the location offset must read out as the local clock. */
  test('formats unix UTC plus offset as local HH:MM', () => {
    // epoch (00:00 UTC) + 6h30m offset = 06:30 local
    const result = util.hmFromUnix(0, 6 * 3600 + 30 * 60);

    expect(result).toBe('06:30');
  });

  /** A negative offset must roll the clock back into the previous hours. */
  test('applies a negative offset', () => {
    // one day past the epoch (00:00 UTC) - 7h = 17:00 the day before
    const result = util.hmFromUnix(24 * 3600, -7 * 3600);

    expect(result).toBe('17:00');
  });

  /** A missing value must yield '' rather than "NaN:NaN". */
  test.each([
    [undefined, 0],
    [1782432000, undefined]
  ])('returns empty when a value is missing (unix=%s, offset=%s)', (unix, offset) => {
    const result = util.hmFromUnix(unix, offset);

    expect(result).toBe('');
  });
});

describe('hmFrom12Hour', () => {
  /** A botched 12-hour parse shows WeatherAPI's sunrise/sunset at the wrong time on the watch. */
  test.each([
    ['05:42 AM', '05:42'],
    ['5:42 AM', '05:42'],
    ['11:59 AM', '11:59'],
    ['05:42 PM', '17:42'],
    ['11:59 PM', '23:59']
  ])('converts %s to 24-hour %s', (input, expected) => {
    const result = util.hmFrom12Hour(input);

    expect(result).toBe(expected);
  });

  /** The 12 o'clock hour is the off-by-twelve trap: 12 AM is midnight, 12 PM is noon. */
  test.each([
    ['12:00 AM', '00:00'],
    ['12:30 AM', '00:30'],
    ['12:00 PM', '12:00'],
    ['12:30 PM', '12:30']
  ])('maps the 12 o\'clock boundary %s to %s', (input, expected) => {
    const result = util.hmFrom12Hour(input);

    expect(result).toBe(expected);
  });

  /** An unparseable time must yield '' so the watch shows a placeholder, never "NaN:NaN". */
  test.each([['not a time'], [null], [undefined]])('returns empty for invalid input (%s)', (input) => {
    const result = util.hmFrom12Hour(input);

    expect(result).toBe('');
  });
});

describe('safeParse', () => {
  /** A valid body must parse so the reading can be shown. */
  test('parses valid JSON into an object', () => {
    const result = util.safeParse('{"temp":12}');

    expect(result).toEqual({ temp: 12 });
  });

  /** A malformed body must yield null (not throw) so the caller can show an error state. */
  test('returns null for malformed JSON instead of throwing', () => {
    const result = util.safeParse('not json');

    expect(result).toBeNull();
  });
});

describe('requestJson', () => {
  /** A structured error body (e.g. a provider's 401 JSON) must still reach the provider so it can classify it, not be swallowed as a network error. */
  test('hands a parseable body to onJson even when the request reported an http error', () => {
    const request = (_url, callback) => callback('http 401', '{"cod":401}');
    const onJson = vi.fn();

    util.requestJson('https://example', request, () => {}, onJson);

    expect(onJson).toHaveBeenCalledWith({ cod: 401 });
  });

  /** A failed request whose body cannot be parsed must become a NET ERROR status, never a fake reading. */
  test('returns a NET ERROR status when the request failed and the body is unparseable', () => {
    const request = (_url, callback) => callback('http 503', '<html>service down</html>');
    let result;

    util.requestJson('https://example', request, (status) => { result = status; }, () => {});

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('NET ERROR');
  });

  /** A 2xx whose body is not JSON must surface as BAD WX DATA rather than success. */
  test('returns a BAD WX DATA status when a successful body is unparseable', () => {
    const request = (_url, callback) => callback(null, 'not json');
    let result;

    util.requestJson('https://example', request, (status) => { result = status; }, () => {});

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('BAD WX DATA');
  });

  /** A clean success must parse and route to the provider. */
  test('hands a parseable success body to onJson', () => {
    const request = (_url, callback) => callback(null, '{"temp":12}');
    const onJson = vi.fn();

    util.requestJson('https://example', request, () => {}, onJson);

    expect(onJson).toHaveBeenCalledWith({ temp: 12 });
  });
});

describe('ok', () => {
  /** The watch renders an integer, since an unrounded float would overflow the field. */
  test('rounds the temperature and marks the result ok', () => {
    const result = util.ok(13.6, 'Clear', 'Town');

    expect(result).toEqual({
      temperature: 14,
      condition: 'CLEAR',
      location: 'Town',
      ok: true
    });
  });

  /** A provider response missing the temperature must not ship NaN as a real reading. */
  test('returns a No Wx Data status when the temperature is not finite', () => {
    const result = util.ok(undefined, 'Clear', 'Town');

    expect(result.ok).toBe(false);
    expect(result.condition).toBe('NO WX DATA');
  });

  /** The condition must be passed through shorten so long phrases never reach the watch. */
  test('shortens the condition text', () => {
    const result = util.ok(10, 'Light rain', 'Town');

    expect(result.condition).toBe('RAIN');
  });

  /** A null location must become '' so the watch never renders the word "null". */
  test('falls back to an empty location label', () => {
    const result = util.ok(10, 'Clear', null);

    expect(result.location).toBe('');
  });

  /** The resolved coordinates must ride along when both are real, so the watch can show lat/lon. */
  test('attaches lat/lon when both are numbers', () => {
    const result = util.ok(10, 'Clear', 'Town', 51.5, -0.12);

    expect(result.lat).toBe(51.5);
    expect(result.lon).toBe(-0.12);
  });

  /** Without both coordinates the watch must show no lat/lon, never a phantom 0,0. */
  test.each([
    [undefined, undefined],
    [51.5, undefined],
    [undefined, -0.12]
  ])('omits lat/lon unless both are numbers (lat=%s, lon=%s)', (lat, lon) => {
    const result = util.ok(10, 'Clear', 'Town', lat, lon);

    expect(result).not.toHaveProperty('lat');
    expect(result).not.toHaveProperty('lon');
  });

  /** The extra readings must ride along, rounded, so the watch can show them. */
  test('attaches the weather extras when provided', () => {
    const extra = {
      humidity: 61.4, windKmh: 12.7, windDir: 'NW', sunrise: '06:30', sunset: '21:30', uvIndex: 4.8, precip: 1.25,
      feelsLike: 9.6, pressure: 1013.4, cloud: 75.6, windGustKmh: 30.2,
      dewPoint: -2.4, tempMax: 18.6, tempMin: 9.2, precipChance: 80.4, uvMax: 7.6, precipTotal: 4.25
    };

    const result = util.ok(10, 'Clear', 'Town', 51.5, -0.12, extra);

    expect(result.humidity).toBe(61);
    expect(result.windKmh).toBe(13);
    expect(result.windDir).toBe('NW');
    expect(result.sunrise).toBe('06:30');
    expect(result.sunset).toBe('21:30');
    expect(result.uvIndex).toBe(5);
    expect(result.precip).toBe(125);
    expect(result.feelsLike).toBe(10);
    expect(result.pressure).toBe(1013);
    expect(result.cloud).toBe(76);
    expect(result.windGustKmh).toBe(30);
    expect(result.dewPoint).toBe(-2);
    expect(result.tempMax).toBe(19);
    expect(result.tempMin).toBe(9);
    expect(result.precipChance).toBe(80);
    expect(result.uvMax).toBe(8);
    expect(result.precipTotal).toBe(425); // 4.25mm -> hundredths
  });

  /** A genuine zero reading (0% cloud, calm wind) must ship - a truthy check would wrongly drop it. */
  test('keeps zero-valued numeric extras instead of dropping them', () => {
    const result = util.ok(10, 'Clear', 'Town', undefined, undefined, { cloud: 0, windKmh: 0 });

    expect(result.cloud).toBe(0);
    expect(result.windKmh).toBe(0);
  });

  /** A reading with no extras object must stay the bare shape, never carry undefined fields. */
  test('omits the extras when none are provided', () => {
    const result = util.ok(10, 'Clear', 'Town');

    expect(result).not.toHaveProperty('humidity');
    expect(result).not.toHaveProperty('windKmh');
    expect(result).not.toHaveProperty('sunrise');
  });

  /** A missing single extra must be dropped, not shipped as NaN or "". */
  test('skips individual extras that are missing or non-numeric', () => {
    const extra = { humidity: undefined, windKmh: 'x', windDir: '', sunrise: '06:30' };

    const result = util.ok(10, 'Clear', 'Town', undefined, undefined, extra);

    expect(result).not.toHaveProperty('humidity');
    expect(result).not.toHaveProperty('windKmh');
    expect(result).not.toHaveProperty('windDir');
    expect(result.sunrise).toBe('06:30');
  });
});
