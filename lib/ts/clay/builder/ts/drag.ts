/**
 * The pointer machinery every Clay builder's drag and drop is built on.
 *
 * What is shared is the handling of the pointer itself: arming a press so a tap is not mistaken
 * for a drag, carrying a ghost under the finger, tracking what is beneath it, and finishing on
 * release or cancel. What is not shared is anything about the thing being dragged or the place it
 * lands, so both are opaque here: the payload is whatever the face wants to move, the target
 * whatever the face wants to drop it on.
 *
 * That split is what lets a resizable grid and a fixed row of panels use the same engine. The
 * grid answers hitTest with a row and column, the panels answer with a slot index, and neither
 * shape reaches this file.
 *
 * A Clay builder piece: esbuild bundles it into the component's initialize, which runs in the
 * config webview, so it sticks to browser APIs.
 */

/** Where a drag began. A palette item has no origin because it is not placed yet. */
export const FROM_PALETTE = null;

/** What a face has to answer for its own drags. */
export interface DragSpec<TPayload, TTarget> {
  /**
   * How far the pointer must travel before an armed press becomes a drag.
   *
   * Without it a tap on something already placed spawns a ghost, which on a touchscreen makes
   * the builder feel like it grabs at every touch. Ten pixels is comfortably past a shaky tap
   * and well under a deliberate drag.
   */
  threshold?: number;

  /** Whether the ghost hangs from its top-left corner or rides centred under the pointer. */
  anchor?: 'pointer' | 'center';

  /** Builds the element that follows the pointer. Sizing it is the face's business. */
  ghost(payload: TPayload): HTMLElement;

  /** What sits under this point, or null for nothing droppable. */
  hitTest(x: number, y: number): TTarget | null;

  /** Whether this payload may land here. Drives the highlight as well as the drop. */
  allows(payload: TPayload, target: TTarget): boolean;

  /**
   * Show the drop feedback, or clear it when target is null.
   *
   * The payload comes along because feedback usually depends on it: a grid lights the whole
   * footprint of the block being carried, not just the cell under the pointer.
   */
  highlight(payload: TPayload | null, target: TTarget | null, allowed: boolean): void;

  /** Commit a drop that allows() approved. */
  drop(payload: TPayload, target: TTarget): void;

  /** Released away from any target. For a placed item this usually means remove it. */
  dropOutside(payload: TPayload): void;

  /**
   * An armed press has passed the threshold and is now a real drag.
   *
   * A builder that takes the item out of its model while it moves does that here, so the space
   * it left reads as free for the rest of the drag.
   */
  lift?(payload: TPayload): void;
}

/** The two ways a face starts a drag. */
export interface DragHandle<TPayload> {
  /** Begin at once. For a palette item, where a press means nothing else. */
  start(payload: TPayload, event: PointerEvent): void;

  /** Arm, and begin only once the pointer has moved far enough to not be a tap. */
  arm(payload: TPayload, event: PointerEvent): void;
}

const DEFAULT_THRESHOLD = 10;

/**
 * Wires the drag onto the document and hands back the two entry points.
 *
 * The listeners are never removed. A pointer that leaves the element still has to finish its
 * drag, and the component lives as long as the config page does.
 *
 * @param spec What the face answers for its own drags.
 * @param doc The document to listen on. Defaults to the ambient one.
 * @return The starters to call from a pointerdown.
 */
export function createDrag<TPayload, TTarget>(
  spec: DragSpec<TPayload, TTarget>,
  doc: Document = document
): DragHandle<TPayload> {
  const threshold = spec.threshold === undefined ? DEFAULT_THRESHOLD : spec.threshold;

  // an armed press that has not travelled far enough yet
  let armed: { payload: TPayload; x: number; y: number } | null = null;
  // a drag actually under way
  let active: { payload: TPayload; ghost: HTMLElement } | null = null;

  function place(ghost: HTMLElement, x: number, y: number): void {
    if (spec.anchor === 'pointer') {
      ghost.style.left = x + 'px';
      ghost.style.top = y + 'px';
      return;
    }

    const box = ghost.getBoundingClientRect();
    ghost.style.left = x - box.width / 2 + 'px';
    ghost.style.top = y - box.height / 2 + 'px';
  }

  function begin(payload: TPayload, x: number, y: number): void {
    const ghost = spec.ghost(payload);
    doc.body.appendChild(ghost);
    active = { payload: payload, ghost: ghost };
    place(ghost, x, y);
    track(x, y);
  }

  /** Move the ghost and light up whatever is under it. */
  function track(x: number, y: number): void {
    if (!active) {
      return;
    }

    place(active.ghost, x, y);

    const target = spec.hitTest(x, y);
    const allowed = target !== null && spec.allows(active.payload, target);
    spec.highlight(active.payload, target, allowed);
  }

  function end(): void {
    if (active) {
      active.ghost.remove();
      active = null;
    }

    armed = null;
    spec.highlight(null, null, false);
  }

  doc.addEventListener('pointermove', (event) => {
    // an armed press only becomes a drag once it has clearly stopped being a tap
    if (armed) {
      const moved = Math.abs(event.clientX - armed.x) > threshold
        || Math.abs(event.clientY - armed.y) > threshold;

      if (moved) {
        const payload = armed.payload;
        armed = null;
        if (spec.lift) {
          spec.lift(payload);
        }
        begin(payload, event.clientX, event.clientY);
      }
    }

    if (active) {
      track(event.clientX, event.clientY);
      event.preventDefault();
    }
  });

  doc.addEventListener('pointerup', (event) => {
    if (!active) {
      // a press that never travelled is a tap, so drop the arming and leave the model alone
      armed = null;
      return;
    }

    const payload = active.payload;
    const target = spec.hitTest(event.clientX, event.clientY);
    end();

    if (target !== null && spec.allows(payload, target)) {
      spec.drop(payload, target);
      return;
    }

    spec.dropOutside(payload);
  });

  doc.addEventListener('pointercancel', () => {
    // a cancelled drag is not a drop. whatever was lifted is the face's to put back
    const payload = active && active.payload;
    end();

    if (payload !== null && payload !== undefined) {
      spec.dropOutside(payload);
    }
  });

  return {
    start(payload: TPayload, event: PointerEvent): void {
      begin(payload, event.clientX, event.clientY);
      event.preventDefault();
    },

    arm(payload: TPayload, event: PointerEvent): void {
      armed = { payload: payload, x: event.clientX, y: event.clientY };
      event.preventDefault();
    },
  };
}
