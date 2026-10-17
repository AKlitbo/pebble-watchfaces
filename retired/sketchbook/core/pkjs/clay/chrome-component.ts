/**
 * The Sketchbook page chrome: a warm paper-and-ink restyle of the stock Clay settings page, shared
 * by every Sketchbook face.
 *
 * It is a style-only Clay component. Clay drops a registered component's style into the page head
 * whether or not any item uses it, so a face registers this once in its components list and the whole
 * page takes the look. Nothing references it as an item, so its template and manipulator never run.
 * The manipulator is only here because Clay refuses to register a component without one.
 *
 * It paints Clay's own chrome classes with !important: the paper background, the section cards, the
 * headings, the form controls, and the submit. The hand lettering is Patrick Hand, the same face the
 * watch draws its clock with, pulled from Google Fonts. Offline the page falls back to a plain sans,
 * which costs the lettering but nothing else.
 *
 * It also repaints two things the framework ships neutral on purpose: the theme preview's frame and
 * the location search dropdown. Those stay skin-agnostic in the framework, so a face paints them
 * from here. This sheet loads after their own styles, so plain declarations win where they need to.
 */

export default {
  name: 'sketchbookChrome',

  // never instantiated: no config item is this type, so this markup is never built. it only exists
  // because a component has to carry a template and a manipulator to register
  template: '<div class="sb-chrome" data-manipulator-target></div>',

  manipulator: 'val',

  style: [
    "@import url('https://fonts.googleapis.com/css2?family=Patrick+Hand&display=swap');",

    // warm paper with a faint dot grid, the way a sketchbook or bullet journal is ruled, and the
    // hand font across the whole page
    'body {',
    '  background-color: #f1e8d4 !important;',
    '  background-image: radial-gradient(rgba(120, 108, 84, 0.18) 1.3px, transparent 1.4px);',
    '  background-size: 20px 20px;',
    '  background-position: -6px -6px;',
    '  color: #2c2a24 !important;',
    '  padding-top: 0.8rem !important;',
    '  padding-bottom: 40px !important;',
    '}',
    'body, .component, .component .label, .component .description, .component .value, .component-heading, .component input, .component select, .component button {',
    "  font-family: 'Patrick Hand', 'Segoe UI', system-ui, sans-serif !important;",
    '}',

    // each section is a paper card with an ink edge and a soft drop
    // a top margin on every section opens a gap after the intro and between the cards, and leaves
    // none after the last one, so the submit still sits close under it
    '.section {',
    '  background: #fcf7ea !important;',
    '  border: 2px solid #35322b !important;',
    '  border-radius: 8px !important;',
    '  box-shadow: 2px 3px 0 rgba(53, 50, 43, 0.16) !important;',
    '  margin-top: 0.9rem !important;',
    '}',
    '.section > .component:first-child { margin-top: 0 !important; }',

    // headings in ink, a touch larger, the section title ruled off underneath
    '.component-heading {',
    '  color: #2c2a24 !important;',
    '  letter-spacing: 0.5px !important;',
    '  font-size: 1.35rem !important;',
    '}',
    // a section opens with a title bar that Clay paints dark. make it a warm tan band with ink
    // lettering, rounded to sit on top of the paper card
    '.section .component-heading:first-child {',
    '  background: #ece0c6 !important;',
    '  color: #2c2a24 !important;',
    '  border-bottom: 2px solid #d3c49f !important;',
    '  border-radius: 6px 6px 0 0 !important;',
    '}',
    '.component .label { color: #2c2a24 !important; font-size: 1.08rem !important; }',
    '.component .description { color: #6f6757 !important; }',

    // form controls on bright paper with an ink edge. Clay's text inputs carry no type attribute, so
    // they are reached through their component wrapper rather than by input type
    'select, .component-input input {',
    '  background: #fffdf5 !important;',
    '  color: #2c2a24 !important;',
    '  border: 2px solid #35322b !important;',
    '  border-radius: 6px !important;',
    '}',

    // the toggle's on state takes the warm sun the faces paint their disc with, not Clay's orange
    '.component-toggle input:checked + .graphic .slide { background: #e8b84b !important; }',
    '.component-toggle input:checked + .graphic .marker { background: #b98a22 !important; }',

    // submit: an ink stamp with cream lettering, full width with a gap under it
    ".component-submit button, button[type='submit'] {",
    '  display: block !important;',
    '  width: 100% !important;',
    '  min-width: 0 !important;',
    '  box-sizing: border-box !important;',
    '  margin: 0 !important;',
    '  padding: 13px !important;',
    '  background: #2c2a24 !important;',
    '  color: #f1e8d4 !important;',
    '  border: 2px solid #2c2a24 !important;',
    '  border-radius: 8px !important;',
    '  font-size: 1.25rem !important;',
    '  letter-spacing: 0.5px !important;',
    '  cursor: pointer !important;',
    '  transition: background 0.12s !important;',
    '}',
    '.component-submit { padding-bottom: 0.8rem !important; }',
    ".component-submit button:hover, button[type='submit']:hover { background: #46413a !important; }",
    ".component-submit button:active, button[type='submit']:active { background: #22201b !important; }",

    // the theme preview frames take the ink edge rather than the framework's neutral grey
    '.theme-preview .tp-img { border-color: #35322b !important; }',

    // the location search dropdown, painted onto the paper
    '.loc-search .loc-list { background: #fcf7ea; color: #2c2a24; border: 2px solid #35322b; border-radius: 6px; }',
    '.loc-search .loc-item { border-bottom: 1px solid #e6dcc1; }',
    '.loc-search .loc-item:hover, .loc-search .loc-item:active { background: #e8b84b; color: #2c2a24; }',
    '.loc-search .loc-note { background: rgba(232, 184, 75, 0.18) !important; border-left-color: #e8b84b !important; }',
    '.loc-search .loc-note:before { border-color: #b98a22 !important; color: #8a6714 !important; }',

    // the page has a day and a night the way the faces do. when the phone is set to dark, the warm
    // paper turns to a deep night blue, the ink turns to a cool off-white, and the accent cools from
    // the daytime sun to a moonlit blue, matching the faces' blue night sky
    ':root { color-scheme: light dark; }',
    '@media (prefers-color-scheme: dark) {',
    '  body {',
    '    background-color: #0f1521 !important;',
    '    background-image: radial-gradient(rgba(150, 170, 205, 0.14) 1.3px, transparent 1.4px) !important;',
    '    color: #dde5f2 !important;',
    '  }',
    // every heading, the page title included, takes the cool ink so the top title stays readable
    '  .component-heading { color: #dde5f2 !important; }',
    '  .section {',
    '    background: #182234 !important;',
    '    border-color: #30405c !important;',
    '    box-shadow: 2px 3px 0 rgba(0, 0, 0, 0.4) !important;',
    '  }',
    '  .section .component-heading:first-child {',
    '    background: #1f2c43 !important;',
    '    color: #dde5f2 !important;',
    '    border-bottom-color: #30405c !important;',
    '  }',
    '  .component .label { color: #dde5f2 !important; }',
    '  .component .description { color: #93a2bd !important; }',
    '  select, .component-input input {',
    '    background: #0e1522 !important;',
    '    color: #dde5f2 !important;',
    '    border-color: #30405c !important;',
    '  }',
    '  .component-toggle input:checked + .graphic .slide { background: #5c8fd4 !important; }',
    '  .component-toggle input:checked + .graphic .marker { background: #35598f !important; }',
    // the ink stamp inverts to a pale one so it still reads on the dark paper
    "  .component-submit button, button[type='submit'] {",
    '    background: #dde5f2 !important;',
    '    color: #0f1521 !important;',
    '    border-color: #dde5f2 !important;',
    '  }',
    "  .component-submit button:hover, button[type='submit']:hover { background: #eef3fb !important; }",
    "  .component-submit button:active, button[type='submit']:active { background: #c3cfe2 !important; }",
    '  .theme-preview .tp-img { border-color: #30405c !important; }',
    '  .loc-search .loc-list { background: #182234; color: #dde5f2; border-color: #30405c; }',
    '  .loc-search .loc-item { border-bottom-color: #273449; }',
    '  .loc-search .loc-item:hover, .loc-search .loc-item:active { background: #5c8fd4; color: #0f1521; }',
    '  .loc-search .loc-note { background: rgba(92, 143, 212, 0.15) !important; border-left-color: #5c8fd4 !important; }',
    '  .loc-search .loc-note:before { border-color: #5c8fd4 !important; color: #8fb3e4 !important; }',
    '}',
  ].join('\n'),
};
