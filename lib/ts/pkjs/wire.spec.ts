/**
 * Specs for the AppMessage wire packing.
 *
 * These byte layouts are a contract with the C decoders, so the field order, the
 * little-endian widths, the no-reading sentinel, and the store-buffer caps are
 * pinned here: a swapped or dropped byte reads the wrong value on the watch.
 */

import { describe, test, expect } from 'vitest';
import wire from './wire';

describe('packForecastHourly', () => {
  /** The header order [count][baseHour][stepHours] is a wire contract with the C decoder. */
  test('writes the count, base hour and step ahead of the columns', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 9, stepHours: 2,
      cols: [{ code: 6, temp: 12 }, { code: 1, temp: 14 }],
    });

    expect(bytes.slice(0, 3)).toEqual([2, 9, 2]);
  });

  /** Each column is [code][tempLow][tempHigh] little-endian, so a swapped byte reads the wrong temp. */
  test('packs each column as code then little-endian temp', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 0, stepHours: 2, cols: [{ code: 6, temp: 300 }],
    });

    // 300 = 0x012C -> low 0x2C high 0x01
    expect(bytes.slice(3)).toEqual([6, 0x2c, 0x01]);
  });

  /** A negative temp must ship as two's-complement or the watch reads a huge positive number. */
  test('encodes a negative temperature as two-complement bytes', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 0, stepHours: 2, cols: [{ code: 0, temp: -5 }],
    });

    // -5 -> 0xFFFB
    expect(bytes.slice(3)).toEqual([0, 0xfb, 0xff]);
  });

  /** A missing reading must ride as the -1000 sentinel so the watch shows a gap, not a 0. */
  test('packs a null temp as the no-reading sentinel', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 0, stepHours: 2, cols: [{ code: 0, temp: null }],
    });

    // -1000 -> 0xFC18
    expect(bytes.slice(3)).toEqual([0, 0x18, 0xfc]);
  });

  /** A wild temp must saturate, or it wraps to a plausible looking wrong reading (99999 -> -31073). */
  test('clamps a temperature past the field to the top of the range', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 0, stepHours: 2, cols: [{ code: 0, temp: 99999 }],
    });

    // 32767 -> 0x7FFF
    expect(bytes.slice(3)).toEqual([0, 0xff, 0x7f]);
  });

  /** A wild negative temp must saturate at the floor, or it wraps to a plausible warm reading. */
  test('clamps a temperature past the bottom of the field to the low of the range', () => {
    const bytes = wire.packForecastHourly({
      baseHour: 0, stepHours: 2, cols: [{ code: 0, temp: -99999 }],
    });

    // -32768 -> 0x8000 little-endian
    expect(bytes.slice(3)).toEqual([0, 0x00, 0x80]);
  });

  /** An empty or missing strip must yield null so nothing is sent and the row keeps its placeholder. */
  test('returns null for a missing or empty strip', () => {
    expect(wire.packForecastHourly(null)).toBeNull();
    expect(wire.packForecastHourly({ baseHour: 0, stepHours: 2, cols: [] })).toBeNull();
  });

  /** More columns than the watch holds must slice to the cap, or the count byte overstates the strip. */
  test('caps the columns at the watch capacity', () => {
    const tenCols = [];
    for (let i = 0; i < 10; i++) {
      tenCols.push({ code: 0, temp: 10 });
    }

    const bytes = wire.packForecastHourly({ baseHour: 0, stepHours: 2, cols: tenCols });

    // count byte pinned to 8 and the body carries exactly 8 three-byte columns
    expect(bytes[0]).toBe(8);
    expect(bytes.length).toBe(3 + 8 * 3);
  });
});

describe('packForecastDaily', () => {
  /** The header [count][baseWeekday] and per-column [code][max][min] is the C decoder's contract. */
  test('writes the count and base weekday then each column high and low', () => {
    const bytes = wire.packForecastDaily({
      baseWeekday: 6, cols: [{ code: 0, tempMax: 25, tempMin: 15 }],
    });

    expect(bytes.slice(0, 2)).toEqual([1, 6]);
    // 25 -> 0x19 0x00 then 15 -> 0x0F 0x00
    expect(bytes.slice(2)).toEqual([0, 0x19, 0x00, 0x0f, 0x00]);
  });

  /** An empty or missing strip must yield null so nothing is sent. */
  test('returns null for a missing or empty strip', () => {
    expect(wire.packForecastDaily(null)).toBeNull();
    expect(wire.packForecastDaily({ baseWeekday: 0, cols: [] })).toBeNull();
  });

  /** More columns than the watch holds must slice to the cap, or the count byte overstates the strip. */
  test('caps the columns at the watch capacity', () => {
    const tenCols = [];
    for (let i = 0; i < 10; i++) {
      tenCols.push({ code: 0, tempMax: 20, tempMin: 10 });
    }

    const bytes = wire.packForecastDaily({ baseWeekday: 0, cols: tenCols });

    // count byte pinned to 8 and the body carries exactly 8 five-byte columns
    expect(bytes[0]).toBe(8);
    expect(bytes.length).toBe(2 + 8 * 5);
  });
});

