/**
 * Spec helper for reading a built Clay config page.
 *
 * Every face in the family checks its page against its own appinfo and its panels against the
 * palette groups, so the walks that find the keys and the panels live here rather than being
 * written out once per face and drifting.
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

/** One module row as the layout builder item carries it. */
export interface PageModule {
  label: string;
  value: number;
  sizes?: string[];
  group?: string;
}

/**
 * The module list handed to the page's layout builder, however deeply the item is nested.
 *
 * @param items The config array, or any part of it.
 * @return The layout builder's moduleOptions, or an empty list when the page has no layout builder.
 */
export function layoutModules(items: unknown): PageModule[] {
  let found: PageModule[] = [];

  function walk(node: unknown): void {
    if (Array.isArray(node)) {
      node.forEach(walk);
      return;
    }

    if (!node || typeof node !== 'object') {
      return;
    }

    const item = node as { type?: unknown; moduleOptions?: unknown; items?: unknown };

    if (item.type === 'layoutBuilder' && Array.isArray(item.moduleOptions)) {
      found = item.moduleOptions as PageModule[];
    }

    if (item.items) {
      walk(item.items);
    }
  }

  walk(items);
  return found;
}

export default { pageKeys, layoutModules };
