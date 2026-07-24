/**
 * The stock fetch cache the phone keeps between runs.
 *
 * PebbleKit JS gets killed and restarted whenever the phone feels like it, and everything the
 * fetch layer knows dies with it. That matters for a quota-limited provider: the throttle in
 * schedule.ts works out whether a poll is worth spending a call on by looking at when we last
 * fetched and which trading day we already hold, and both of those reset to nothing on a restart,
 * so the gate reopens and Alpha Vantage gets polled all over again.
 *
 * Keeping the last good strip alongside them means a watch with an empty store still has something
 * to show while the gate is shut, which is the whole point of the gate being shut.
 *
 * Kept next to the stock layer it serves rather than in the pkjs bootstrap, same as schedule.ts.
 */

/** What survives a restart: the two throttle stamps plus the last strip worth showing. */
export interface StockCache {
  lastAsOf: string;
  lastFetchMs: number;
  strip: number[] | null;
}

/** The slice of localStorage this needs, passed in so the specs can hand it a fake. */
export interface CacheStorage {
  getItem(key: string): string | null;
  setItem(key: string, value: string): void;
}

// its own key on purpose. the Clay settings blob is rewritten wholesale by Clay's own config
// handler and diffed key by key elsewhere, so anything extra parked in there would be clobbered
const CACHE_KEY = 'stock-cache';

/** A packed strip is only worth keeping if it is really a list of numbers we can hand the watch. */
function readStrip(value: unknown): number[] | null {
  if (!Array.isArray(value) || !value.length) {
    return null;
  }

  for (let i = 0; i < value.length; i++) {
    if (typeof value[i] !== 'number' || !isFinite(value[i])) {
      return null;
    }
  }
  return value as number[];
}

/**
 * Reads the cache back, falling back to empty for anything that does not look right.
 *
 * Nothing here is trusted. The blob may be missing, half-written, or left over from an older
 * shape, and a bad strip would go straight to the watch, so each field is checked on its own and
 * a bad one costs only itself.
 *
 * @param storage Where to read from, normally localStorage.
 * @return The saved cache, or empty values when there is nothing usable.
 */
function load(storage: CacheStorage): StockCache {
  try {
    const saved = JSON.parse(storage.getItem(CACHE_KEY) as string);
    if (!saved || typeof saved !== 'object') {
      return { lastAsOf: '', lastFetchMs: 0, strip: null };
    }

    return {
      lastAsOf: typeof saved.lastAsOf === 'string' ? saved.lastAsOf : '',
      lastFetchMs: typeof saved.lastFetchMs === 'number' && isFinite(saved.lastFetchMs) ? saved.lastFetchMs : 0,
      strip: readStrip(saved.strip),
    };
  } catch (error) {
    return { lastAsOf: '', lastFetchMs: 0, strip: null };
  }
}

/**
 * Writes the cache, and shrugs if the phone will not take it.
 *
 * Storage throws when it is full or turned off, and a cache that cannot be written is only a
 * missed saving on the next fetch, so it must never take the app down with it.
 *
 * @param storage Where to write to, normally localStorage.
 * @param cache The stamps and strip to keep.
 */
function save(storage: CacheStorage, cache: StockCache): void {
  try {
    storage.setItem(CACHE_KEY, JSON.stringify(cache));
  } catch (error) {
    // nothing to do about it and nothing that needs saying
  }
}

export default { load, save, CACHE_KEY };
