/**
 * The Sketchbook Horizon page chrome: a warm paper-and-ink restyle of the stock Clay settings
 * page.
 *
 * It is a style-only Clay component. Clay drops a registered component's style into the page head
 * whether or not any item uses it, so the face registers this once in its components list and the whole
 * page takes the look. Nothing references it as an item, so its template and manipulator never run.
 * The manipulator is only here because Clay refuses to register a component without one.
 *
 * It paints Clay's own chrome classes with !important: the paper background, the section cards, the
 * headings, the form controls, and the submit. The hand lettering is Patrick Hand, the same face the
 * watch draws its clock with. The page carries a Latin subset of it as Sketchbook Hand, from
 * page-font.g.js, so it draws offline and asks the wearer to allow no font site.
 *
 * It also repaints two things the framework ships neutral on purpose: the theme preview's frame and
 * the location search dropdown. Those stay skin-agnostic in the framework, so the face paints them
 * from here. This sheet loads after their own styles, so plain declarations win where they need to.
 */

import pageFont from './page-font.g';

export default {
  name: 'sketchbookChrome',

  // never instantiated: no config item is this type, so this markup is never built. it only exists
  // because a component has to carry a template and a manipulator to register
  template: '<div class="sb-chrome" data-manipulator-target></div>',

  manipulator: 'val',

  style: [
    pageFont.rule,

    // the page has a day and a night the way the scenes do. when the phone is set to dark, the warm
    // paper turns to a deep night blue, the ink turns to a cool off-white, and the accent cools from
    // the daytime sun to a moonlit blue, matching the scenes' blue night sky
    ':root { color-scheme: light dark; }',

    // every colour on the page is one of these, so each rule below is written once and the night only
    // swaps the values. the framework's own rows, such as the plugin notes and the supported plugins
    // list, read the --pf- ones. the day takes the deep gold for links and the sun for a warning. the
    // face's ink and card are the framework's, so the night swaps them there
    ':root {',
    '  --pf-accent: #8a6714;',
    '  --pf-on-accent: #fcf7ea;',
    '  --pf-note-accent: #e8b84b;',
    '  --pf-note-info-accent: #8a8070;',
    '  --pf-surface: #fcf7ea;',
    '  --pf-ink: #2c2a24;',
    '  --pf-line: #e6dcc1;',
    '  --sb-paper: #f1e8d4;',
    '  --sb-dots: rgba(120, 108, 84, 0.18);',
    '  --sb-ink: var(--pf-ink);',
    '  --sb-muted: #6f6757;',
    '  --sb-card: var(--pf-surface);',
    '  --sb-edge: #35322b;',
    '  --sb-shadow: rgba(53, 50, 43, 0.16);',
    '  --sb-band: #ece0c6;',
    '  --sb-band-edge: #d3c49f;',
    '  --sb-field: #fffdf5;',
    '  --sb-sun: #e8b84b;',
    '  --sb-sun-deep: #b98a22;',
    '  --sb-on-sun: #2c2a24;',
    '  --sb-stamp-hover: #46413a;',
    '  --sb-stamp-press: #22201b;',
    '  --sb-rule: #e6dcc1;',
    '  --sb-note: rgba(232, 184, 75, 0.18);',
    '  --sb-note-mark: #b98a22;',
    '  --sb-note-ink: #8a6714;',
    '}',
    '@media (prefers-color-scheme: dark) {',
    '  :root {',
    '    --pf-accent: #8fb3e4;',
    '    --pf-on-accent: #0f1521;',
    '    --pf-note-accent: #5c8fd4;',
    '    --pf-note-info-accent: #93a2bd;',
    '    --pf-surface: #182234;',
    '    --pf-ink: #dde5f2;',
    '    --pf-line: #30405c;',
    '    --sb-paper: #0f1521;',
    '    --sb-dots: rgba(150, 170, 205, 0.14);',
    '    --sb-muted: #93a2bd;',
    '    --sb-edge: #30405c;',
    '    --sb-shadow: rgba(0, 0, 0, 0.4);',
    '    --sb-band: #1f2c43;',
    '    --sb-band-edge: #30405c;',
    '    --sb-field: #0e1522;',
    '    --sb-sun: #5c8fd4;',
    '    --sb-sun-deep: #35598f;',
    '    --sb-on-sun: #0f1521;',
    // the ink stamp inverts to a pale one so it still reads on the dark paper
    '    --sb-stamp-hover: #eef3fb;',
    '    --sb-stamp-press: #c3cfe2;',
    '    --sb-rule: #273449;',
    '    --sb-note: rgba(92, 143, 212, 0.15);',
    '    --sb-note-mark: #5c8fd4;',
    '    --sb-note-ink: #8fb3e4;',
    '  }',
    // the one rule the night restates rather than swapping a value, since only the night's dot grid
    // is !important
    '  body { background-image: radial-gradient(var(--sb-dots) 1.3px, transparent 1.4px) !important; }',
    '}',

    // paper with a faint dot grid, the way a sketchbook or bullet journal is ruled, and the hand font
    // across the whole page
    'body {',
    '  background-color: var(--sb-paper) !important;',
    '  background-image: radial-gradient(var(--sb-dots) 1.3px, transparent 1.4px);',
    '  background-size: 20px 20px;',
    '  background-position: -6px -6px;',
    '  color: var(--sb-ink) !important;',
    '  padding-top: 0.8rem !important;',
    '  padding-bottom: 40px !important;',
    '}',
    'body, .component, .component .label, .component .description, .component .value, .component-heading, .component input, .component select, .component button {',
    "  font-family: '" + pageFont.family + "', 'Segoe UI', system-ui, sans-serif !important;",
    '}',

    // each section is a paper card with an ink edge and a soft drop
    // a top margin on every section opens a gap after the intro and between the cards, and leaves
    // none after the last one, so the submit still sits close under it
    '.section {',
    '  background: var(--sb-card) !important;',
    '  border: 2px solid var(--sb-edge) !important;',
    '  border-radius: 8px !important;',
    '  box-shadow: 2px 3px 0 var(--sb-shadow) !important;',
    '  margin-top: 0.9rem !important;',
    '}',
    '.section > .component:first-child { margin-top: 0 !important; }',

    // every heading, the page title included, in ink and a touch larger, the section title ruled off
    // underneath
    '.component-heading {',
    '  color: var(--sb-ink) !important;',
    '  letter-spacing: 0.5px !important;',
    '  font-size: 1.35rem !important;',
    '}',
    // a section opens with a title bar that Clay paints dark. make it a band with ink lettering,
    // rounded to sit on top of the paper card
    '.section .component-heading:first-child {',
    '  background: var(--sb-band) !important;',
    '  color: var(--sb-ink) !important;',
    '  border-bottom: 2px solid var(--sb-band-edge) !important;',
    '  border-radius: 6px 6px 0 0 !important;',
    '}',
    '.component .label { color: var(--sb-ink) !important; font-size: 1.08rem !important; }',
    '.component .description { color: var(--sb-muted) !important; }',

    // form controls on bright paper with an ink edge. Clay's text inputs carry no type attribute, so
    // they are reached through their component wrapper rather than by input type
    'select, .component-input input {',
    '  background: var(--sb-field) !important;',
    '  color: var(--sb-ink) !important;',
    '  border: 2px solid var(--sb-edge) !important;',
    '  border-radius: 6px !important;',
    '}',

    // the toggle's on state takes the sun the scenes paint their disc with, not Clay's orange
    '.component-toggle input:checked + .graphic .slide { background: var(--sb-sun) !important; }',
    '.component-toggle input:checked + .graphic .marker { background: var(--sb-sun-deep) !important; }',

    // submit: an ink stamp lettered in the paper colour, full width with a gap under it
    ".component-submit button, button[type='submit'] {",
    '  display: block !important;',
    '  width: 100% !important;',
    '  min-width: 0 !important;',
    '  box-sizing: border-box !important;',
    '  margin: 0 !important;',
    '  padding: 13px !important;',
    '  background: var(--sb-ink) !important;',
    '  color: var(--sb-paper) !important;',
    '  border: 2px solid var(--sb-ink) !important;',
    '  border-radius: 8px !important;',
    '  font-size: 1.25rem !important;',
    '  letter-spacing: 0.5px !important;',
    '  cursor: pointer !important;',
    '  transition: background 0.12s !important;',
    // Clay gives a pressed submit a bright tap highlight of its own, which the Android webview lays
    // over the whole button, so it goes clear and the pressed ink below shows instead
    '  -webkit-tap-highlight-color: transparent !important;',
    '}',
    '.component-submit { padding-bottom: 0.8rem !important; }',
    ".component-submit button:hover, button[type='submit']:hover { background: var(--sb-stamp-hover) !important; }",
    ".component-submit button:active, button[type='submit']:active { background: var(--sb-stamp-press) !important; }",

    // the theme preview frames take the ink edge rather than the framework's neutral grey
    '.pf-theme .pf-theme-img { border-color: var(--sb-edge) !important; }',

    // the location search dropdown, painted onto the paper. the framework already fills it from
    // --pf-surface and letters it in --pf-ink, so only the edge is the face's own
    '.pf-loc-search .pf-loc-list { border: 2px solid var(--sb-edge); border-radius: 6px; }',
    '.pf-loc-search .pf-loc-item { border-bottom: 1px solid var(--sb-rule); }',
    '.pf-loc-search .pf-loc-item:hover, .pf-loc-search .pf-loc-item:active { background: var(--sb-sun); color: var(--sb-on-sun); }',
    '.pf-loc-search .pf-loc-note { background: var(--sb-note) !important; border-left-color: var(--sb-sun) !important; }',
    '.pf-loc-search .pf-loc-note:before { border-color: var(--sb-note-mark) !important; color: var(--sb-note-ink) !important; }',
  ].join('\n'),
};
