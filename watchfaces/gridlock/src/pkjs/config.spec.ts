/**
 * Specs for the config page's message keys.
 *
 * A key the page sends but the appinfo never declared is the quietest failure in the whole
 * settings path: Clay maps an unknown key to the literal dict key "undefined0", the watch ignores
 * it, and the setting simply never works. Nothing warns, nothing throws, and it looks like a bug
 * in whatever feature the key belonged to. Comparing the two lists catches it here instead.
 */

import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect } from 'vitest';
import config from './config';

const appinfo = JSON.parse(
  fs.readFileSync(path.resolve(import.meta.dirname, '../../config/pebble.appinfo.json'), 'utf8')
) as { messageKeys: string[] };

/** Every messageKey the page carries, however deeply the item is nested in sections. */
function pageKeys(items: unknown): string[] {
  const found: string[] = [];

  function walk(node: unknown): void {
    if (Array.isArray(node)) {
      node.forEach(walk);
      return;
    }
    if (!node || typeof node !== 'object') {
      return;
    }
    const item = node as { messageKey?: unknown; items?: unknown };
    if (typeof item.messageKey === 'string') {
      found.push(item.messageKey);
    }
    if (item.items) {
      walk(item.items);
    }
  }

  walk(items);
  return found;
}

describe('config page message keys', () => {
  /** The page has to actually carry some, or the walk above is silently finding nothing. */
  test('the page declares message keys', () => {
    const result = pageKeys(config);

    expect(result.length).toBeGreaterThan(20);
  });

  /** Every key the page sends must be one the watch was built to receive. */
  test('every key on the page is declared in the appinfo', () => {
    const declared = new Set(appinfo.messageKeys);

    const result = pageKeys(config).filter((key) => !declared.has(key));

    expect(result).toEqual([]);
  });

  /** The night layout keys in particular, since they span the page, the appinfo and the C schema. */
  test('the night layout keys are wired end to end', () => {
    const onPage = new Set(pageKeys(config));

    for (const key of ['LAYOUT', 'LAYOUT_NIGHT', 'LAYOUT_NIGHT_MODE', 'LAYOUT_NIGHT_START', 'LAYOUT_NIGHT_END']) {
      expect(onPage.has(key), `${key} missing from the config page`).toBe(true);
      expect(appinfo.messageKeys, `${key} missing from the appinfo`).toContain(key);
    }
  });
});
