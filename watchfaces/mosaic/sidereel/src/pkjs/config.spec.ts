/**
 * Specs for the date formats the config page offers.
 *
 * This face draws its date with plain strftime, so it has no way to fill in the .beats token the
 * shared date list can carry. A {B} format reaching this page would render on the watch as the
 * literal braces, which reads as a corrupted date rather than an option that does not apply.
 */

import { describe, test, expect } from 'vitest';
import config from './config';

/** The values the date format select offers, wherever on the page it sits. */
function dateFormatValues(items: unknown): string[] {
  const found: string[] = [];

  function walk(node: unknown): void {
    if (Array.isArray(node)) {
      node.forEach(walk);
      return;
    }
    if (!node || typeof node !== 'object') {
      return;
    }
    const item = node as { messageKey?: unknown; options?: unknown; items?: unknown };
    if (item.messageKey === 'CLOCK_DATE_FORMAT' && Array.isArray(item.options)) {
      for (const option of item.options as Array<{ value?: unknown }>) {
        found.push(String(option.value));
      }
    }
    if (item.items) {
      walk(item.items);
    }
  }

  walk(items);
  return found;
}

describe('config page date formats', () => {
  /** This face draws its date with plain strftime, so a {B} token would reach the watch as text. */
  test('offers no date format carrying the .beats token', () => {
    const result = dateFormatValues(config).filter((value) => value.includes('{B}'));

    expect(result).toEqual([]);
  });

  /** The filter above passes just as happily on an empty list, which would prove nothing. */
  test('offers date formats at all', () => {
    const result = dateFormatValues(config);

    expect(result.length).toBeGreaterThan(10);
  });
});
