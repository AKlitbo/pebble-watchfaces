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
import { mount } from '../../../../../../../lib/ts/clay/builder/ts/testing/harness';
import { moduleOptionsFixture } from '../../../../../core/pkjs/clay/builder/ts/testing/fixtures';

/** Mounts the layout builder with the shared module fixture and no thumbnails. */
function mountLayout() {
  const mounted = mount(component, { moduleOptions: moduleOptionsFixture, moduleThumbnails: {} });
  mounted.ctx.initialize();
  return mounted;
}

/**
 * The two hidden stores the config page renders beside the builder.
 *
 * The library keeps the four layouts; the night one carries whichever is assigned. They are
 * separate inputs because a single .gl-store lookup would keep finding the first of the two.
 */
function mountStores(): { library: HTMLInputElement; night: HTMLInputElement } {
  function input(extra: string): HTMLInputElement {
    const element = document.createElement('input');
    element.type = 'hidden';
    element.className = 'gl-store ' + extra;
    document.body.appendChild(element);
    return element;
  }
  return { library: input('gl-library'), night: input('gl-night') };
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
  /**
   * A broken Clear would leave stale blocks the user cannot get rid of.
   *
   * It has to come out as the sentinel rather than an empty string: the watch discards an empty
   * cstring instead of storing it, so '' would leave the old layout in place forever.
   */
  test('Clear empties the layout to the sentinel', () => {
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1;3,0,2,2,1');

    root.querySelector<HTMLElement>('.lb-btn-clear').click();
    const result = ctx.get();

    expect(result).toBe('0');
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

    expect(result).toBe('0');
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

    expect(result).toBe('0');
    expect(document.querySelector<HTMLElement>('.lb-ghost')).toBeNull();
  });
});

describe('the layout library', () => {
  /**
   * The rule the whole feature rests on.
   *
   * The grid shows one of four layouts, but LAYOUT must always carry the one assigned to day.
   * Publishing whatever happens to be on screen would ship the wrong layout to the watch the
   * moment somebody tabbed away to edit their night grid.
   */
  test('LAYOUT stays the day layout while another tab is being edited', () => {
    const stores = mountStores();
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    // move to layout 2 and build something different there
    root.querySelectorAll<HTMLElement>('.lb-ltab')[1].click();
    root.querySelector<HTMLElement>('.lb-btn-clear').click();

    expect(ctx.get()).toBe('2,0,0,2,1');
    expect(JSON.parse(stores.library.value).layouts[1]).toBe('0');
  });

  /** Assigning a layout to night has to publish it, or the watch never sees one. */
  test('assigning night publishes that layout to the night store', () => {
    const stores = mountStores();
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    // build something on layout 3, then come back so the day grid is the one on screen
    root.querySelectorAll<HTMLElement>('.lb-ltab')[2].click();
    root.querySelector<HTMLElement>('.lb-btn-clear').click();
    root.querySelectorAll<HTMLElement>('.lb-ltab')[0].click();
    const night = root.querySelectorAll<HTMLSelectElement>('.lb-assign-sel')[1];
    night.value = '2';
    night.dispatchEvent(new Event('change'));

    expect(stores.night.value).toBe('0');
    expect(JSON.parse(stores.library.value).night).toBe(2);
  });

  /** And clearing the assignment has to turn it off rather than leave a stale layout behind. */
  test('setting night back to none empties the night store', () => {
    const stores = mountStores();
    const { root } = mountLayout();

    const night = root.querySelectorAll<HTMLSelectElement>('.lb-assign-sel')[1];
    night.value = '1';
    night.dispatchEvent(new Event('change'));
    night.value = '-1';
    night.dispatchEvent(new Event('change'));

    expect(stores.night.value).toBe('0');
    expect(JSON.parse(stores.library.value).night).toBe(-1);
  });

  /** Without a store the page must still work, since Clay may render it after the builder. */
  test('builds with no stores on the page at all', () => {
    const { ctx } = mountLayout();

    ctx.set('2,0,0,2,1');

    expect(ctx.get()).toBe('2,0,0,2,1');
  });

  /**
   * The ordering trap, and the most expensive bug the feature can have.
   *
   * Clay builds page items one at a time and only attaches each after setting it, so the builder
   * can run before the stores beside it exist. Reading nothing is survivable. Writing that nothing
   * back is not: it replaces four saved layouts with blanks, silently, on the first tab switch.
   */
  test('does not blank a saved library it was built too early to read', async () => {
    const saved = JSON.stringify({ layouts: ['2,0,0,2,1', '3,0,0,2,1', '6,0,0,2,1', '1,0,0,4,1'], day: 0, night: -1 });
    const { root } = mountLayout(); // no stores yet, exactly as Clay would have it
    const stores = mountStores();
    stores.library.value = saved;

    root.querySelectorAll<HTMLElement>('.lb-ltab')[1].click();

    expect(JSON.parse(stores.library.value).layouts).toEqual(JSON.parse(saved).layouts);
  });

  /** And once the store does turn up, the layouts in it are the ones being edited. */
  test('picks up a library that arrives after it was built', async () => {
    const saved = JSON.stringify({ layouts: ['2,0,0,2,1', '3,0,0,2,1', '6,0,0,2,1', '1,0,0,4,1'], day: 2, night: 3 });
    const { root } = mountLayout();
    const stores = mountStores();
    stores.library.value = saved;

    await new Promise((resolve) => { setTimeout(resolve, 0); }); // let the deferred re-read run
    root.querySelectorAll<HTMLElement>('.lb-ltab')[3].click();

    expect(JSON.parse(stores.library.value).layouts[2]).toBe('6,0,0,2,1');
    expect(JSON.parse(stores.library.value).day).toBe(2);
  });
});
