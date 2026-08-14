/**
 * Types shared by every Clay builder, whatever shape the face it edits happens to be.
 *
 * Nothing here knows about grids, cells or panels. A face or family that needs that vocabulary
 * extends these in its own types file.
 *
 * All type-only, so esbuild strips every import of them and nothing here reaches the bundled
 * component.
 */

/** Label to { size key to data url } screenshot map, as module-thumbnails.g.js exports it. */
export type Thumbs = Record<string, Record<string, string>>;

/**
 * The component instance initialize runs as, with enough of Clay's shape for what a builder
 * touches. Not the component itself: that is the object the generated .g.js exports.
 */
export interface ClayComponentInstance {
  $element: HTMLElement[];
  config?: { moduleOptions?: RawOption[]; moduleThumbnails?: Thumbs };
  trigger(event: string): void;
}

/**
 * One row of the option list Clay hands a builder on its config, in the form every builder needs.
 * A face carrying more per-option data extends this rather than redeclaring it.
 */
export interface RawOption {
  value: number;
  label: string;
  icon?: string;
  color?: string;
}

// ModuleMeta is not here. a face's clay/module-meta.ts is runtime code that
// config.ts ships so a type import from this folder would drag it into the
// compiled bundle. it lives in lib/ts/clay/types.ts with the other shapes the
// shipped page uses