describe('packStockStrip', () => {
  const okSlot = { ok: true, symbol: 'AAPL', price: 261.74, changePercent: 0.47 };

  /** The count leads the strip and the price rides as little-endian cents, the C decoder's contract. */
  test('writes the count then price as little-endian cents', () => {
    const bytes = wire.packStockStrip([okSlot]);

    // count 1 then ok 1 then 26174 cents = 0x663E -> 0x3E 0x66 0x00 0x00
    expect(bytes.slice(0, 2)).toEqual([1, 1]);
    expect(bytes.slice(2, 6)).toEqual([0x3e, 0x66, 0x00, 0x00]);
  });

  /** The percent rides as signed hundredths, so a fall must ship as two's-complement not a huge value. */
  test('encodes a negative percent as two-complement hundredths', () => {
    const bytes = wire.packStockStrip([{ ok: true, symbol: 'TSLA', price: 100, changePercent: -2.1 }]);

    // -210 hundredths -> 0xFF2E little-endian at offset 6..8
    expect(bytes.slice(6, 8)).toEqual([0x2e, 0xff]);
  });

  /** The symbol rides length-prefixed as ASCII so the watch can read the ticker back out. */
  test('length-prefixes the symbol bytes', () => {
    const bytes = wire.packStockStrip([okSlot]);

    // after count(1)+ok(1)+price(4)+pct(2) = offset 8: len 4 then A A P L
    expect(bytes.slice(8)).toEqual([4, 65, 65, 80, 76]);
  });

  /** A failed slot must carry its status text with ok = 0 so the watch can show "RATE LIMIT". */
  test('packs a failed slot status text with ok zero', () => {
    const bytes = wire.packStockStrip([{ ok: false, symbol: '', status: 'RATE LIMIT', price: 0, changePercent: 0 }]);

    expect(bytes[1]).toBe(0);
    // status rides in the symbol field so its length leads the text
    expect(bytes[8]).toBe('RATE LIMIT'.length);
  });

  /** An empty or missing list must yield null so nothing is sent and the panel keeps its placeholder. */
  test('returns null for a missing or empty list', () => {
    expect(wire.packStockStrip(null)).toBeNull();
    expect(wire.packStockStrip([])).toBeNull();
  });

  /**
   * A hole must still pack a record or the count outruns the records behind it and the C decoder
   * throws the whole strip away, losing the good quotes along with the missing ones.
   */
  test('packs a sparse slot so the count still matches the records', () => {
    const sparse = new Array(3);
    sparse[0] = okSlot;
    sparse[2] = { ok: true, symbol: 'F', price: 11.92, changePercent: 2.15 };

    const bytes = wire.packStockStrip(sparse);

    // count says 3 so three records must follow: ok + price(4) + pct(2) + len + label each
    expect(bytes[0]).toBe(3);
    const lengths = ['AAPL'.length, 0, 'F'.length];
    let offset = 1;
    for (const labelLength of lengths) {
      expect(bytes.length).toBeGreaterThanOrEqual(offset + 8 + labelLength);
      expect(bytes[offset + 7]).toBe(labelLength);
      offset += 8 + labelLength;
    }
    expect(bytes.length).toBe(offset);
  });

  /** The hole itself reads as a failed slot so the watch shows a placeholder, not a stale price. */
  test('packs a sparse slot as a failed quote', () => {
    const sparse = new Array(2);
    sparse[0] = okSlot;

    const bytes = wire.packStockStrip(sparse);

    // the second record starts after count(1) + the first record (8 + 4 label bytes)
    const second = 1 + 8 + 'AAPL'.length;
    expect(bytes[second]).toBe(0); // ok = 0
    expect(bytes.slice(second + 1, second + 5)).toEqual([0, 0, 0, 0]); // price = 0
  });
});

