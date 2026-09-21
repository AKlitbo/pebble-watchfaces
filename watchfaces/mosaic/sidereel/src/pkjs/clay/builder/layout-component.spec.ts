// @vitest-environment jsdom
/**
 * Specs for sidereel's layoutBuilder Clay component, driven end to end.
 *
 * Sidereel keeps its own copy of the builder wiring, since its grid is half a screen wide and the
 * reel owns the rest, so gridlock's specs cover none of it. What is worth pinning here is the part
 * the user cannot see going wrong: which hidden store each assigned layout is written into. A job
 * whose store never gets written looks settled on the config page and does nothing on the watch.
 *
 * jsdom reports every element rect as zeros, so the geometry rules get their own unit specs and
 * these drive the assignment pickers instead.
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
 * The hidden stores the config page renders beside the builder.
 *
 * The library keeps every layout, and the night and quiet ones carry whichever is assigned to
 * them. They are separate inputs because a single .gl-store lookup would keep finding the first.
 */
function mountStores(): { library: HTMLInputElement; night: HTMLInputElement; quiet: HTMLInputElement } {
  function input(extra: string): HTMLInputElement {
    const element = document.createElement('input');
    element.type = 'hidden';
    element.className = 'gl-store ' + extra;
    document.body.appendChild(element);
    return element;
  }
  return { library: input('gl-library'), night: input('gl-night'), quiet: input('gl-quiet') };
}

beforeEach(() => {
  document.body.innerHTML = '';
});

describe('layout assignments', () => {
  /**
   * Assigning a layout to Quiet Time has to reach its own store, or the watch never gets one.
   *
   * The picker is on the page whenever the store is, so an assignment that is only kept in the
   * library reads back as saved and leaves Quiet Time drawing the day layout forever.
   */
  test('assigning Quiet Time publishes that layout to the quiet store', () => {
    const stores = mountStores();
    const { ctx, root } = mountLayout();
    ctx.set('2,0,0,2,1');

    const quiet = root.querySelectorAll<HTMLSelectElement>('.lb-assign-sel')[2];
    quiet.value = '2';
    quiet.dispatchEvent(new Event('change'));

    const result = stores.quiet.value;

    expect(result).toBe('0');
    expect(JSON.parse(stores.library.value).quiet).toBe(2);
  });

  /**
   * The two alternate jobs must not share a store, or assigning one would overwrite the other.
   *
   * Each layout is seeded with different blocks so the two stores can be told apart. Writing both
   * jobs to one input would leave whichever went last showing up under both.
   */
  test('night and Quiet Time publish to their own stores', () => {
    const stores = mountStores();
    stores.library.value = JSON.stringify({
      layouts: ['2,0,0,2,1', '3,0,0,2,1', '6,0,0,2,1', '0', '0'], day: 0, night: -1, quiet: -1,
    });
    const { root } = mountLayout();

    const selects = root.querySelectorAll<HTMLSelectElement>('.lb-assign-sel');
    selects[1].value = '1';
    selects[1].dispatchEvent(new Event('change'));
    selects[2].value = '2';
    selects[2].dispatchEvent(new Event('change'));

    const result = { night: stores.night.value, quiet: stores.quiet.value };

    expect(result).toEqual({ night: '3,0,0,2,1', quiet: '6,0,0,2,1' });
  });

  /**
   * The Quiet Time assignment has to come back off the late read with the other two.
   *
   * Clay may build the component before its stores, so the library is re-read on the next tick.
   * Dropping the assignment there saves emptiness back over the layout the user picked.
   */
  test('keeps the Quiet Time assignment from a library that arrives late', async () => {
    const saved = JSON.stringify({ layouts: ['2,0,0,2,1', '3,0,0,2,1', '6,0,0,2,1', '1,3,0,2,1'], day: 0, night: 1, quiet: 2 });
    const { root } = mountLayout();
    const stores = mountStores();
    stores.library.value = saved;

    await new Promise((resolve) => { setTimeout(resolve, 0); }); // let the deferred re-read run
    root.querySelectorAll<HTMLElement>('.lb-ltab')[1].click();

    const result = JSON.parse(stores.library.value).quiet;

    expect(result).toBe(2);
  });
});
