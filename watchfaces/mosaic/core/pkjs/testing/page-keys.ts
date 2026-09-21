/**
 * Spec helper for reading a built Clay config page.
 *
 * Every face in the family checks its page against its own appinfo, so the walk that finds the
 * keys lives here rather than being written out once per face and drifting.
 *
 * Only specs import this, so it never reaches the watch.
 */

/**
 * Every messageKey the page carries, however deeply the item is nested in sections.
 *
 * @param items The config array, or any part of it.
 * @return The keys in the order they appear, including any repeats.
 */
export function pageKeys(items: unknown): string[] {
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

export default { pageKeys };
