/**
 * Specs for the iCal parser.
 *
 * The watch only ever sees what this file returns, so the coverage here is about the shapes a
 * real feed throws at us: UTC, zoned and floating times, all-day dates, folded lines, escaped
 * text, and the future-window filter that keeps the agenda short.
 *
 * Two of those carry most of the weight. A zoned time has to land on the right instant whether
 * the phone knows the zone or only the feed does, since being out by an hour looks exactly like
 * a calendar that works. And a repeating event has to be walked out into real occurrences, since
 * showing only its first one looks exactly like an empty calendar.
 *
 * The awkward calendar shapes are driven through a whole feed rather than poked at directly,
 * because a feed is the only input this ever gets and the reading of it is a library's job now.
 */

import { describe, test, expect } from 'vitest';
import ical from './ical';
import { fetchRequest } from '../../testing/fetch-request';

// a fixed clock so the window filter is deterministic: 2026-07-10 12:00 UTC
const NOW = Math.floor(Date.UTC(2026, 6, 10, 12, 0, 0) / 1000);

// wraps event blocks in a minimal VCALENDAR so the fixtures read like a real feed
function feed(body: string): string {
  return 'BEGIN:VCALENDAR\r\nVERSION:2.0\r\n' + body + 'END:VCALENDAR\r\n';
}

describe('parseIcal timed events', () => {
  /** A UTC start/end must decode to absolute epochs, or every event drifts by the wearer's offset. */
  test('reads a timed event with an explicit DTEND', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260710T173000Z\r\nDTEND:20260710T190000Z\r\nSUMMARY:Picnic with Rachel\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 17, 30, 0) / 1000));
    expect(result[0].endEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 19, 0, 0) / 1000));
    expect(result[0].allDay).toBe(false);
    expect(result[0].title).toBe('Picnic with Rachel');
  });

  /** A missing DTEND must default to a one-hour block so free/busy still has a cell to shade. */
  test('defaults a missing DTEND to one hour after the start', () => {
    const source = feed('BEGIN:VEVENT\r\nDTSTART:20260710T200000Z\r\nSUMMARY:Call Mom\r\nEND:VEVENT\r\n');

    const result = ical.parseIcal(source, NOW);

    const start = Math.floor(Date.UTC(2026, 6, 10, 20, 0, 0) / 1000);
    expect(result[0].startEpoch).toBe(start);
    expect(result[0].endEpoch).toBe(start + 60 * 60);
  });
});

// the shape a real feed sends: a zone is named by an event and defined by the calendar, in the
// same VCALENDAR. these are the live US rules, summer time from the second Sunday of March to
// the first Sunday of November. a feed that names a zone it never defines is malformed, and the
// reader has no way to place it, so there are no fixtures for that shape
const NEW_YORK_ZONE =
  'BEGIN:VTIMEZONE\r\nTZID:America/New_York\r\n' +
  'BEGIN:STANDARD\r\nDTSTART:16011104T020000\r\nTZOFFSETFROM:-0400\r\nTZOFFSETTO:-0500\r\n' +
  'RRULE:FREQ=YEARLY;BYMONTH=11;BYDAY=1SU\r\nEND:STANDARD\r\n' +
  'BEGIN:DAYLIGHT\r\nDTSTART:16010311T020000\r\nTZOFFSETFROM:-0500\r\nTZOFFSETTO:-0400\r\n' +
  'RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=2SU\r\nEND:DAYLIGHT\r\nEND:VTIMEZONE\r\n';

