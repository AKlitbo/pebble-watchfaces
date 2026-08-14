/**
 * This face's answers for the shared drag engine: what a panel drag carries, where it may land,
 * and what happens when it gets there.
 *
 * The pointer handling itself lives in lib. All that is here is the four-panel shape: hit testing
 * four boxes rather than mapping a pointer onto a grid pitch, and swapping two panels rather than
 * placing a block into free space.
 */

import { SLOT_COUNT, ID_EMPTY, canPlace, isTall, swallowedBy } from './geometry';
import { fillVisual, readoutById } from './visuals';
import type { Readout, Thumbs } from './visuals';
import { createDrag } from '../../../../../../../../lib/ts/clay/builder/ts/drag';

/** What the drag needs from the component around it. */
export interface DragEnv {
  root: HTMLElement;
  readouts: Readout[];
  thumbs: Thumbs;
  /** The four panel boxes, in slot order. A hidden one reports an empty rect and is skipped. */
  slotEls: HTMLElement[];
  getSlots: () => number[];
  setSlots: (slots: number[]) => void;
}

/** A readout in flight, and the panel it was lifted from (-1 straight off the palette). */
interface Dragged {
  id: number;
  from: number;
}

/**
 * Work out what the four slots would become, or null when the move is not allowed.
 *
 * Dragging between panels swaps them, which can push a tall pick somewhere it cannot go, so the
 * whole candidate arrangement is checked rather than just the panel under the pointer.
 *
 * @param slots The current four.
 * @param id What is being dragged.
 * @param from The slot it came from, or -1 from the palette.
 * @param to The slot it is being dropped on.
 * @return The new four, or null to refuse the drop.
 */
export function tryPlace(slots: number[], id: number, from: number, to: number): number[] | null {
  const next = slots.slice();

  if (from >= 0) {
    next[from] = slots[to];
  }

  next[to] = id;

  // a tall pick fills its column so whatever was under it is gone rather than pushed aside
  const swallowed = swallowedBy(id, to);
  if (swallowed >= 0) {
    next[swallowed] = ID_EMPTY;
  }

  for (let slot = 0; slot < SLOT_COUNT; slot++) {
    if (!canPlace(next[slot], slot)) {
      return null;
    }
  }

  return next;
}

/**
 * Wire up dragging.
 *
 * @param env The component's handles.
 * @return Starters for the two places a drag can begin.
 */
export function installDrag(env: DragEnv) {
  const doc = env.root.ownerDocument || document;

  /** Which panel is under a point, or null. A hidden panel has no box and cannot win. */
  function slotAt(x: number, y: number): number | null {
    for (let slot = 0; slot < SLOT_COUNT; slot++) {
      const box = env.slotEls[slot].getBoundingClientRect();
      if (box.width && x >= box.left && x <= box.right && y >= box.top && y <= box.bottom) {
        return slot;
      }
    }

    return null;
  }

  const drag = createDrag<Dragged, number>({
    anchor: 'center',

    ghost(dragged) {
      const ghost = doc.createElement('div');
      ghost.className = 'sb-ghost' + (isTall(dragged.id) ? ' tall' : '');
      fillVisual(ghost, readoutById(env.readouts, dragged.id), env.thumbs, false);
      return ghost;
    },

    hitTest: slotAt,

    allows(dragged, slot) {
      return tryPlace(env.getSlots(), dragged.id, dragged.from, slot) !== null;
    },

    highlight(dragged, slot, allowed) {
      for (let i = 0; i < SLOT_COUNT; i++) {
        env.slotEls[i].classList.remove('over', 'blocked');
      }

      if (slot !== null) {
        env.slotEls[slot].classList.add(allowed ? 'over' : 'blocked');
      }
    },

    drop(dragged, slot) {
      const next = tryPlace(env.getSlots(), dragged.id, dragged.from, slot);
      if (next) {
        env.setSlots(next);
      }
    },

    dropOutside(dragged) {
      // a panel dragged away is emptied. a palette drag that lands nowhere is just abandoned
      if (dragged.from < 0) {
        return;
      }

      const cleared = env.getSlots().slice();
      cleared[dragged.from] = ID_EMPTY;
      env.setSlots(cleared);
    },
  }, doc);

  return {
    fromPalette(id: number, event: PointerEvent) {
      if (readoutById(env.readouts, id)) {
        drag.start({ id: id, from: -1 }, event);
      }
    },

    // armed rather than started, so a tap on a panel is not mistaken for picking it up
    fromSlot(slot: number, event: PointerEvent) {
      const id = env.getSlots()[slot];
      if (id !== ID_EMPTY) {
        drag.arm({ id: id, from: slot }, event);
      }
    },
  };
}
