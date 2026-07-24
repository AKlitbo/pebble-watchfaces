/**
 * Specs for the weather-condition vocabulary.
 *
 * conditions.ts is the single source of truth that lib/tools/build-conditions.ts
 * turns into the C icon and label tables, and that the pkjs producer emits tokens
 * from. It is data, but data with invariants: a duplicate or malformed row ships a
 * dead generated branch or a RESOURCE_ID_ICON_undefined. So the structural contract
 * is tested, not the individual values (which would just restate the table).
 */

import { describe, test, expect } from 'vitest';
import conditions from './conditions';

describe('condition vocabulary', () => {
  /** A duplicate token generates a dead second branch, so that condition silently shows the first row's icon and label. */
  test('has no duplicate tokens', () => {
    const tokens = conditions.conditions.map((row) => row.token);

    const result = tokens.filter((token, index) => tokens.indexOf(token) !== index);

    expect(result).toEqual([]);
  });

  /** A row missing its token, icon, or a label generates RESOURCE_ID_ICON_undefined or shows the user an empty word. */
  test.each(['token', 'resource', 'labelShort', 'labelLong'])('has a non-empty %s on every condition', (field) => {
    const result = conditions.conditions.filter((row) => !row[field]);

    expect(result).toEqual([]);
  });

  /** resource names are interpolated into C RESOURCE_ID_ICON_<name> identifiers, so a lowercased or spaced value breaks the generated build. */
  test('uses valid resource-id suffixes for day and night icons', () => {
    const resources = conditions.conditions
      .flatMap((row) => [row.resource, row.nightResource])
      .filter(Boolean);

    const result = resources.filter((name) => !/^[A-Z0-9_]+$/.test(name));

    expect(result).toEqual([]);
  });

  /** A fallback missing its icon or labels renders nothing at all for any unrecognized condition. */
  test('gives the fallback an icon and both labels', () => {
    const fallback = conditions.fallback;

    const result = ['resource', 'labelShort', 'labelLong'].filter((field) => !fallback[field]);

    expect(result).toEqual([]);
  });
});