describe('parseIcal zoned events', () => {
  /**
   * A feed states a meeting in its own zone. Reading it on the phone's clock instead puts the
   * event out by the difference between the two, which is the whole bug: a New York 09:00
   * standup would read 09:00 to someone in London rather than 14:00.
   */
  test('reads a TZID time in its own zone not the phone local one', () => {
    const source = feed(
      NEW_YORK_ZONE +
      'BEGIN:VEVENT\r\nDTSTART;TZID=America/New_York:20260710T090000\r\n' +
      'DTEND;TZID=America/New_York:20260710T100000\r\nSUMMARY:Standup\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // 2026-07-10 is summer, so New York runs at UTC-4 and 09:00 there is 13:00 UTC
    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 13, 0, 0) / 1000));
    expect(result[0].endEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 14, 0, 0) / 1000));
  });

  /** The zone's own summer time has to be honoured, not a fixed offset for the zone. */
  test('applies the winter offset for a winter date in the same zone', () => {
    // a January window so the same zone sits at UTC-5 instead of UTC-4
    const winterNow = Math.floor(Date.UTC(2027, 0, 10, 12, 0, 0) / 1000);
    const source = feed(
      NEW_YORK_ZONE +
      'BEGIN:VEVENT\r\nDTSTART;TZID=America/New_York:20270111T090000\r\nSUMMARY:Standup\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, winterNow);

    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2027, 0, 11, 14, 0, 0) / 1000));
  });

  /** A Z time is already absolute, so the zone machinery must leave it exactly alone. */
  test('leaves a UTC time untouched', () => {
    const source = feed('BEGIN:VEVENT\r\nDTSTART:20260710T173000Z\r\nSUMMARY:Picnic\r\nEND:VEVENT\r\n');

    const result = ical.parseIcal(source, NOW);

    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 17, 30, 0) / 1000));
  });
});

describe('parseIcal feed-defined zones', () => {
  // the shape Outlook and Exchange send: a zone name the phone has never heard of, defined by the
  // feed itself. the two sub-blocks carry the US summer time rules
  const outlookZone =
    'BEGIN:VTIMEZONE\r\nTZID:Eastern Standard Time\r\n' +
    'BEGIN:STANDARD\r\nDTSTART:16011104T020000\r\nTZOFFSETFROM:-0400\r\nTZOFFSETTO:-0500\r\n' +
    'RRULE:FREQ=YEARLY;BYMONTH=11;BYDAY=1SU\r\nEND:STANDARD\r\n' +
    'BEGIN:DAYLIGHT\r\nDTSTART:16010311T020000\r\nTZOFFSETFROM:-0500\r\nTZOFFSETTO:-0400\r\n' +
    'RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=2SU\r\nEND:DAYLIGHT\r\nEND:VTIMEZONE\r\n';

  /**
   * Intl has never heard of "Eastern Standard Time", so without reading the feed's own definition
   * the event falls back to the wearer's clock and reads at whatever time that happens to be.
   */
  test('resolves a zone only the feed defines', () => {
    const source = feed(
      outlookZone +
      'BEGIN:VEVENT\r\nDTSTART;TZID=Eastern Standard Time:20260710T090000\r\n' +
      'DTEND;TZID=Eastern Standard Time:20260710T100000\r\nSUMMARY:Sync\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // July is inside the DAYLIGHT rule, so the zone runs at UTC-4 and 09:00 there is 13:00 UTC
    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 13, 0, 0) / 1000));
  });

  /** The feed's own summer time rules have to be followed, not just its first offset. */
  test('picks the winter offset from the feed for a winter date', () => {
    const winterNow = Math.floor(Date.UTC(2027, 0, 10, 12, 0, 0) / 1000);
    const source = feed(
      outlookZone +
      'BEGIN:VEVENT\r\nDTSTART;TZID=Eastern Standard Time:20270111T090000\r\nSUMMARY:Sync\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, winterNow);

    // January is inside the STANDARD rule, so the zone runs at UTC-5 and 09:00 there is 14:00 UTC
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2027, 0, 11, 14, 0, 0) / 1000));
  });

  /** A zone that never changes carries one sub-block and no rule at all. */
  test('resolves a fixed-offset zone with no summer time', () => {
    const source = feed(
      'BEGIN:VTIMEZONE\r\nTZID:Fixed Plus Five Thirty\r\n' +
      'BEGIN:STANDARD\r\nDTSTART:16010101T000000\r\nTZOFFSETFROM:+0530\r\nTZOFFSETTO:+0530\r\n' +
      'END:STANDARD\r\nEND:VTIMEZONE\r\n' +
      'BEGIN:VEVENT\r\nDTSTART;TZID=Fixed Plus Five Thirty:20260711T090000\r\nSUMMARY:Call\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // 09:00 at UTC+5:30 is 03:30 UTC
    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 11, 3, 30, 0) / 1000));
  });

  /** The feed's own definition wins, since a phone-known name can still be redefined. */
  test('prefers the feed definition over the phone zone database', () => {
    const source = feed(
      'BEGIN:VTIMEZONE\r\nTZID:America/New_York\r\n' +
      'BEGIN:STANDARD\r\nDTSTART:16010101T000000\r\nTZOFFSETFROM:+0000\r\nTZOFFSETTO:+0000\r\n' +
      'END:STANDARD\r\nEND:VTIMEZONE\r\n' +
      'BEGIN:VEVENT\r\nDTSTART;TZID=America/New_York:20260711T090000\r\nSUMMARY:Odd\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // the feed says this name means UTC, so 09:00 is 09:00 rather than Intl's 13:00
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 11, 9, 0, 0) / 1000));
  });

  /**
   * A parameter value is allowed to be quoted, and a zone name with spaces in it usually is. The
   * quotes are not part of the name, so leaving them on means the VTIMEZONE block defining that
   * very name is never found and the event quietly reads on the phone's clock instead.
   */
  test('resolves a quoted TZID the same as a bare one', () => {
    const source = feed(
      outlookZone +
      'BEGIN:VEVENT\r\nDTSTART;TZID="Eastern Standard Time":20260710T090000\r\nSUMMARY:Sync\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 10, 13, 0, 0) / 1000));
  });

  /** A name nothing can place still yields the event, just on the phone's clock. */
  test('falls back to the phone clock for a zone nothing defines', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART;TZID=Nowhere/Nothing:20260711T090000\r\nSUMMARY:Lost\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(
      Math.floor(new Date(2026, 6, 11, 9, 0, 0).getTime() / 1000)
    );
  });
});

