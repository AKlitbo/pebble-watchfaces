/**
 * The settings page's custom function: it moves each theme picker's description up between its
 * dropdown and the picture of the choice, so it reads with the dropdown rather than after the shot.
 *
 * The framework builds a theme picker as its dropdown, the picture, then the description. Once the
 * page is built, this moves the description element itself, so the framework can still show and hide
 * it. Every other dropdown keeps its description under the field, where Clay puts it.
 *
 * Clay copies this function's body into the page, so it reaches nothing outside itself: no import
 * and no module constant.
 */

/** The bits of a Clay row this reaches. */
interface FieldRow {
  $element?: HTMLElement[];
}

/** The bits of the Clay page this reaches. */
interface FieldPage {
  EVENTS: { AFTER_BUILD: string };
  on(event: string, handler: () => void): void;
  getAllItems(): FieldRow[];
}

/**
 * Moves every theme picker's description above its picture once the page is built.
 *
 * @param this The Clay page.
 */
export default function fieldHelp(this: FieldPage): void {
  this.on(this.EVENTS.AFTER_BUILD, () => {
    this.getAllItems().forEach((row) => {
      const root = row.$element && row.$element[0];

      if (!root || !root.classList.contains('pf-theme')) {
        return;
      }

      const picture = root.querySelector(':scope > .pf-theme-preview');
      const description = root.querySelector(':scope > .description');

      if (picture && description) {
        root.insertBefore(description, picture);
      }
    });
  });
}
