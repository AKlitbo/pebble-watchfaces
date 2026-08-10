/**
 * Specs for the US-market-hours stock throttling.
 *
 * The phase boundaries and the per-provider polling floors decide how much of a
 * paid provider's quota the watch spends, so the session edges (the 09:30 and
 * 16:00 bells, the weekend, the post-close window) are pinned down here.
 */

import { describe, test, expect, vi } from 'vitest';
import schedule from './schedule';

// build an epoch ms for a given US Eastern wall-clock time. July is EDT (UTC-4) so the
// ET hour maps to UTC by adding 4 (Date.UTC folds any overflow into the next day). In
// 2026 July 1 is a Wednesday and July 4 a Saturday so the phase can be steered by day
function etMs(day: number, hourEt: number, minuteEt = 0): number {
  return Date.UTC(2026, 6, day, hourEt + 4, minuteEt);
}

// the same for a winter date, when ET is EST (UTC-5) instead. in 2026 January 5 is a Monday
function estMs(day: number, hourEt: number, minuteEt = 0): number {
  return Date.UTC(2026, 0, day, hourEt + 5, minuteEt);
}

const FIFTEEN_MIN = 15 * 60 * 1000;
const THREE_HOURS = 3 * 60 * 60 * 1000;
const NINETY_MIN = 90 * 60 * 1000;
const TWO_HOURS = 2 * 60 * 60 * 1000;
const TODAY_ET = '2026-07-01';

describe('marketPhase', () => {
  /** The regular session drives Twelve Data's tight polling, so its bounds must be right. */
  test('reads a weekday mid-session as open', () => {
    const result = schedule.marketPhase(etMs(1, 10, 0));

    expect(result).toBe('open');
  });

  /** The 09:30 bell is the open edge, so it must count as open not pre-market. */
  test('counts the 09:30 open bell as open', () => {
    const result = schedule.marketPhase(etMs(1, 9, 30));

    expect(result).toBe('open');
  });

  /** The 16:00 bell flips the session into the post-close window Alpha Vantage waits on. */
  test('counts the 16:00 close bell as postclose', () => {
    const result = schedule.marketPhase(etMs(1, 16, 0));

    expect(result).toBe('postclose');
  });

  /** Late evening is past the window where the close is worth chasing, so it reads closed. */
  test('reads late evening as closed', () => {
    const result = schedule.marketPhase(etMs(1, 22, 0));

    expect(result).toBe('closed');
  });

  /** The weekend has no session, so even midday must read closed. */
  test('reads a weekend midday as closed', () => {
    const result = schedule.marketPhase(etMs(4, 12, 0));

    expect(result).toBe('closed');
  });

  /** The weekday is derived from the ET date parts, not the locale weekday name: an engine that spells the short weekday differently would miss the old lookup table, read undefined, and let a weekend fall through as an open session that over-polls paid providers. */
  test('reads a weekend as closed even when the engine spells the weekday differently', () => {
    // a Sunday (2026-07-05) whose short weekday is a non-English spelling the old
    // { Sun..Sat } table would not contain
    vi.stubGlobal('Intl', {
      DateTimeFormat: function () {
        return {
          formatToParts: () => [
            { type: 'weekday', value: 'dimanche' },
            { type: 'year', value: '2026' },
            { type: 'month', value: '07' },
            { type: 'day', value: '05' },
            { type: 'hour', value: '12' },
            { type: 'minute', value: '00' },
          ],
        };
      },
    });

    const result = schedule.marketPhase(Date.UTC(2026, 6, 5, 16, 0));

    expect(result).toBe('closed');
    vi.unstubAllGlobals();
  });
});

describe('marketPhase across daylight saving', () => {
  /**
   * The session is Eastern wall-clock, so the UTC offset it sits at is -4 in summer and -5 in
   * winter. Every other case here is a July one, and a fixed summer offset would pass all of them
   * while running the whole winter session an hour out: Alpha Vantage would burn quota chasing a
   * close that has not rung, and Twelve Data would spend the last open hour on its closed floor.
   */
  test('reads the winter open bell as open, an hour off the summer offset', () => {
    const result = schedule.marketPhase(estMs(5, 9, 30));

    expect(result).toBe('open');
  });

  /** A minute before the winter bell must still be closed, which a summer offset would call open. */
  test('reads a minute before the winter open bell as closed', () => {
    const result = schedule.marketPhase(estMs(5, 9, 29));

    expect(result).toBe('closed');
  });

  /** The last minute of the winter session is still open, not already post-close. */
  test('reads the last minute of the winter session as open', () => {
    const result = schedule.marketPhase(estMs(5, 15, 59));

    expect(result).toBe('open');
  });

  /** The winter close bell flips the session the same way the summer one does. */
  test('reads the winter close bell as postclose', () => {
    const result = schedule.marketPhase(estMs(5, 16, 0));

    expect(result).toBe('postclose');
  });

  /** The clearest statement of the contract: one UTC clock time, two seasons, two phases. */
  test('puts the same UTC time on opposite sides of the bell in summer and winter', () => {
    const summer = schedule.marketPhase(Date.UTC(2026, 6, 1, 13, 45));
    const winter = schedule.marketPhase(Date.UTC(2026, 0, 5, 13, 45));

    expect(summer).toBe('open');
    expect(winter).toBe('closed');
  });
});