describe('parseIcal recurring events', () => {
  /**
   * A weekly standup is one VEVENT plus a rule. Reading only its DTSTART shows the very first
   * occurrence from years ago, which the window then drops, so the agenda looks empty even
   * though the calendar is full.
   */
  test('expands a weekly rule into the occurrences inside the window', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240108T090000Z\r\nDTEND:20240108T093000Z\r\n' +
      'RRULE:FREQ=WEEKLY;BYDAY=MO\r\nSUMMARY:Standup\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // the window is 2026-07-10 (a Friday) plus six days, so it catches Monday the 13th
    expect(result).toHaveLength(1);
    expect(result[0].title).toBe('Standup');
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 13, 9, 0, 0) / 1000));
  });

  /** A daily rule must fill the window, not stop at one. */
  test('expands a daily rule across the whole window', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240108T080000Z\r\nDTEND:20240108T083000Z\r\n' +
      'RRULE:FREQ=DAILY\r\nSUMMARY:Meds\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // today's 08:00 has already gone at the 12:00 NOW, so the window holds the next six
    expect(result).toHaveLength(6);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 11, 8, 0, 0) / 1000));
    expect(result[5].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 16, 8, 0, 0) / 1000));
  });

  /** INTERVAL has to be honoured or a fortnightly meeting shows up every week. */
  test('honours INTERVAL on a daily rule', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260711T080000Z\r\nDTEND:20260711T083000Z\r\n' +
      'RRULE:FREQ=DAILY;INTERVAL=3\r\nSUMMARY:Bins\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result.map((event) => event.startEpoch)).toEqual([
      Math.floor(Date.UTC(2026, 6, 11, 8, 0, 0) / 1000),
      Math.floor(Date.UTC(2026, 6, 14, 8, 0, 0) / 1000),
    ]);
  });

  /** UNTIL ends the series, so a rule that already lapsed must contribute nothing. */
  test('stops a rule at its UNTIL', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240108T090000Z\r\nDTEND:20240108T093000Z\r\n' +
      'RRULE:FREQ=WEEKLY;BYDAY=MO;UNTIL=20250101T000000Z\r\nSUMMARY:Old standup\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(0);
  });

  /** COUNT ends the series after a set number, however far back it started. */
  test('stops a rule after its COUNT', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260713T090000Z\r\nDTEND:20260713T093000Z\r\n' +
      'RRULE:FREQ=DAILY;COUNT=2\r\nSUMMARY:Two days\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result.map((event) => event.startEpoch)).toEqual([
      Math.floor(Date.UTC(2026, 6, 13, 9, 0, 0) / 1000),
      Math.floor(Date.UTC(2026, 6, 14, 9, 0, 0) / 1000),
    ]);
  });

  /** BYDAY can name several days, so one rule lands more than once a week. */
  test('expands a BYDAY rule onto each named weekday', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240108T090000Z\r\nDTEND:20240108T093000Z\r\n' +
      'RRULE:FREQ=WEEKLY;BYDAY=MO,WE\r\nSUMMARY:Gym\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result.map((event) => event.startEpoch)).toEqual([
      Math.floor(Date.UTC(2026, 6, 13, 9, 0, 0) / 1000),
      Math.floor(Date.UTC(2026, 6, 15, 9, 0, 0) / 1000),
    ]);
  });

  /** A cancelled occurrence must not show, or the watch nags about a meeting that is off. */
  test('drops an occurrence named by EXDATE', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240108T090000Z\r\nDTEND:20240108T093000Z\r\n' +
      'RRULE:FREQ=WEEKLY;BYDAY=MO,WE\r\nEXDATE:20260713T090000Z\r\nSUMMARY:Gym\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result.map((event) => event.startEpoch)).toEqual([
      Math.floor(Date.UTC(2026, 6, 15, 9, 0, 0) / 1000),
    ]);
  });

  /** A moved occurrence rides as its own VEVENT and must replace the one the rule made. */
  test('lets a RECURRENCE-ID event replace the occurrence it names', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nUID:gym@example.com\r\nDTSTART:20240108T090000Z\r\nDTEND:20240108T093000Z\r\n' +
      'RRULE:FREQ=WEEKLY;BYDAY=MO\r\nSUMMARY:Gym\r\nEND:VEVENT\r\n' +
      'BEGIN:VEVENT\r\nUID:gym@example.com\r\nRECURRENCE-ID:20260713T090000Z\r\n' +
      'DTSTART:20260713T180000Z\r\nDTEND:20260713T183000Z\r\nSUMMARY:Gym moved\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].title).toBe('Gym moved');
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 13, 18, 0, 0) / 1000));
  });

  /**
   * A feed can send just the moved copy of a meeting, its RRULE master trimmed away by whatever
   * synced it. Dropping the orphan because its rule is gone loses a real event off the agenda, so
   * it has to read as a plain one on its own.
   */
  test('keeps a RECURRENCE-ID event whose master rule is not in the feed', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nUID:lost@example.com\r\nRECURRENCE-ID:20260713T090000Z\r\n' +
      'DTSTART:20260713T180000Z\r\nDTEND:20260713T183000Z\r\nSUMMARY:Orphan moved\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].title).toBe('Orphan moved');
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 13, 18, 0, 0) / 1000));
  });

  /**
   * A monthly meeting is usually pinned to a weekday, not a date: "the second Tuesday". Stepping
   * by the start's day-of-month instead lands on the wrong day every month, and when that day
   * falls outside the window the event just silently never shows.
   */
  test('expands a monthly BYDAY rule onto the nth weekday of the month', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240109T090000Z\r\nDTEND:20240109T093000Z\r\n' +
      'RRULE:FREQ=MONTHLY;BYDAY=2TU\r\nSUMMARY:Board\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    // the second Tuesday of July 2026 is the 14th, which the window covers
    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 14, 9, 0, 0) / 1000));
  });

  /** A negative ordinal counts back from the end of the month. */
  test('reads a negative BYDAY ordinal as counting back from the month end', () => {
    // the last Friday of July 2026 is the 31st, so a window in late July catches it
    const lateJuly = Math.floor(Date.UTC(2026, 6, 27, 12, 0, 0) / 1000);
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240126T170000Z\r\nDTEND:20240126T173000Z\r\n' +
      'RRULE:FREQ=MONTHLY;BYDAY=-1FR\r\nSUMMARY:Retro\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, lateJuly);

    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 31, 17, 0, 0) / 1000));
  });

  /** A plain monthly rule with no BYDAY still steps by the day of the month it started on. */
  test('keeps a plain monthly rule on its day of the month', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20240113T090000Z\r\nDTEND:20240113T093000Z\r\n' +
      'RRULE:FREQ=MONTHLY\r\nSUMMARY:Rent\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 13, 9, 0, 0) / 1000));
  });

  /** A plain event alongside a rule must be untouched by any of the expansion. */
  test('leaves a non-recurring event alone', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260711T090000Z\r\nDTEND:20260711T100000Z\r\nSUMMARY:One off\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result).toHaveLength(1);
    expect(result[0].startEpoch).toBe(Math.floor(Date.UTC(2026, 6, 11, 9, 0, 0) / 1000));
  });
});

