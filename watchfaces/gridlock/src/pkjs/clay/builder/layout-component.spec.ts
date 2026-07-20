// @vitest-environment jsdom
/**
 * Specs for the layoutBuilder Clay component, driven end to end.
 *
 * The pieces carry their own unit specs, but the user flows only exist once
 * everything is wired together inside the assembled component: a LAYOUT wire
 * string in, the grid and palette rendered, the preset buttons,
 * import/export, and the pointer driven drag and drop. These specs mount the
 * generated layout-component.g.js the way Clay does and drive it the way a
 * user would.
 *
 * jsdom reports every element rect as zeros, so drop targets always resolve
 * to the top left cell. Placement is exercised there and the geometry rules
 * get their own unit specs.
 */

import { describe, test, expect, beforeEach } from 'vitest';
import component from '../layout-component.g.js';
import { mount, moduleOptionsFixture } from './ts/testing/harness';

/** Mounts the layout builder with the shared module fixture and no thumbnails. */
function mountLayout() {
  const mounted = mount(component, { moduleOptions: moduleOptionsFixture, moduleThumbnails: {} });
  mounted.ctx.initialize();
  return mounted;
}

/** A pointer style event jsdom can dispatch (MouseEvent carries the client coords). */
function pointer(type: string, clientX: number, clientY: number): MouseEvent {
  return new MouseEvent(type, { clientX: clientX, clientY: clientY, bubbles: true });
}

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('LAYOUT wire string round trip', () => {
  /** A drifted serializer would hand the watch a layout it cannot parse back. */
  test('set then get returns the blocks sorted by row then column', () => {
    const { ctx } = mountLayout();

    ctx.set('1,2,0,4,1;2,0,0,2,2');
    const result = ctx.get();

    expect(result).toBe('2,0,0,2,2;1,2,0,4,1');
  });

  /** Out of range rows or columns from an edited import would draw blocks off the watch screen. */
  test('clamps a block outside the grid back inside', () => {
    const { ctx } = mountLayout();

    ctx.set('2,9,3,2,1');
    const result = ctx.get();

    expect(result).toBe('2,4,2,2,1');
  });

  /** A full width block starting mid row would overlap its neighbour on the watch. */
  test('forces a 4 wide block to column 0 and clamps its row', () => {
    const { ctx } = mountLayout();

    ctx.set('49,4,2,4,2');
    const result = ctx.get();

    expect(result).toBe('49,3,0,4,2');
  });

  /** A negative row from a hand edited import would crash the render loop. */
  test('clamps a negative row to 0', () => {
    const { ctx } = mountLayout();

    ctx.set('2,-1,0,2,1');
    const result = ctx.get();

    expect(result).toBe('2,0,0,2,1');
  });

  /** Junk pasted into import must not wipe the valid blocks around it. */
  test('skips junk segments and module 0 but keeps the good blocks', () => {
    const { ctx } = mountLayout();

    ctx.set('banana;;0,0,0,2,1;2,0,0,2,1;3,1');
    const result = ctx.get();

    expect(result).toBe('2,0,0,2,1');
  });
});

describe('preset buttons', () => {
  const goldens = {
    default: '2,0,0,2,2;12,0,2,2,1;13,1,2,2,1;1,2,0,4,1;3,3,0,2,2;6,3,2,2,2',
    1: '2,0,0,2,1;18,0,2,2,1;5,1,0,2,2;6,1,2,2,2;9,3,0,2,2;7,3,2,2,2',
    2: '17,0,0,2,1;28,0,2,2,2;18,1,0,2,1;25,2,0,4,1;22,3,0,2,1;21,3,2,2,1;11,4,0,2,1;23,4,2,2,1',
    3: '3,0,0,2,2;12,0,2,2,1;10,1,2,2,1;1,2,0,4,1;6,3,0,2,1;11,3,2,2,1;5,4,0,2,1;13,4,2,2,1',
    4: '2,0,0,2,1;5,0,2,2,1;1,1,0,4,1;9,2,0,2,1;6,2,2,2,1;8,3,0,2,2;28,3,2,2,2',
    5: '2,0,0,2,1;17,0,2,2,1;18,1,0,2,1;19,1,2,2,1;3,2,0,2,2;28,2,2,2,2;6,4,0,2,1;5,4,2,2,1',
  };

  Object.keys(goldens).forEach((presetId) => {
    /** A drifted preset would hand new users a broken starter layout with one tap. */
    test(`the "${presetId}" preset serializes to its golden string`, () => {
      const { ctx, root } = mountLayout();

      root.querySelector<HTMLElement>(`.lb-preset[data-preset="${presetId}"]`).click();
      const result = ctx.get();

      expect(result).toBe(goldens[presetId]);
    });
  });
});

