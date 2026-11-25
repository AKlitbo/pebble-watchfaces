/**
 * The line an import popup shows when the pasted text holds nothing it can use.
 *
 * The layout and colour imports both read what they can and skip the rest, so a stray paste can
 * read as empty. Applying that would wipe the grid or every colour. Each builder refuses it instead
 * and calls this, so the wearer sees why nothing happened.
 *
 * This is a Clay builder piece. esbuild bundles it into a component's initialize, which runs in
 * the config webview, so it sticks to browser APIs.
 */

/**
 * Puts the message in the popup just above its buttons, or updates it when one is already there.
 *
 * The line goes as soon as the wearer edits the text, since it no longer describes what is in the box.
 *
 * @param panel The open popup that buildIoPanel filled.
 * @param noteClass The class the line wears, which its stylesheet sizes.
 * @param buttonsClass The class on the popup's button row, which the line sits above.
 * @param message What to tell the wearer.
 */
export function showImportNote(panel: HTMLElement, noteClass: string, buttonsClass: string, message: string): void {
  let note = panel.querySelector('.' + noteClass) as HTMLElement | null;

  if (!note) {
    note = document.createElement('div');
    note.className = noteClass + ' mo-io-note';
    note.setAttribute('role', 'alert');

    const buttons = panel.querySelector('.' + buttonsClass);

    panel.insertBefore(note, buttons);

    const textarea = panel.querySelector('textarea');
    const shown = note;

    if (textarea) {
      textarea.addEventListener('input', function () {
        if (shown.parentNode) {
          shown.parentNode.removeChild(shown);
        }
      });
    }
  }

  note.textContent = message;
}