describe('parseIcal all-day events', () => {
  /** An all-day date must flag allDay and span a full day, or the agenda tries to show a time. */
  test('reads a date-only event as all-day spanning 24h', () => {
    const source = feed('BEGIN:VEVENT\r\nDTSTART;VALUE=DATE:20260711\r\nSUMMARY:Public holiday\r\nEND:VEVENT\r\n');

    const result = ical.parseIcal(source, NOW);

    const start = Math.floor(new Date(2026, 6, 11, 0, 0, 0).getTime() / 1000);
    expect(result[0].allDay).toBe(true);
    expect(result[0].startEpoch).toBe(start);
    expect(result[0].endEpoch).toBe(start + 24 * 60 * 60);
  });
});

describe('parseIcal text handling', () => {
  /** A folded SUMMARY must stitch back together, or a long title arrives truncated at the fold. */
  test('unfolds a title split across continuation lines', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260710T173000Z\r\nSUMMARY:Quarterly planning\r\n  and budget review\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result[0].title).toBe('Quarterly planning and budget review');
  });

  /** Escaped commas and a LOCATION must decode cleanly, since both feed the card's text. */
  test('unescapes the summary and reads the location', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260710T173000Z\r\nSUMMARY:Lunch\\, then a walk\r\nLOCATION:Central Park\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    expect(result[0].title).toBe('Lunch, then a walk');
    expect(result[0].location).toBe('Central Park');
  });
});