describe('packCalendarStrip epoch range', () => {
  /**
   * A feed can carry an open-ended DTEND centuries out. Keeping only the low four bytes of it
   * lands the end back in 1969, before the start, and every duration drawn from the pair goes
   * negative.
   */
  test('clamps a far-future end epoch instead of wrapping it negative', () => {
    const farFuture = 253402300799; // 9999-12-31, past what an int32 can carry
    const bytes = wire.packCalendarStrip([
      { title: 'Open ended', location: '', startEpoch: 1800000000, endEpoch: farFuture, allDay: false },
    ]);

    // the end rides at offset 5 (count 1 + start 4) and must saturate at INT32_MAX
    const end = bytes.slice(5, 9);

    expect(end).toEqual([0xff, 0xff, 0xff, 0x7f]);
  });

  /** The everyday case must be untouched, or the clamp would be rewriting real times. */
  test('leaves an ordinary epoch alone', () => {
    const bytes = wire.packCalendarStrip([
      { title: 'Sync', location: '', startEpoch: 1800000000, endEpoch: 1800003600, allDay: false },
    ]);

    // 1800003600 = 0x6B49E010, low byte first
    const end = bytes.slice(5, 9);

    expect(end).toEqual([0x10, 0xe0, 0x49, 0x6b]);
  });
});

describe('packCalendarStrip', () => {
  const event = { title: 'Standup', location: 'Room 2', startEpoch: 1000000, endEpoch: 1003600, allDay: false };

  /** The count leads the strip and the start rides as a little-endian epoch, the C decoder's contract. */
  test('writes the count then the start epoch as little-endian bytes', () => {
    const bytes = wire.packCalendarStrip([event]);

    // count 1 then 1000000 = 0x0F4240 little-endian
    expect(bytes[0]).toBe(1);
    expect(bytes.slice(1, 5)).toEqual([0x40, 0x42, 0x0f, 0x00]);
  });

  /** The all-day bit tells the watch to show a date with no time, so it must ride in the flags byte. */
  test('sets the all-day flag bit', () => {
    const bytes = wire.packCalendarStrip([{ ...event, allDay: true }]);

    // flags sit after count(1) + start(4) + end(4)
    expect(bytes[9]).toBe(1);
  });

  /** An over-long title or location must be capped to the store's buffers or the wire overruns them. */
  test('caps the title and location to the store buffer lengths', () => {
    const bytes = wire.packCalendarStrip([{
      ...event,
      title: 'x'.repeat(40),
      location: 'y'.repeat(40),
    }]);

    // titleLen sits after count(1) + start(4) + end(4) + flags(1)
    const titleLen = bytes[10];
    const locLen = bytes[10 + 1 + titleLen];

    expect(titleLen).toBe(24);
    expect(locLen).toBe(16);
  });

  /** More events than the strip can hold must be capped so the wire never overruns the store. */
  test('caps the event list at the strip size', () => {
    const many = Array.from({ length: 8 }, () => event);

    const bytes = wire.packCalendarStrip(many);

    expect(bytes[0]).toBe(6);
  });

  /** An empty or missing list must yield null so nothing is sent and the panel keeps its placeholder. */
  test('returns null for a missing or empty list', () => {
    expect(wire.packCalendarStrip(null)).toBeNull();
    expect(wire.packCalendarStrip([])).toBeNull();
  });
});

describe('bytesEqual', () => {
  /** The same bytes must compare equal so an unchanged strip skips the redundant push. */
  test('reports identical arrays as equal', () => {
    const result = wire.bytesEqual([1, 2, 3], [1, 2, 3]);

    expect(result).toBe(true);
  });

  /** A differing length means the strip moved, so it must send. */
  test('reports different lengths as not equal', () => {
    const result = wire.bytesEqual([1, 2, 3], [1, 2]);

    expect(result).toBe(false);
  });

  /** A single changed byte must count as a change so the new strip goes out. */
  test('reports a differing byte at the same length as not equal', () => {
    const result = wire.bytesEqual([1, 2, 3], [1, 9, 3]);

    expect(result).toBe(false);
  });

  /** Two nulls (nothing sent yet, nothing to send) count as equal so nothing fires. */
  test('reports two nulls as equal', () => {
    const result = wire.bytesEqual(null, null);

    expect(result).toBe(true);
  });

  /** A null against a real strip means a first send, so it must not be equal. */
  test('reports a null against a strip as not equal', () => {
    const result = wire.bytesEqual(null, [1, 2, 3]);

    expect(result).toBe(false);
  });

  /** Two empty strips match, so an empty-to-empty refresh skips the send. */
  test('reports two empty arrays as equal', () => {
    const result = wire.bytesEqual([], []);

    expect(result).toBe(true);
  });
});