describe('shouldThrottleStockFetch', () => {
  describe('twelvedata', () => {
    /** While the market is open Twelve Data honours the interval down to the 15-min quota floor. */
    test('throttles a poll inside the open 15-minute floor', () => {
      const now = etMs(1, 10, 0);

      const result = schedule.shouldThrottleStockFetch('twelvedata', false, now - (FIFTEEN_MIN - 1), '', now);

      expect(result).toBe(true);
    });

    /** Past the 15-min floor an open-market poll must go, or the quote sits stale during trading. */
    test('lets an open poll through once the 15-minute floor has passed', () => {
      const now = etMs(1, 10, 0);

      const result = schedule.shouldThrottleStockFetch('twelvedata', false, now - FIFTEEN_MIN, '', now);

      expect(result).toBe(false);
    });

    /** With the market shut the price is frozen, so it eases back to the 3-hour floor. */
    test('throttles a closed-market poll inside the 3-hour floor', () => {
      const now = etMs(1, 3, 0);

      const result = schedule.shouldThrottleStockFetch('twelvedata', false, now - (THREE_HOURS - 1), '', now);

      expect(result).toBe(true);
    });

    /** Past the 3-hour floor a closed-market poll goes, so it catches the next open promptly. */
    test('lets a closed poll through once the 3-hour floor has passed', () => {
      const now = etMs(1, 3, 0);

      const result = schedule.shouldThrottleStockFetch('twelvedata', false, now - THREE_HOURS, '', now);

      expect(result).toBe(false);
    });
  });

  describe('alphavantage', () => {
    /** The close only publishes after the bell, so an in-session poll must not spend quota. */
    test('idles during the open session', () => {
      const now = etMs(1, 10, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, 0, '', now);

      expect(result).toBe(true);
    });

    /** After the close it waits a couple of hours between tries, so a fresh poll waits. */
    test('throttles a post-close poll inside the two-hour floor', () => {
      const now = etMs(1, 18, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, now - (TWO_HOURS - 1), '', now);

      expect(result).toBe(true);
    });

    /**
     * The floor is what keeps the 16:00-22:00 window inside the 25-a-day allowance: at two hours
     * it fits three tries, and 4 symbols spend a call each, so 12 of 25. Drop it back to an hour
     * and the same window fits six tries for 24 of 25, which one forced fetch tips over.
     */
    test('throttles a post-close poll ninety minutes after the last one', () => {
      const now = etMs(1, 18, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, now - NINETY_MIN, '', now);

      expect(result).toBe(true);
    });

    /** Once the floor has passed it tries again, or it might never see the close land. */
    test('lets a post-close poll through once the two hours have passed', () => {
      const now = etMs(1, 18, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, now - TWO_HOURS, '', now);

      expect(result).toBe(false);
    });

    /** Once it holds today's trading day there is nothing new until tomorrow, so it stops. */
    test('idles after it has captured today\'s close', () => {
      const now = etMs(1, 18, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, 0, TODAY_ET, now);

      expect(result).toBe(true);
    });

    /** Overnight, before the next session, there is still nothing new to fetch. */
    test('idles overnight', () => {
      const now = etMs(1, 3, 0);

      const result = schedule.shouldThrottleStockFetch('alphavantage', false, 0, '', now);

      expect(result).toBe(true);
    });
  });

  describe('real-time providers', () => {
    /** Finnhub is real-time so throttling it would make the user's chosen refresh interval a lie. */
    test('never throttles a finnhub poll', () => {
      const result = schedule.shouldThrottleStockFetch('finnhub', false, 0, '', etMs(1, 3, 0));

      expect(result).toBe(false);
    });

    /** Yahoo is real-time too, so it always honours the interval whatever the market phase. */
    test('never throttles a yahoo poll', () => {
      const result = schedule.shouldThrottleStockFetch('yahoo', false, 0, '', etMs(1, 3, 0));

      expect(result).toBe(false);
    });
  });

  /** A settings change may have swapped the ticker or provider so a forced fetch ignores the gate. */
  test('lets a forced fetch through even when the gate would hold it', () => {
    const now = etMs(1, 18, 0);

    const result = schedule.shouldThrottleStockFetch('alphavantage', true, 0, TODAY_ET, now);

    expect(result).toBe(false);
  });
});