describe('parseIcal window and ordering', () => {
  /** Finished and far-off events must be dropped and the rest sorted, or the agenda shows junk. */
  test('drops past and beyond-window events and sorts the rest', () => {
    const source = feed(
      'BEGIN:VEVENT\r\nDTSTART:20260712T090000Z\r\nSUMMARY:Later this week\r\nEND:VEVENT\r\n' +
      'BEGIN:VEVENT\r\nDTSTART:20260710T090000Z\r\nDTEND:20260710T100000Z\r\nSUMMARY:Already over\r\nEND:VEVENT\r\n' +
      'BEGIN:VEVENT\r\nDTSTART:20260710T150000Z\r\nSUMMARY:Coming up\r\nEND:VEVENT\r\n' +
      'BEGIN:VEVENT\r\nDTSTART:20260720T090000Z\r\nSUMMARY:Beyond the window\r\nEND:VEVENT\r\n'
    );

    const result = ical.parseIcal(source, NOW);

    const titles = result.map((event) => event.title);
    expect(titles).toEqual(['Coming up', 'Later this week']);
  });
});

/**
 * The awkward calendar shapes, each of which was once read wrong here.
 *
 * These are the cases worth keeping whatever engine is underneath, because every one of them
 * fails silently: a birthday on the wrong day or a meeting that never appears looks exactly like
 * a calendar that works. The answers come from a real calendar, not from any implementation.
 * July 2026 opens on a Wednesday, so its Tuesdays are 7/14/21/28 and its Sundays 5/12/19/26.
 */
