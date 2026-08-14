// @vitest-environment jsdom
/**
 * Specs for the shared overlay host.
 *
 * Every popup in both builders opens through one of these, so the lifecycle
 * rules here decide whether a config screen can end up with a dead backdrop
 * dimming it and no way out. The sibling structure matters as much as the
 * open/close pair: the panel is appended beside the backdrop rather than
 * inside it, which is what keeps a tap on the panel from dismissing it.
 */

import { describe, test, expect, beforeEach } from 'vitest';
import { createOverlayHost } from './overlay';

beforeEach(() => {
  document.body.innerHTML = '';
});

/** The classes of whatever the hosts have put on the body, in document order. */
function bodyClasses(): string[] {
  return Array.from(document.body.children).map((el) => el.className);
}

describe('open', () => {
  /** A panel parented anywhere but body inherits an ancestor transform and its fixed centring lands off screen. */
  test('appends the backdrop and the panel straight to body', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', false);

    const result = host.open();

    expect(result.parentNode).toBe(document.body);
    expect(bodyClasses()).toEqual(['tb-overlay', 'tb-sheet']);
  });

  /** Handing back the backdrop instead would have the caller fill the dim layer, so the popup renders behind itself. */
  test('returns the panel for the caller to fill', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', false);

    const result = host.open();

    expect(result.className).toBe('tb-sheet');
  });

  /** Stacking a second pair leaves a backdrop behind after the close, dimming the settings page with nothing to dismiss. */
  test('tears down its own open pair before putting up another', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', false);
    const first = host.open();

    const result = host.open();

    expect(bodyClasses()).toEqual(['tb-overlay', 'tb-sheet']);
    expect(first.parentNode).toBeNull();
    expect(result.parentNode).toBe(document.body);
  });

  /** The sheet hosts the picker, so a host that closed everything would rip the sheet out from under the picker it just opened. */
  test('leaves another host\'s overlay standing', () => {
    const sheet = createOverlayHost('tb-overlay', 'tb-sheet', false);
    const picker = createOverlayHost('tb-pick-overlay', 'tb-pick', true);
    sheet.open();

    picker.open();

    expect(bodyClasses()).toEqual(['tb-overlay', 'tb-sheet', 'tb-pick-overlay', 'tb-pick']);
  });
});

describe('close', () => {
  /** Leaving either half behind blocks every tap on the settings page underneath it. */
  test('removes both the backdrop and the panel', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', false);
    host.open();

    host.close();

    expect(bodyClasses()).toEqual([]);
  });

  /** Apply closes the panel that the tap-outside handler may have already closed, and a throw there takes the whole config screen down. */
  test('does nothing when there is no open pair', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', true);
    host.open();

    const result = () => {
      host.close();
      host.close();
    };

    expect(result).not.toThrow();
    expect(bodyClasses()).toEqual([]);
  });
});

describe('dismissOnTap', () => {
  /** Tap outside is the only way out of the picker, so losing it traps the user in the popup. */
  test('closes the pair when the backdrop is tapped', () => {
    const host = createOverlayHost('tb-pick-overlay', 'tb-pick', true);
    host.open();
    const backdrop = document.body.children[0] as HTMLElement;

    backdrop.click();

    expect(bodyClasses()).toEqual([]);
  });

  /** The sheet only closes through its own Done button, and a stray tap on the dim would drop a half finished edit. */
  test('keeps the pair up when dismissOnTap is off', () => {
    const host = createOverlayHost('tb-overlay', 'tb-sheet', false);
    host.open();
    const backdrop = document.body.children[0] as HTMLElement;

    backdrop.click();

    expect(bodyClasses()).toEqual(['tb-overlay', 'tb-sheet']);
  });

  /** Nesting the panel inside the backdrop would bubble every tap on a swatch into the dismiss handler, closing the picker on the first tap. */
  test('ignores a tap on the panel itself', () => {
    const host = createOverlayHost('tb-pick-overlay', 'tb-pick', true);
    const panel = host.open();

    panel.click();

    expect(bodyClasses()).toEqual(['tb-pick-overlay', 'tb-pick']);
  });
});