describe('actions bar', () => {
  /** A broken Clear would leave stale blocks the user cannot get rid of. */
  test('Clear empties the layout', () => {
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1;3,0,2,2,1');

    root.querySelector<HTMLElement>('.lb-btn-clear').click();
    const result = ctx.get();

    expect(result).toBe('');
  });

  /** A dead import path would strand users who share layouts as text. */
  test('Import/Export applies a pasted layout string and closes the panel', () => {
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    root.querySelector<HTMLElement>('.lb-btn-io').click();
    const textarea = document.querySelector<HTMLTextAreaElement>('.lb-io-textarea');
    expect(textarea.value).toBe('2,0,0,2,1');
    textarea.value = '3,1,2,2,1';
    document.querySelector<HTMLElement>('.lb-io-btn.primary').click();
    const result = ctx.get();

    expect(result).toBe('3,1,2,2,1');
    expect(document.querySelector<HTMLElement>('.lb-panel')).toBeNull();
  });
});

describe('drag and drop', () => {
  /** If a palette drop stops landing, no module can ever be placed. */
  test('dragging a palette module onto the grid places it', () => {
    const { ctx, root } = mountLayout();

    const batteryItem = root.querySelector<HTMLElement>('.lb-pal-item[data-value="2"]');
    batteryItem.dispatchEvent(pointer('pointerdown', 5, 5));
    document.dispatchEvent(pointer('pointermove', 6, 6));
    document.dispatchEvent(pointer('pointerup', 5, 5));
    const result = ctx.get();

    expect(result).toBe('2,0,0,2,1');
    expect(batteryItem.classList.contains('placed')).toBe(true);
    expect(document.querySelector<HTMLElement>('.lb-ghost')).toBeNull();
  });

  /** If dragging off grid stops removing, the only way to clear one block is Clear all. */
  test('dragging a placed block off the grid removes it', () => {
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    const block = root.querySelector<HTMLElement>('.lb-block');
    block.dispatchEvent(pointer('pointerdown', 5, 5));
    document.dispatchEvent(pointer('pointermove', 100, 100));
    document.dispatchEvent(pointer('pointerup', 500, 500));
    const result = ctx.get();

    expect(result).toBe('');
    expect(document.querySelector<HTMLElement>('.lb-ghost')).toBeNull();
  });

  /** A tap that never moved must not turn a later pointer move into a phantom drag. */
  test('a tap on a block without dragging leaves the layout alone', () => {
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    const block = root.querySelector<HTMLElement>('.lb-block');
    block.dispatchEvent(pointer('pointerdown', 5, 5));
    document.dispatchEvent(pointer('pointerup', 5, 5));
    document.dispatchEvent(pointer('pointermove', 100, 100));
    const result = ctx.get();

    expect(result).toBe('2,0,0,2,1');
    expect(document.querySelector<HTMLElement>('.lb-ghost')).toBeNull();
  });

  /** A cancelled drag that leaks its ghost leaves a block floating over the page forever. */
  test('pointercancel cleans up the ghost and places nothing', () => {
    const { ctx, root } = mountLayout();

    root.querySelector<HTMLElement>('.lb-pal-item[data-value="2"]').dispatchEvent(pointer('pointerdown', 5, 5));
    document.dispatchEvent(pointer('pointercancel', 5, 5));
    document.dispatchEvent(pointer('pointerup', 5, 5));
    const result = ctx.get();

    expect(result).toBe('');
    expect(document.querySelector<HTMLElement>('.lb-ghost')).toBeNull();
  });
});
