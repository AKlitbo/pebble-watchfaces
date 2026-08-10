// @vitest-environment jsdom
/**
 * Specs for the themeBuilder Clay component, driven end to end.
 *
 * The pieces carry their own unit specs, but the user flows only exist once
 * everything is wired together inside the assembled component: the
 * APPEARANCE_CUSTOM_COLORS wire string in all three of its formats, the
 * editor sheet with its header/border toggles, the colour picker's stage
 * then apply flow, and the bulk sweeps. These specs mount the generated
 * theme-component.g.js the way Clay does and drive it the way a user would.
 *
 * The wire goldens double as the contract with the C decoder in
 * settings_schema.c. If one of these breaks, the watch reads the string wrong.
 */

import { describe, test, expect, beforeEach } from 'vitest';
// the pieces are shared but the assembled component is generated into each face. every face in
// the family builds the same theme component from these sources, so gridlock's copy stands in
import component from '../../../../gridlock/src/pkjs/clay/theme-component.g.js';
import { mount, moduleOptionsFixture } from './ts/testing/harness';

/** Mounts the theme builder with the shared module fixture and no thumbnails. */
function mountTheme() {
  const mounted = mount(component, { moduleOptions: moduleOptionsFixture, moduleThumbnails: {} });
  mounted.ctx.initialize();
  return mounted;
}

/** Mounts, opens the editor sheet, and returns the mount plus the rendered rows. */
function openEditor() {
  const mounted = mountTheme();
  mounted.root.querySelector<HTMLElement>('.tb-edit-btn').click();
  return { ...mounted, rows: document.querySelectorAll<HTMLElement>('.tb-row') };
}

/** Finds a button by its visible label among the sheet's bar buttons. */
function barButton(label: string): HTMLElement | null {
  const buttons = document.querySelectorAll<HTMLElement>('.tb-bar-btn');
  for (const button of buttons) {
    if (button.textContent === label) {
      return button;
    }
  }
  return null;
}

beforeEach(() => {
  document.body.innerHTML = '';
  document.documentElement.style.overflow = '';
});

describe('APPEARANCE_CUSTOM_COLORS wire string', () => {
  /** An empty table must stay the "0" sentinel the watch treats as nothing set. */
  test('nothing set serializes to "0"', () => {
    const { ctx } = mountTheme();

    const result = ctx.get();

    expect(result).toBe('0');
  });

  /** A user upgrading from the oldest release would lose their colours if this migration breaks. */
  test('migrates a legacy 5 char record to the ~3 format', () => {
    const { ctx } = mountTheme();

    ctx.set('.....AB.CH');
    const result = ctx.get();

    expect(result).toBe('~3BAB.C|BP.');
  });

  /** A user upgrading from the positional ~ format would lose their colours if this migration breaks. */
  test('migrates a ~ positional record to the ~3 format', () => {
    const { ctx } = mountTheme();

    ctx.set('~......AB.CP.');
    const result = ctx.get();

    expect(result).toBe('~3BAB.C|BP.');
  });

  /** The current format must read back exactly or every save would mutate the settings. */
  test('a ~3 string round trips unchanged', () => {
    const { ctx } = mountTheme();

    ctx.set('~3BAB.C|BP.');
    const result = ctx.get();

    expect(result).toBe('~3BAB.C|BP.');
  });

  /** Garbage from a bad paste must fall back to defaults instead of half applied colours. */
  test('junk input serializes back to "0"', () => {
    const { ctx } = mountTheme();

    ctx.set('garbage!');
    const result = ctx.get();

    expect(result).toBe('0');
  });
});

