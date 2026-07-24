/**
 * Wire packing for the AppMessage strips.
 *
 * Turns the parsed weather forecast, stock, and calendar structures into the
 * byte layouts the watch decodes. The field order and widths here are a contract
 * with the C side: change one and the matching decoder has to change too.
 */

import type { HourlyStrip, DailyStrip } from '../weather/util';
import type { StockQuote } from '../stock/util';
import type { CalendarEvent } from '../calendar/ical';

// how many columns a forecast strip can carry. matches WEATHER_FORECAST_COLS on the watch,
// which re-clamps the count anyway, so slicing here just keeps the count byte honest and the
// packer in step with the stock and calendar ones
const FORECAST_MAX_COLS = 8;
// how many tickers the watchlist strip can carry. matches STOCK_MAX_SLOTS on the watch
const STOCK_MAX_SLOTS = 4;
// calendar strip caps matching CALENDAR_MAX_SLOTS / CAL_TITLE_LEN / CAL_LOC_LEN on the watch
const CALENDAR_MAX_SLOTS = 6;
const CALENDAR_TITLE_MAX = 24;
const CALENDAR_LOC_MAX = 16;
// a forecast column with no reading ships this sentinel so the watch draws a placeholder
const FORECAST_NO_TEMP = -1000;

/**
 * Clamps a number to the signed range of a little-endian field, rounded first.
 * A price or percent that overflows its field would wrap to a wild wrong value.
 */
function clampInt(value: number, bits: number): number {
  const limit = Math.pow(2, bits - 1);
  let rounded = Math.round(Number(value) || 0);
  if (rounded > limit - 1) {
    rounded = limit - 1;
  }
  if (rounded < -limit) {
    rounded = -limit;
  }
  return rounded;
}

/**
 * Encodes a signed value as two little-endian bytes. A missing reading rides as
 * the FORECAST_NO_TEMP sentinel so the watch draws a placeholder. The reading is clamped
 * because a provider handing back something wild would otherwise wrap to a plausible
 * looking wrong temperature rather than saturate.
 */
function int16Bytes(value: number | null | undefined): number[] {
  const reading = (value === null || value === undefined) ? FORECAST_NO_TEMP : clampInt(value, 16);
  const wrapped = reading < 0 ? reading + 0x10000 : reading;
  return [wrapped & 0xff, (wrapped >> 8) & 0xff];
}

/**
 * Pushes a length byte then the text as 7-bit ASCII, which is how every string field on the
 * wire is laid out. Callers slice the text to their own field width first.
 */
function pushAscii(bytes: number[], text: string): void {
  bytes.push(text.length);
  for (let i = 0; i < text.length; i++) {
    bytes.push(text.charCodeAt(i) & 0x7f);
  }
}

/**
 * Encodes a signed integer as little-endian bytes. Uses division rather than bit
 * shifts so a 32-bit value does not trip JS's signed shift operators.
 */
function leBytes(value: number, byteCount: number): number[] {
  let wrapped = value < 0 ? value + Math.pow(2, byteCount * 8) : value;
  const out = [];
  for (let i = 0; i < byteCount; i++) {
    out.push(wrapped % 256);
    wrapped = Math.floor(wrapped / 256);
  }
  return out;
}

/**
 * Packs the hourly forecast strip into the wire bytes the watch decodes.
 * Layout: [count][baseHour][stepHours] then per column [code][tempLow][tempHigh].
 */
function packForecastHourly(hourly: HourlyStrip | null | undefined): number[] | null {
  if (!hourly || !hourly.cols || !hourly.cols.length) {
    return null;
  }

  const cols = hourly.cols.slice(0, FORECAST_MAX_COLS);
  const bytes = [cols.length, hourly.baseHour & 0xff, hourly.stepHours & 0xff];
  cols.forEach((col) => {
    bytes.push(col.code & 0xff, ...int16Bytes(col.temp));
  });
  return bytes;
}

/**
 * Packs the 7-day forecast strip into the wire bytes the watch decodes.
 * Layout: [count][baseWeekday] then per column [code][maxLow][maxHigh][minLow][minHigh].
 */
