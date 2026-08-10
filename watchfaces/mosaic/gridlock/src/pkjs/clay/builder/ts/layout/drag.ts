/**
 * The pointer driven drag and drop for the layout builder. A drag starts from
 * a palette icon or from a placed block that moved far enough, a ghost
 * follows the pointer, cells light up where the block would land, and
 * releasing either places the block or drops it off the grid to remove it.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { GRID_ROWS, GRID_COLS, sizeKey, canPlace, snapDrop } from './geometry';
import { fillBlockVisual } from './visuals';
import type { Block, ModuleInfo } from '../../../../../../../core/pkjs/clay/builder/ts/types';

/** The module riding under the pointer mid drag, palette icon or lifted block. */
export interface DraggedModule {
  value: number;
  icon: string;
  color: string;
  label: string;
  w: number;
  h: number;
}

/** What the engine cannot own itself, handed in by the builder that hosts it. */
export interface DragEnv {
  gridEl: HTMLElement;
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
 * Wires the drag state machine onto the document and hands back the two entry
 * points the rest of the builder needs.
 *
 * The env carries what the engine cannot own itself: the grid element, a
 * getBlocks() handle to the live blocks array (the array reference changes on
 * import, so it is always fetched fresh), render() to repaint after a change,
 * and modInfo/thumbFor already bound to the module list and thumbnails.
 */
export function createDragEngine(env: DragEnv): DragEngine {
  let initialPointer: { x: number; y: number } | null = null;
  let potentialDragBlockIdx: number | null = null;
  let draggedModule: DraggedModule | null = null;
  let draggedBlockIdx: number | null = null;
  let ghostEl: HTMLElement | null = null;

  function startDragFromPalette(m: ModuleInfo, sizeGroupKey: string, e: PointerEvent): void {
    e.preventDefault();

    const w = parseInt(sizeGroupKey[2], 10);
    const h = parseInt(sizeGroupKey[0], 10);
    draggedModule = {
      value: m.value,
      icon: m.icon,
      color: m.color,
      label: m.label,
      w: w,
      h: h,
    };
    draggedBlockIdx = null;
    potentialDragBlockIdx = null;

    createGhost(e);
  }

  // a block press does not drag yet. it arms and the drag only starts once the
  // pointer has moved far enough that this is clearly not a tap
  function armBlockDrag(idx: number, e: PointerEvent): void {
    e.preventDefault();

    initialPointer = { x: e.clientX, y: e.clientY };
    potentialDragBlockIdx = idx;
  }

  function createGhost(e: PointerEvent): void {
    const dragged = draggedModule as DraggedModule;
    ghostEl = document.createElement('div');
    ghostEl.className = 'lb-ghost lb-block';
    if (dragged.h >= 2) {
      ghostEl.classList.add('big');
    }

    fillBlockVisual(
      ghostEl,
      env.thumbFor(dragged.value, sizeKey(dragged.w, dragged.h) || ''),
      dragged as unknown as ModuleInfo,
      dragged.w
    );

    const w = dragged.w;
    const h = dragged.h;
    ghostEl.style.width = w * 52 + (w - 1) * 3 + 'px';
    ghostEl.style.height = h * 45 + (h - 1) * 3 + 'px';

    document.body.appendChild(ghostEl);
    moveGhost(e);
    updateSelectionHighlight(e);
  }

  function moveGhost(e: PointerEvent): void {
    if (!ghostEl) {
      return;
    }

    ghostEl.style.left = e.clientX + 'px';
    ghostEl.style.top = e.clientY + 'px';
  }

  function removeGhost(): void {
    if (ghostEl) {
      document.body.removeChild(ghostEl);
      ghostEl = null;
    }
  }

  function getDropTarget(e: PointerEvent): { r: number; c: number } | null {
    const rect = env.gridEl.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    if (x < -10 || x > rect.width + 10 || y < -10 || y > rect.height + 10) {
      return null;
    }

    // the grid has 5px inner padding so shift the pointer in before mapping to a cell
    const c = Math.max(0, Math.min(GRID_COLS - 1, Math.floor((x - 5) / 55))); // col pitch 52 + 3 gap
    const r = Math.max(0, Math.min(GRID_ROWS - 1, Math.floor((y - 5) / 48))); // row pitch 45 + 3 gap

    return { r: r, c: c };
  }

  function clearHighlight(): void {
    const els = env.gridEl.querySelectorAll('.lb-cell');
    for (let i = 0; i < els.length; i++) {
      els[i].classList.remove('selecting');
    }
  }

  function updateSelectionHighlight(e: PointerEvent): void {
    clearHighlight();
    if (!draggedModule) {
      return;
    }

    const target = getDropTarget(e);
    if (!target) {
      return;
    }

    const w = draggedModule.w;
    const h = draggedModule.h;
    const snap = snapDrop(target, w, h);

    if (canPlace(env.getBlocks(), snap.row, snap.col, w, h)) {
      const els = env.gridEl.querySelectorAll<HTMLElement>('.lb-cell');
      for (let i = 0; i < els.length; i++) {
        const cr = parseInt(els[i].dataset.row as string, 10);
        const cc = parseInt(els[i].dataset.col as string, 10);
        if (cr >= snap.row && cr < snap.row + h && cc >= snap.col && cc < snap.col + w) {
          els[i].classList.add('selecting');
        }
      }
    }
  }

  document.addEventListener('pointermove', function (e) {
    if (potentialDragBlockIdx !== null && initialPointer) {
      const dx = e.clientX - initialPointer.x;
      const dy = e.clientY - initialPointer.y;
      if (Math.abs(dx) > 10 || Math.abs(dy) > 10) {
        const blocks = env.getBlocks();
        const block = blocks[potentialDragBlockIdx];
        const mod = env.modInfo(block.module);
        draggedModule = {
          value: block.module,
          icon: mod.icon,
          color: mod.color,
          label: mod.label,
          w: block.w,
          h: block.h,
        };
        draggedBlockIdx = potentialDragBlockIdx;
        potentialDragBlockIdx = null;

        blocks.splice(draggedBlockIdx, 1);
        env.render();
        createGhost(e);
      }
    }

    if (draggedModule) {
      moveGhost(e);
      updateSelectionHighlight(e);
    }
  });

  document.addEventListener('pointerup', function (e) {
    if (potentialDragBlockIdx !== null) {
      // a tap without a drag does nothing. just clear the pending index so a
      // later pointermove cannot start a phantom drag of this block
      potentialDragBlockIdx = null;
    }

    if (draggedModule) {
      const target = getDropTarget(e);
      let placed = false;

      if (target) {
        const w = draggedModule.w;
        const h = draggedModule.h;
        const snap = snapDrop(target, w, h);

        if (canPlace(env.getBlocks(), snap.row, snap.col, w, h)) {
          env.getBlocks().push({
            module: draggedModule.value,
            row: snap.row,
            col: snap.col,
            w: w,
            h: h,
          });
          placed = true;
          env.render();
        }
      }

      if (!placed && draggedBlockIdx !== null) {
        // dragging a placed module off the grid removes it. it was already
        // spliced out at drag start so just render to be safe
        env.render();
      }

      removeGhost();
      draggedModule = null;
      draggedBlockIdx = null;
      clearHighlight();
    }
  });

  document.addEventListener('pointercancel', function () {
    potentialDragBlockIdx = null;
    removeGhost();
    draggedModule = null;
    draggedBlockIdx = null;
    clearHighlight();
  });

  return {
    startDragFromPalette: startDragFromPalette,
    armBlockDrag: armBlockDrag,
  };
}
