/**
 * Shared domain types for the pkjs layer: the layout wire-string block, the Clay
 * setting rows the config builders emit, and the per-module presentation metadata.
 *
 * These are pure vocabulary: every consumer imports them with `import type`, which erases,
 * so nothing here reaches the watch beyond the empty module tsc emits for the file itself.
 * They exist to let the compiler check the shapes both sides pass around.
 */

/** One placed block in the layout grid, the shape the codec round-trips. */
export interface Block {
  module: number;
  row: number;
  col: number;
  w: number;
  h: number;
}

/** The four placeable block sizes, keyed the way the wire string names them. */
export type SizeKey = '1x2' | '2x2' | '1x4' | '2x4';

/** A Clay setting row, the shape the config.ts helpers build. */
export interface ClayItem {
  type:
    | 'section'
    | 'heading'
    | 'text'
    | 'select'
    | 'toggle'
    | 'input'
    | 'color'
    | 'button'
    | 'submit'
    | 'locationsearch'
    | 'layoutBuilder'
    | 'themeBuilder'
    | 'hiddenStore';
  /** Names the row for getItemById, for a row a custom function has to reach. */
  id?: string;
  messageKey?: string;
  label?: string;
  defaultValue?: string | number | boolean;
  description?: string;
  options?: Array<{ label: string; value: string | number }>;
  items?: ClayItem[];
  attributes?: Record<string, string | number>;
  /** color only: false shows the true colours instead of their washed-out sunlight pair. */
  sunlight?: boolean;
}

/** Per-module presentation metadata for the Clay builders, keyed by module label. */
export interface ModuleMeta {
  icon: string;
  blockColor: string;
  /** resources/thumbnails/<slug>-<size>.png filename stem, read by generate-thumbnails.ts. */
  slug: string;
}
