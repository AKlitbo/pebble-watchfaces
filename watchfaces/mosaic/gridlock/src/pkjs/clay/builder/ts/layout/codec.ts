/**
 * The LAYOUT wire string the watch reads: one "module,row,col,w,h" entry per
 * placed block, joined with semicolons. It is a free grid so each block keeps
 * its own position and nothing has to collapse into a fixed row shape.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { GRID_ROWS } from './geometry';
import type { Block } from '../../../../../../../core/pkjs/clay/builder/ts/types';

/**
 * What an empty grid sends instead of an empty string.
 *
 * The watch discards an empty cstring rather than storing it (see
 * settings_apply_inbox), so "" cannot say "I cleared this" — the old layout
 * would just stay. A single character that parses to no blocks can.
 */
export const EMPTY_LAYOUT = '0';

/**
 * Dumps the placed blocks to the wire string, sorted by row then column so
 * the same layout always writes the same text. An empty grid writes the
 * sentinel, which is the only form of "nothing" that survives the trip.
 */
export function serializeLayout(blocks: Block[]): string {
  const sorted = blocks.slice().sort(function (a, b) {
    return a.row - b.row || a.col - b.col;
  });

  const out = [];
  for (let i = 0; i < sorted.length; i++) {
    const block = sorted[i];
    out.push(block.module + ',' + block.row + ',' + block.col + ',' + block.w + ',' + block.h);
  }

  return out.length ? out.join(';') : EMPTY_LAYOUT;
}

/**
 * Reads a wire string back into a fresh blocks array. Junk segments and
 * module 0 are skipped, sizes snap to the placeable ones, and rows and
 * columns clamp back onto the grid so an edited import cannot draw a block
 * off the watch screen.
 */
export function parseLayoutString(str: string): Block[] {
  const blocks: Block[] = [];
  const segs = (str || '').split(';');

  for (let i = 0; i < segs.length; i++) {
    const seg = segs[i];
    if (!seg) {
      continue;
    }
    const parts = seg.split(',');
    if (parts.length < 5) {
      continue;
    }

    const module = parseInt(parts[0], 10) || 0;
    if (!module) {
      continue; // module 0 or junk is not a real block
    }

    const w = parseInt(parts[3], 10) >= 4 ? 4 : 2;
    const h = parseInt(parts[4], 10) >= 2 ? 2 : 1;
    // a 4-wide block spans the whole row so it can only start at column 0
    // (same rule placement enforces). otherwise clamp to the two valid start columns
    const col = w === 4 ? 0 : (parseInt(parts[2], 10) >= 2 ? 2 : 0);
    let row = parseInt(parts[1], 10) || 0;
    if (row < 0) {
      row = 0;
    }
    if (row + h > GRID_ROWS) {
      row = GRID_ROWS - h;
    }

    blocks.push({ module: module, row: row, col: col, w: w, h: h });
  }

  return blocks;
}
