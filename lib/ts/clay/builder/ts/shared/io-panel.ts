/**
 * The shared import/export panel body: a title, a textarea holding the current
 * wire string, a Copy button, and an Apply button. Both builders open one of
 * these so users can share their layout or colours as plain text.
 *
 * This is a Clay builder piece. esbuild bundles it into a component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

/** The controls buildIoPanel wires up: which classes to use and what to do on Apply. */
export interface IoPanelOpts {
  title: string;
  css: { title: string; textarea: string; buttons: string; button: string };
  value: string;
  onApply: (text: string) => void;
  copyResetMs: number;
}

/**
 * Fills a panel with the import/export controls.
 *
 * Copy selects the textarea and copies it, flipping its label to "Copied!"
 * for a moment. Apply hands the textarea text to onApply, which is where the
 * caller parses it and closes the panel. The primary Apply button gets
 * css.button plus " primary".
 */
export function buildIoPanel(panel: HTMLElement, opts: IoPanelOpts): void {
  const title = document.createElement('div');
  title.className = opts.css.title;
  title.textContent = opts.title;
  panel.appendChild(title);

  const textarea = document.createElement('textarea');
  textarea.className = opts.css.textarea;
  textarea.value = opts.value;
  panel.appendChild(textarea);

  const buttons = document.createElement('div');
  buttons.className = opts.css.buttons;

  const copyButton = document.createElement('button');
  copyButton.type = 'button';
  copyButton.className = opts.css.button;
  copyButton.textContent = 'Copy';
  copyButton.addEventListener('click', function () {
    textarea.select();
    document.execCommand('copy');
    copyButton.textContent = 'Copied!';
    setTimeout(function () {
      copyButton.textContent = 'Copy';
    }, opts.copyResetMs);
  });
  buttons.appendChild(copyButton);

  const applyButton = document.createElement('button');
  applyButton.type = 'button';
  applyButton.className = opts.css.button + ' primary';
  applyButton.textContent = 'Apply';
  applyButton.addEventListener('click', function () {
    opts.onApply(textarea.value);
  });
  buttons.appendChild(applyButton);

  panel.appendChild(buttons);
}
