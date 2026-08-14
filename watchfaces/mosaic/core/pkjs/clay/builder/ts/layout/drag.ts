/**
 * The family's answers for the shared drag engine: what a block drag carries, where it may land,
 * and what happens when it gets there.
 *
 * The pointer handling itself lives in lib. What is here is the grid: turning a pointer position
 * into a row and column at the cell pitch, snapping a footprint to a legal column, and checking
 * it against what is already placed. Every mosaic face draws the same size cells, so the pitch is
 * shared, but what may sit where is each face's own and arrives through the env.
 *
 * A block is spliced out of the model the moment its drag begins, so the space it leaves reads as
 * free for the rest of the move. That is why a drag ending anywhere illegal removes it: there is
 * nothing to put back, which is also how a block is deleted.
 *
 * A Clay builder piece. esbuild bundles it into the component's initialize, which runs in the
 * config webview, so it sticks to browser APIs.
 */

import { fillBlockVisual } from './visuals';
import { createDrag } from '../../../../../../../../lib/ts/clay/builder/ts/drag';
import type { Block, ModuleInfo, SizeKey } from '../types';

/** The module riding under the pointer mid drag, palette icon or lifted block. */
export interface DraggedModule {
  value: number;
  icon: string;
  color: string;
  label: string;
  w: number;
  h: number;
  /** Where it was lifted from, or null when it came straight off the palette. */
  blockIdx: number | null;
}

/** A cell the pointer is over, before snapping. */
interface Cell {
  r: number;
  c: number;
}

/**
 * The grid rules, which belong to the face rather than the family.
 *
 * gridlock is an open grid, sidereel keeps a column for its reel and a row for the hour pointer.
 * Same cells, different answers about what may sit in them, so the rules are handed in.
 */
export interface DragGeometry {
  rows: number;
  cols: number;
  sizeKey(w: number, h: number): SizeKey | null;
  canPlace(blocks: Block[], row: number, col: number, w: number, h: number, ignoreIdx?: number): boolean;
  snapDrop(target: { r: number; c: number }, w: number, h: number): { row: number; col: number };
}

/** What the engine cannot own itself, handed in by the builder that hosts it. */
export interface DragEnv {
  gridEl: HTMLElement;
  geometry: DragGeometry;
  getBlocks(): Block[];
  render(): void;
  modInfo(value: number): ModuleInfo;
  thumbFor(value: number, key: string): string | null;
}

/** The two entry points the rest of the builder drives the drag through. */
export interface DragEngine {
  startDragFromPalette(m: ModuleInfo, sizeGroupKey: string, e: PointerEvent): void;
  armBlockDrag(idx: number, e: PointerEvent): void;
}

/**
 * Wires the drag onto the document and hands back the two entry points.
 *
 * The env carries what the engine cannot own itself: the grid element, a getBlocks() handle to
 * the live blocks array (the array reference changes on import, so it is always fetched fresh),
 * render() to repaint after a change, and modInfo/thumbFor already bound to the module list and
 * thumbnails.
 */
export function createDragEngine(env: DragEnv): DragEngine {
  const rules = env.geometry;

  /** The cell under a point, or null when the pointer has left the grid. */
  function cellAt(x: number, y: number): Cell | null {
    const rect = env.gridEl.getBoundingClientRect();
    const localX = x - rect.left;
    const localY = y - rect.top;

    if (localX < -10 || localX > rect.width + 10 || localY < -10 || localY > rect.height + 10) {
      return null;
    }

    // the grid has 5px inner padding so shift the pointer in before mapping to a cell
    const c = Math.max(0, Math.min(rules.cols - 1, Math.floor((localX - 5) / 55))); // col pitch 52 + 3 gap
    const r = Math.max(0, Math.min(rules.rows - 1, Math.floor((localY - 5) / 48))); // row pitch 45 + 3 gap

    return { r: r, c: c };
  }

  function clearHighlight(): void {
    const cells = env.gridEl.querySelectorAll('.lb-cell');
    for (let i = 0; i < cells.length; i++) {
      cells[i].classList.remove('selecting');
    }
  }

  const drag = createDrag<DraggedModule, Cell>({
    anchor: 'pointer',

    ghost(dragged) {
      const ghost = document.createElement('div');
      ghost.className = 'lb-ghost lb-block';
      if (dragged.h >= 2) {
        ghost.classList.add('big');
      }

      fillBlockVisual(
        ghost,
        env.thumbFor(dragged.value, rules.sizeKey(dragged.w, dragged.h) || ''),
        dragged as unknown as ModuleInfo,
        dragged.w
      );

      ghost.style.width = dragged.w * 52 + (dragged.w - 1) * 3 + 'px';
      ghost.style.height = dragged.h * 45 + (dragged.h - 1) * 3 + 'px';
      return ghost;
    },

    hitTest: cellAt,

    allows(dragged, cell) {
      const snap = rules.snapDrop(cell, dragged.w, dragged.h);
      return rules.canPlace(env.getBlocks(), snap.row, snap.col, dragged.w, dragged.h);
    },

    highlight(dragged, cell, allowed) {
      clearHighlight();
      if (!dragged || cell === null || !allowed) {
        return;
      }

      // light the whole footprint the block would cover, not just the cell under the pointer
      const snap = rules.snapDrop(cell, dragged.w, dragged.h);
      const cells = env.gridEl.querySelectorAll<HTMLElement>('.lb-cell');
      for (let i = 0; i < cells.length; i++) {
        const cr = parseInt(cells[i].dataset.row as string, 10);
        const cc = parseInt(cells[i].dataset.col as string, 10);
        if (cr >= snap.row && cr < snap.row + dragged.h && cc >= snap.col && cc < snap.col + dragged.w) {
          cells[i].classList.add('selecting');
        }
      }
    },

    drop(dragged, cell) {
      const snap = rules.snapDrop(cell, dragged.w, dragged.h);
      env.getBlocks().push({
        module: dragged.value,
        row: snap.row,
        col: snap.col,
        w: dragged.w,
        h: dragged.h,
      });
      env.render();
    },

    dropOutside() {
      // a lifted block was spliced out when the drag began, so letting go anywhere illegal is
      // what removes it. a palette drag that lands nowhere simply never placed anything
      env.render();
    },

    lift(dragged) {
      if (dragged.blockIdx !== null) {
        env.getBlocks().splice(dragged.blockIdx, 1);
        env.render();
      }
    },
  });

  return {
    startDragFromPalette(m: ModuleInfo, sizeGroupKey: string, e: PointerEvent): void {
      drag.start({
        value: m.value,
        icon: m.icon,
        color: m.color,
        label: m.label,
        w: parseInt(sizeGroupKey[2], 10),
        h: parseInt(sizeGroupKey[0], 10),
        blockIdx: null,
      }, e);
    },

    // armed rather than started: a press on a placed block only becomes a drag once the pointer
    // has moved far enough that it is clearly not a tap
    armBlockDrag(idx: number, e: PointerEvent): void {
      const block = env.getBlocks()[idx];
      const mod = env.modInfo(block.module);

      drag.arm({
        value: block.module,
        icon: mod.icon,
        color: mod.color,
        label: mod.label,
        w: block.w,
        h: block.h,
        blockIdx: idx,
      }, e);
    },
  };
}
