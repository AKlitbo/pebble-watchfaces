/**
 * A Clay component that stores a string but shows nothing.
 *
 * It is just a hidden input wired to Clay's built-in "val" manipulator, so its
 * value rides along with the rest of the settings: saved to the phone when the
 * config page closes and seeded back the next time it opens. The Pebble config
 * webview blocks its own localStorage, so this is how a config page keeps its
 * own scratch state (like a layout library) across visits.
 *
 * Point a page item at it with a messageKey and read or write the value off the
 * input from another component in the same page. A page with more than one of
 * these must give each a `storeClass`, since they all carry .gl-store and a
 * plain querySelector would keep finding the first.
 */

/** What Clay binds initialize to: the element and the page item's own config. */
interface HiddenStoreContext {
  $element: HTMLElement[];
  config?: { storeClass?: string };
}

export default {
  name: 'hiddenStore',

  // the input is the manipulator target itself, so Clay's val get/set round-trip
  // its value. the class is the handle another component finds it by
  template: '<input type="hidden" class="gl-store" data-manipulator-target>',

  manipulator: 'val',

  /** Stamps on the caller's own class, so several stores on one page stay tellable apart. */
  initialize(this: HiddenStoreContext): void {
    const extra = this.config && this.config.storeClass;
    const element = this.$element && this.$element[0];
    if (extra && element) {
      element.classList.add(extra);
    }
  },
};
