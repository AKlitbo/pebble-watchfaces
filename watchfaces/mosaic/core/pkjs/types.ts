/**
 * Shared domain types for the pkjs layer: the layout wire-string block and the per-module
 * presentation metadata.
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

/**
 * Per-module presentation metadata, keyed by module label. Carries no grid vocabulary, so it is
 * lib's and re-exported here for the two module-meta.ts files that read it by this path.
 */
export type { ModuleMeta } from '../../lib/ts/clay/types';
