/**
 * The LAYOUT wire string the watch reads: one "module,row,col,w,h" entry per
 * placed block, joined with semicolons. It is a free grid so each block keeps
 * its own position and nothing has to collapse into a fixed row shape.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { GRID_ROWS, BLOCK_COL, POINTER_ROW, spansPointer } from './geometry';
import { EMPTY_LAYOUT } from '../../../../../../../core/pkjs/clay/builder/ts/layout/wire';
import type { Block } from '../../../../../../../core/pkjs/clay/builder/ts/types';

// re-exported so everything that speaks this face's wire format still reaches it through the codec
export { EMPTY_LAYOUT };

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

    // this face has one column of one width, so the only thing a size can vary is its height
    const w = 2;
    const h = parseInt(parts[4], 10) >= 2 ? 2 : 1;

    let row = parseInt(parts[1], 10) || 0;
    if (row < 0) {
      row = 0;
    }
    if (row + h > GRID_ROWS) {
      row = GRID_ROWS - h;
    }
    // an imported string can name a block on the pointer, which is a shape the face cannot
    // draw. pull it to the nearer half rather than dropping it
    if (spansPointer(row, h)) {
      row = row < POINTER_ROW ? POINTER_ROW - h : POINTER_ROW + 1;
    }
    if (row < 0) {
      row = 0;
    }
    if (row + h > GRID_ROWS) {
      row = GRID_ROWS - h;
    }

    blocks.push({ module: module, row: row, col: BLOCK_COL, w: w, h: h });
  }

  return blocks;
}
