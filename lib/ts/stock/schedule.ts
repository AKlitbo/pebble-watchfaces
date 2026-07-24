/**
 * US-market-hours throttling for the stock providers.
 *
 * Works out the trading phase of the US market and, from it, whether a given
 * provider is worth polling right now. Real-time feeds always fetch, while the
 * quota-limited ones ease off when the market is shut or their data can't have
 * moved yet. Kept next to the stock layer it governs rather than in the pkjs
 * bootstrap.
 */

const MINUTE_MS = 60 * 1000;
// Twelve Data honours the watch's interval while the market is open floored so 4 symbols
// stay under its 800/day cap (~384/day at 15 min). when the market is shut its price is
// frozen so it drops to an occasional refresh that still catches the next open
const TD_OPEN_FLOOR_MS = 15 * MINUTE_MS;
const TD_CLOSED_FLOOR_MS = 3 * 60 * MINUTE_MS;
// Alpha Vantage publishes the day's close at no fixed time after the 16:00 ET bell so once the
// market shuts we poll every couple of hours until its trading day catches up then idle till
// tomorrow. it only allows 25 calls a day and spends one per symbol so the whole six hour window
// has to fit 4 symbols with room to spare. two hours gives three tries for 12 calls
const AV_POLL_FLOOR_MS = 120 * MINUTE_MS;

/**
 * Breaks an instant into its US Eastern wall-clock parts, the zone US markets keep.
 * weekday is 0=Sunday..6=Saturday, and date is the ET calendar day as "YYYY-MM-DD".
 */
function etParts(now: number): { weekday: number; hour: number; minute: number; date: string } {
  const date = new Date(now);
  // one formatter carries both the clock and the calendar day so they never disagree
  // across a zone boundary. en-CA gives the YYYY-MM-DD date shape
  const parts = new Intl.DateTimeFormat('en-CA', {
    timeZone: 'America/New_York',
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    hour12: false,
  }).formatToParts(date);

  const lookup: Record<string, string> = {};
  parts.forEach((part) => { lookup[part.type] = part.value; });

  // hour comes back as "24" at midnight in some engines so fold it to 0
  const hour = Number(lookup.hour) % 24;
  // derive the weekday from the ET calendar day itself not a locale short-name string
  // (some engines spell it differently and a bad match would read undefined and let a
  // weekend fall through as an open trading day and over-poll paid providers)
  const weekday = new Date(Date.UTC(Number(lookup.year), Number(lookup.month) - 1, Number(lookup.day))).getUTCDay();

  return {
    weekday: weekday,
    hour: hour,
    minute: Number(lookup.minute),
    date: `${lookup.year}-${lookup.month}-${lookup.day}`,
  };
}

/**
 * The trading phase of the US market at the given instant: 'open' (Mon-Fri
 * 09:30-16:00 ET), 'postclose' (Mon-Fri 16:00-22:00 ET), or 'closed'.
 */
function marketPhase(now: number): string {
  const et = etParts(now);
  if (et.weekday === 0 || et.weekday === 6) {
    return 'closed';
  }

  const minutes = et.hour * 60 + et.minute;
  if (minutes >= 9 * 60 + 30 && minutes < 16 * 60) {
    return 'open';
  }
  if (minutes >= 16 * 60 && minutes < 22 * 60) {
    return 'postclose';
  }
  return 'closed';
}

/**
 * Whether a stock poll should be skipped, so a provider only fetches when its data
 * could actually have moved. Finnhub and Yahoo are real-time and always fetch.
 * Twelve Data honours the interval while open and eases off when shut. Alpha
 * Vantage only polls after the close and stops once it has today's trading day.
 *
 * The two Alpha Vantage stamps only bound anything if they outlive a restart, so they are read
 * back off the phone (see cache.ts) rather than starting empty on every run.
 */
function shouldThrottleStockFetch(provider: string, force: boolean, lastFetchMs: number, lastAsOf: string, now: number): boolean {
  if (force) {
    return false;
  }

  const sinceLast = now - lastFetchMs;

  if (provider === 'twelvedata') {
    const floor = marketPhase(now) === 'open' ? TD_OPEN_FLOOR_MS : TD_CLOSED_FLOOR_MS;
    return sinceLast < floor;
  }

  if (provider === 'alphavantage') {
    // already holding today's close so nothing new lands until tomorrow
    if (lastAsOf && lastAsOf === etParts(now).date) {
      return true;
    }
    // the close only publishes after the bell so only chase it in the evening window
    if (marketPhase(now) === 'postclose') {
      return sinceLast < AV_POLL_FLOOR_MS;
    }
    return true;
  }

  // finnhub and yahoo and anything unknown are real-time so honour the watch's interval
  return false;
}

export default { marketPhase, shouldThrottleStockFetch };
