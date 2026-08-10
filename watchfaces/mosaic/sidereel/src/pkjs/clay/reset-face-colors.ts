/**
 * The Reset Face Colours button.
 *
 * Clay's colour rows have no "unset" the way the panel editor's swatches do, so the four face
 * pickers need a control that puts them back. Clay serialises this with toSource and re-runs it
 * inside the config webview, so it reaches nothing outside itself: no imports, no module-scope
 * constants, and the defaults come off the rows themselves rather than being written twice.
 */

/** The bits of a Clay row this reaches. */
interface ClayRow {
  config: { defaultValue?: unknown };
  set(value: unknown): void;
  on(event: string, handler: () => void): void;
}

/** The bits of the Clay config page this reaches. */
interface ClayPage {
  EVENTS: { AFTER_BUILD: string };
  on(event: string, handler: () => void): void;
  getItemById(id: string): ClayRow;
  getItemByMessageKey(messageKey: string): ClayRow;
}

export default function resetFaceColors(this: ClayPage): void {
  // arrows rather than a this alias, so the handlers reach the page without carrying a copy
  this.on(this.EVENTS.AFTER_BUILD, () => {
    const keys = [
      'APPEARANCE_POINTER_COLOR',
      'APPEARANCE_POINTER_INK',
      'APPEARANCE_REEL_COLOR',
      'APPEARANCE_REEL_INK',
    ];

    this.getItemById('resetFaceColors').on('click', () => {
      // this only restages the pickers. the user still saves to push them to the watch
      keys.forEach((key) => {
        const row = this.getItemByMessageKey(key);
        row.set(row.config.defaultValue);
      });
    });
  });
}