describe('editor sheet', () => {
  /** A themeHidden module getting its own row would show duplicate swatches that fight each other. */
  test('lists one row per visible module and hides the themeHidden ones', () => {
    const { rows } = openEditor();

    const names = Array.from(document.querySelectorAll<HTMLElement>('.tb-row-nm')).map((el) => el.textContent);

    expect(rows.length).toBe(6);
    expect(names).toContain('Hourly/Daily Forecast');
    expect(names).not.toContain('Forecast (4-Day)');
  });

  /** Offering H on a headerless panel would write a flag the watch can never show. */
  test('an always headerless size row offers only the border toggle', () => {
    const { rows } = openEditor();

    const clockSizeRows = rows[0].querySelectorAll<HTMLElement>('.sz');
    const barToggles = clockSizeRows[0].querySelectorAll<HTMLElement>('.tb-toggle');
    const blockToggles = clockSizeRows[1].querySelectorAll<HTMLElement>('.tb-toggle');

    expect(barToggles.length).toBe(1);
    expect(barToggles[0].title).toBe('Border 1x4');
    expect(blockToggles.length).toBe(2);
  });

  /** A header toggle writing the wrong bit would hide the wrong panel's header on the watch. */
  test('toggling one header flag serializes that module and size alone', () => {
    const { ctx } = openEditor();

    document.querySelector<HTMLElement>('.tb-toggle[title="Header 1x2"]').click();
    const result = ctx.get();

    expect(result).toBe('~3|CB.');
  });

  /** A broken Reset would leave colours the user cannot clear without a reinstall. */
  test('Reset clears every colour and flag', () => {
    const { ctx, root } = mountTheme();
    ctx.set('~3Cw...|CB.');
    root.querySelector<HTMLElement>('.tb-edit-btn').click();

    barButton('Reset').click();
    const result = ctx.get();

    expect(result).toBe('0');
  });

  /** Done must both close the sheet and tell Clay the value changed, or the save button never lights. */
  test('Done closes the sheet and triggers change', () => {
    const { ctx } = openEditor();

    barButton('Done').click();

    expect(document.querySelector<HTMLElement>('.tb-sheet')).toBeNull();
    expect(ctx.trigger).toHaveBeenCalledWith('change');
  });
});

describe('bulk bar', () => {
  /** A bulk sweep missing one size row leaves that panel out of step with the sweep label. */
  test('All borders Off then On round trips back to "0"', () => {
    const { ctx } = openEditor();

    barButton('All borders Off').click();
    const afterOff = ctx.get();
    barButton('All borders On').click();
    const afterOn = ctx.get();

    expect(afterOff).toContain('|');
    expect(afterOff).not.toBe('0');
    expect(afterOn).toBe('0');
  });
});

describe('colour picker', () => {
  /** If Apply stops committing, no colour choice ever reaches the watch. */
  test('picking a palette colour and applying commits the channel', () => {
    const { ctx, rows } = openEditor();

    rows[1].querySelectorAll<HTMLElement>('.tb-swatch')[0].click();
    document.querySelector<HTMLElement>('.tb-cell[title="Red"]').click();
    document.querySelector<HTMLElement>('.tb-pick .tb-io-btn.primary').click();
    const result = ctx.get();

    expect(result).toBe('~3Cw...|');
    expect(document.querySelector<HTMLElement>('.tb-pick')).toBeNull();
  });

  /** The Vibrant shortcut committing the wrong byte would paint a different colour than the watch's own table. */
  test('the Vibrant toggle stages the module signature colour', () => {
    const { ctx, rows } = openEditor();

    rows[1].querySelectorAll<HTMLElement>('.tb-swatch')[0].click();
    const options = document.querySelectorAll<HTMLElement>('.tb-mv-opt');
    options[1].click();
    document.querySelector<HTMLElement>('.tb-pick .tb-io-btn.primary').click();
    const result = ctx.get();

    expect(result).toBe('~3CM...|');
  });

  /** Tap outside is cancel. Committing staged colours there would apply edits the user walked away from. */
  test('tapping outside the picker discards the staged colour', () => {
    const { ctx, rows } = openEditor();

    rows[1].querySelectorAll<HTMLElement>('.tb-swatch')[0].click();
    document.querySelector<HTMLElement>('.tb-cell[title="Red"]').click();
    document.querySelector<HTMLElement>('.tb-pick-overlay').click();
    const result = ctx.get();

    expect(result).toBe('0');
    expect(document.querySelector<HTMLElement>('.tb-pick')).toBeNull();
  });
});