function packForecastDaily(daily: DailyStrip | null | undefined): number[] | null {
  if (!daily || !daily.cols || !daily.cols.length) {
    return null;
  }

  const cols = daily.cols.slice(0, FORECAST_MAX_COLS);
  const bytes = [cols.length, daily.baseWeekday & 0xff];
  cols.forEach((col) => {
    bytes.push(col.code & 0xff, ...int16Bytes(col.tempMax), ...int16Bytes(col.tempMin));
  });
  return bytes;
}

/**
 * Packs the watchlist quotes into the wire bytes the watch decodes.
 * Layout: [count] then per slot [ok][price int32 LE cents][pct int16 LE
 * hundredths][symLen][sym bytes]. A failed slot carries its short status text.
 */
function packStockStrip(results: Array<Pick<StockQuote, 'ok' | 'price' | 'changePercent' | 'symbol' | 'status'>> | null): number[] | null {
  if (!results || !results.length) {
    return null;
  }

  const slots = results.slice(0, STOCK_MAX_SLOTS);
  const bytes = [slots.length];

  // the loop walks every index so an array hole still packs as a failed slot. a skipped slot
  // would leave the count byte above the number of records behind it and the watch bails out of
  // the whole strip and keeps its stale one
  for (let slot = 0; slot < slots.length; slot++) {
    const result = slots[slot];
    const ok = result && result.ok ? 1 : 0;
    // clampInt already rounds so pass the raw scaled value straight through
    const priceCents = clampInt((result && result.price || 0) * 100, 32);
    const pctHundredths = clampInt((result && result.changePercent || 0) * 100, 16);
    // a good slot carries its ticker while a failed one carries its status text so the
    // watch has something to show. cap to what the store's symbol buffer holds
    const label = String((result && (result.ok ? result.symbol : result.status)) || '').toUpperCase().slice(0, 11);

    bytes.push(ok);
    bytes.push(...leBytes(priceCents, 4));
    bytes.push(...leBytes(pctHundredths, 2));
    pushAscii(bytes, label);
  }

  return bytes;
}

/**
 * Packs upcoming calendar events into the wire bytes the watch decodes.
 * Layout: [count] then per event [startEpoch int32 LE][endEpoch int32 LE][flags bit0=allDay]
 * [titleLen][title bytes][locLen][loc bytes]. Absolute epochs so the watch keeps it fresh.
 */
function packCalendarStrip(events: CalendarEvent[] | null): number[] | null {
  if (!events || !events.length) {
    return null;
  }

  const slots = events.slice(0, CALENDAR_MAX_SLOTS);
  const bytes = [slots.length];

  slots.forEach((event) => {
    const title = String(event.title || '').slice(0, CALENDAR_TITLE_MAX);
    const location = String(event.location || '').slice(0, CALENDAR_LOC_MAX);

    // clamp both epochs into the int32 the watch reads them back as. a feed can carry an
    // open-ended DTEND far past 2038, and the low four bytes of that land back in 1969, which
    // puts the end before the start and every duration it feeds goes negative
    bytes.push(...leBytes(clampInt(event.startEpoch, 32), 4));
    bytes.push(...leBytes(clampInt(event.endEpoch, 32), 4));
    bytes.push(event.allDay ? 1 : 0);

    pushAscii(bytes, title);
    pushAscii(bytes, location);
  });

  return bytes;
}

/**
 * True when two packed strips are byte-for-byte identical, so a redundant push
 * to the watch can be skipped. Two nulls count as equal.
 */
function bytesEqual(left: number[] | null, right: number[] | null): boolean {
  if (left === right) {
    return true;
  }
  if (!left || !right || left.length !== right.length) {
    return false;
  }
  for (let index = 0; index < left.length; index++) {
    if (left[index] !== right[index]) {
      return false;
    }
  }
  return true;
}

export default { packForecastHourly, packForecastDaily, packStockStrip, packCalendarStrip, bytesEqual, STOCK_MAX_SLOTS };