describe('parseIcal awkward recurrences', () => {
  const rule = (dtstart: string, rrule: string) => feed(
    'BEGIN:VEVENT\r\nUID:r\r\nDTSTART:' + dtstart + '\r\nRRULE:' + rrule + '\r\nSUMMARY:R\r\nEND:VEVENT\r\n'
  );
  const at = (year: number, month: number, day: number) => Math.floor(Date.UTC(year, month - 1, day, 12) / 1000);
  const days = (events: { startEpoch: number }[]) =>
    events.map((event) => new Date(event.startEpoch * 1000).toISOString().slice(0, 10));

  /**
   * A birthday is the commonest yearly event there is and it names no weekday, so the start's
   * own day is the only thing saying when it lands. Losing that puts every one of them on the
   * 1st of its month, where the window never sees it.
   */
  test('keeps a plain yearly rule on the day it started', () => {
    const result = rule('20200713T090000Z', 'FREQ=YEARLY');

    expect(days(ical.parseIcal(result, at(2026, 7, 10)))).toEqual(['2026-07-13']);
  });

  /** A rule on the 31st has to step over the months that are too short and carry on. */
  test('skips a short month and still lands on the next long one', () => {
    const result = rule('20260131T090000Z', 'FREQ=MONTHLY');

    expect(days(ical.parseIcal(result, at(2026, 3, 28)))).toEqual(['2026-03-31']);
  });

  /** The second Tuesday of July 2026 is the 14th, a weekday the rule picks rather than a date. */
  test('lands a monthly rule on the nth weekday of the month', () => {
    const result = rule('20240109T090000Z', 'FREQ=MONTHLY;BYDAY=2TU');

    expect(days(ical.parseIcal(result, at(2026, 7, 12)))).toEqual(['2026-07-14']);
  });

  /** A negative ordinal counts back, so it follows a month's own length. */
  test('counts a negative ordinal back from the month end', () => {
    const result = rule('20260131T090000Z', 'FREQ=MONTHLY;BYMONTHDAY=-1');

    expect(days(ical.parseIcal(result, at(2026, 2, 25)))).toEqual(['2026-02-28']);
  });

  /**
   * A week is counted from Monday, so Sunday closes it rather than opening it. Counting out from
   * the wrong end lands every Sunday of a fortnightly rule seven days adrift, and a rule that
   * runs every week hides it because its weeks touch.
   */
  test('lands a fortnightly rule on the right side of the skipped week', () => {
    const result = rule('20260713T090000Z', 'FREQ=WEEKLY;INTERVAL=2;BYDAY=MO,SU');

    // the rule runs the week of Monday the 13th, so its Sunday is the 19th, not the 12th or 26th
    expect(days(ical.parseIcal(result, at(2026, 7, 14)))).toEqual(['2026-07-19']);
  });

  /**
   * A COUNT rule cannot be jumped forward to the window, since the only way to know how many
   * have gone is to walk them. Giving up part way hands back nothing, which reads as an empty
   * calendar rather than a late one.
   */
  test('walks a long COUNT rule all the way to a window years later', () => {
    const result = rule('20150101T090000Z', 'FREQ=DAILY;COUNT=9999');

    // the window opens at noon and the rule fires at 09:00 for an hour, so that day's own
    // occurrence has already finished and the six behind it are what is left
    expect(days(ical.parseIcal(result, at(2026, 7, 11)))).toEqual([
      '2026-07-12', '2026-07-13', '2026-07-14', '2026-07-15', '2026-07-16', '2026-07-17',
    ]);
  });

  /**
   * A known ical.js deviation, pinned rather than worked around.
   *
   * RFC 5545 3.3.10 says a recurrence landing on a date that does not exist MUST be ignored, so a
   * leap day birthday should only fall on the leap years. ical.js slides it to the 1st of March
   * instead, and is not even consistent with itself: it drops an impossible day for a monthly
   * rule (see the 31st above) and slides it for a yearly one.
   *
   * Left as it is on purpose. For a watchface, showing the birthday on the 1st beats hiding it
   * three years in four, and this is the library's to fix rather than ours to patch. If an
   * upstream release corrects it, this test goes red and says so.
   */
  test('slides a leap day to the 1st of march, which the rules say it should not', () => {
    const result = rule('20240229T090000Z', 'FREQ=YEARLY');

    expect(days(ical.parseIcal(result, at(2027, 2, 25)))).toEqual(['2027-03-01']);
  });

  /** And on a real leap year it lands where it belongs. */
  test('lands a leap day on the leap year itself', () => {
    const result = rule('20240229T090000Z', 'FREQ=YEARLY');

    expect(days(ical.parseIcal(result, at(2028, 2, 25)))).toEqual(['2028-02-29']);
  });
});

