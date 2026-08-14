/**
 * Shared Clay types for the pkjs runtime. Kept in lib (not imported from
 * src/pkjs) so lib stays independent as the copy-as-starter base.
 */

/** A value Clay stores for a setting: a scalar, or the { value } wrapper Clay
 * puts around some component values. */
export type ClayValue = string | number | boolean | { value: string | number | boolean };

/**
 * One Clay config item (a section, heading, select, toggle, input, custom
 * component, etc.). The fields both the config builder and the runtime touch are
 * declared. Custom-component props ride along as optional. No index signature, so
 * a face's own typed config array stays assignable to this.
 */
export interface ClayConfigItem {
  type: string;
  messageKey?: string;
  label?: string;
  description?: string;
  defaultValue?: string | number | boolean;
  options?: Array<{ label: string; value: string | number }>;
  items?: ClayConfigItem[];
  attributes?: Record<string, unknown>;
  /** Clay's own per-item filter, resolved against the watch the page was opened from. Names come
   * from Clay's capability map (PLATFORM_GABBRO, BW, HEALTH and so on) and take a NOT_ prefix to
   * invert, so ['NOT_PLATFORM_GABBRO'] drops the item on a round watch and keeps it everywhere
   * else. An item with none declared is always shown. */
  capabilities?: string[];
  // custom components (layoutBuilder / themeBuilder / slotBuilder) carry these
  moduleOptions?: unknown;
  moduleThumbnails?: unknown;
  /** hiddenStore only: the extra class that tells one store on a page from another. */
  storeClass?: string;
}

/**
 * Per-option presentation metadata, keyed by label in a face's clay/module-meta.ts.
 *
 * embed-thumbnails.ts reads the slug from here, so the thumbnail list has one home rather than a
 * second copy alongside the option list.
 *
 * Here rather than with the builder types because a module-meta.ts is runtime code that config.ts
 * ships. A type import reaching into clay/builder/ would pull that folder into the compiled
 * bundle, which is the one thing those pieces must never do.
 */
export interface ModuleMeta {
  icon: string;
  blockColor: string;
  /** resources/thumbnails/<slug>-<size>.png filename stem. */
  slug: string;
}
