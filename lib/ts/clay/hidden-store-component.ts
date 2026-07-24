/**
 * A Clay component that stores a string but shows nothing.
 *
 * It is just a hidden input wired to Clay's built-in "val" manipulator, so its
 * value rides along with the rest of the settings: saved to the phone when the
 * config page closes and seeded back the next time it opens. The Pebble config
 * webview blocks its own localStorage, so this is how a config page keeps its
 * own scratch state (like saved layout slots) across visits.
 *
 * Point a page item at it with a messageKey and read or write the value off the
 * .gl-store input from another component in the same page.
 */
export default {
  name: 'hiddenStore',

  // the input is the manipulator target itself, so Clay's val get/set round-trip
  // its value. the class is the handle another component finds it by
  template: '<input type="hidden" class="gl-store" data-manipulator-target>',

  manipulator: 'val',
};