describe('parseIcal bad input', () => {
  /**
   * A fetch can hand back an error page or a truncated body. The panels read an empty list as
   * "nothing on", which beats taking the whole app down over it.
   */
  test('reads something that is not iCal as no events', () => {
    const result = ical.parseIcal('<html>gateway timeout</html>', NOW);

    expect(result).toEqual([]);
  });

  /** An empty feed is the everyday case for a calendar with nothing in the window. */
  test('reads an empty string as no events', () => {
    const result = ical.parseIcal('', NOW);

    expect(result).toEqual([]);
  });
});

// live integration against a real iCal feed. opt-in via RUN_LIVE_CALENDAR=1 plus a
// CALENDAR_ICS_URL in .env (your Google "Secret address in iCal format"). fetches and parses
// your real feed and prints the next few events so the fetch/parse path can be eyeballed end
// to end and a feed shape the parser mishandles shows up here
describe.skipIf(process.env.RUN_LIVE_CALENDAR !== '1' || !process.env.CALENDAR_ICS_URL)('live', () => {
  const url = process.env.CALENDAR_ICS_URL;
  const live = () => new Promise<{ error: string | null; body?: string }>((resolve) => fetchRequest(url, (error, body) => resolve({ error: error, body: body })));

  /** The real feed must fetch and parse into a usable, sorted list of upcoming events. */
  test('parses upcoming events from the real feed', async () => {
    const response = await live();

    expect(response.error).toBe(null);

    const events = ical.parseIcal(response.body);

    console.log('Live calendar: ' + events.length + ' upcoming event(s)');
    events.slice(0, 5).forEach((event) => {
      const when = event.allDay ? 'all-day' : new Date(event.startEpoch * 1000).toLocaleString();
      console.log('  ' + when + '  ' + event.title + (event.location ? '  @ ' + event.location : ''));
    });

    events.forEach((event) => {
      expect(typeof event.startEpoch).toBe('number');
      expect(event.endEpoch).toBeGreaterThanOrEqual(event.startEpoch);
      expect(typeof event.title).toBe('string');
    });
  });
});
