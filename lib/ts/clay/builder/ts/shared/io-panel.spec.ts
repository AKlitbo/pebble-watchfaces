// @vitest-environment jsdom
/**
 * Specs for the shared import/export panel.
 *
 * This is the only way a user gets a layout or a colour set out of the watch
 * and back in, and every failure here is silent: the box seeds with the wrong
 * string, Apply re-imports the text the user just replaced, or Copy leaves
 * whatever was on the clipboard before. The two builders hand in different
 * class sets, so the styling contract gets pinned here too.
 */

import { describe, test, expect, beforeEach, afterEach, vi } from 'vitest';
import { buildIoPanel } from './io-panel';
import type { IoPanelOpts } from './io-panel';

const CSS = { title: 'tb-pick-title', textarea: 'tb-io-textarea', buttons: 'tb-io-btns', button: 'tb-io-btn' };
const COPY_RESET_MS = 1500;

// what the fake execCommand saw selected at the moment the copy ran
let copied: string | null;

beforeEach(() => {
  vi.useFakeTimers();
  document.body.innerHTML = '';
  copied = null;

  // jsdom has no execCommand so stand in for it. recording the selected text
  // rather than the call is what proves select() ran first
  document.execCommand = ((command: string) => {
    const textarea = document.querySelector('textarea');
    if (command === 'copy' && textarea) {
      copied = textarea.value.slice(textarea.selectionStart, textarea.selectionEnd);
    }
    return true;
  }) as typeof document.execCommand;
});

afterEach(() => {
  vi.useRealTimers();
});

/** Builds the panel into a fresh host, with the pieces a spec drives handed back. */
function build(overrides?: Partial<IoPanelOpts>) {
  const panel = document.createElement('div');
  document.body.appendChild(panel);

  const onApply = vi.fn();
  buildIoPanel(panel, {
    title: 'Import / Export Colours',
    css: CSS,
    value: 'seeded-wire-string',
    onApply: onApply,
    copyResetMs: COPY_RESET_MS,
    ...overrides,
  });

  const textarea = panel.querySelector('textarea') as HTMLTextAreaElement;
  const buttons = panel.querySelectorAll('button');

  return { panel, textarea, onApply, copyButton: buttons[0], applyButton: buttons[1] };
}

describe('layout', () => {
  /** Each builder styles the panel with its own prefix, and a dropped class renders the popup as an unstyled stack of controls. */
  test('stamps the caller\'s classes onto every control', () => {
    const { panel, textarea, copyButton } = build();

    const result = [
      (panel.firstChild as HTMLElement).className,
      textarea.className,
      (copyButton.parentNode as HTMLElement).className,
      copyButton.className,
    ];

    expect(result).toEqual([CSS.title, CSS.textarea, CSS.buttons, CSS.button]);
  });

  /** Without the primary marker both buttons look alike, so the user cannot tell which one commits the paste. */
  test('marks Apply as the primary button', () => {
    const { applyButton } = build();

    const result = applyButton.className;

    expect(result).toBe(CSS.button + ' primary');
  });

  /** An unseeded box means the user opens Export and copies nothing at all. */
  test('seeds the textarea with the current wire string', () => {
    const { textarea } = build({ value: 'gl:1,2,3' });

    const result = textarea.value;

    expect(result).toBe('gl:1,2,3');
  });

  /** Labelling the buttons the other way round has Copy overwrite the layout with whatever is in the box. */
  test('orders the buttons Copy then Apply', () => {
    const { copyButton, applyButton } = build();

    const result = [copyButton.textContent, applyButton.textContent];

    expect(result).toEqual(['Copy', 'Apply']);
  });
});

describe('Apply', () => {
  /** Passing the seed instead of the live text silently discards the string the user just pasted in. */
  test('hands the edited text to onApply', () => {
    const { textarea, applyButton, onApply } = build({ value: 'old-layout' });
    textarea.value = 'pasted-layout';

    applyButton.click();

    expect(onApply).toHaveBeenCalledWith('pasted-layout');
  });

  /** The caller parses and closes inside onApply, so an Apply that never fires leaves the popup up with the import dropped. */
  test('fires onApply once per tap', () => {
    const { applyButton, onApply } = build();

    applyButton.click();

    expect(onApply).toHaveBeenCalledTimes(1);
  });

  /** Copy must not commit anything, or reading the export back writes it straight over the live settings. */
  test('is not triggered by Copy', () => {
    const { copyButton, onApply } = build();

    copyButton.click();

    expect(onApply).not.toHaveBeenCalled();
  });
});

describe('Copy', () => {
  /** Copying without selecting first leaves the old clipboard in place, so the user pastes something unrelated to a friend. */
  test('copies the whole textarea contents', () => {
    const { textarea, copyButton } = build();
    textarea.value = 'edited-wire-string';

    copyButton.click();

    expect(copied).toBe('edited-wire-string');
  });

  /** No label change reads as a dead button and the user taps it again wondering whether it worked. */
  test('flips the label to Copied! on the tap', () => {
    const { copyButton } = build();

    copyButton.click();

    expect(copyButton.textContent).toBe('Copied!');
  });

  /** A label stuck on Copied! gives the next copy no feedback at all. */
  test('restores the label after copyResetMs', () => {
    const { copyButton } = build();
    copyButton.click();

    vi.advanceTimersByTime(COPY_RESET_MS - 1);
    expect(copyButton.textContent).toBe('Copied!');

    vi.advanceTimersByTime(1);
    expect(copyButton.textContent).toBe('Copy');
  });

  /** A hard coded delay would ignore the builder that asks for a longer one, cutting its confirmation short. */
  test('honours the caller\'s reset delay', () => {
    const { copyButton } = build({ copyResetMs: 2000 });
    copyButton.click();

    vi.advanceTimersByTime(1500);

    expect(copyButton.textContent).toBe('Copied!');
  });
});
