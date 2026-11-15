/**
 * The settings page chrome: a 95-style restyle of the stock Clay page, to match the desktops.
 *
 * It is a style-only Clay component. Clay drops a registered component's style into the page head
 * whether or not any item uses it, so registering it once gives the whole page the look. Nothing
 * references it as an item, so its template and manipulator never run. The manipulator is only here
 * because Clay refuses to register a component without one.
 *
 * It paints Clay's own classes with !important. The page sits on a dotted teal desktop, each section
 * is a grey window with a navy title bar, the controls are sunken white fields and boxes, and the
 * submit is a big bevelled SAVE button. The titles and the button are set in VT323 and the rest in
 * Runtime Mono, a subset of Share Tech Mono, both carried in page-font.g.js so the page draws offline
 * and asks the wearer to allow no font site.
 *
 * The theme preview's frame and the location search dropdown ship neutral in the framework, so they
 * are painted here. Each theme picker's description is moved above its picture by field-help.ts.
 */

import pageFont from './page-font.g';

// the 95 palette, all Pebble-64 colours so the page matches the watch
const DESKTOP = '#005555';
const DOTS = '#00AAAA';
const FACE = '#AAAAAA';
const SHADOW = '#555555';
const TITLE = '#0000AA';

// the raised and sunken bevels every window, button, and field is drawn with
const RAISED = `inset 1px 1px 0 #FFFFFF, inset -1px -1px 0 #000000, inset 2px 2px 0 ${FACE}, inset -2px -2px 0 ${SHADOW}`;
const SUNKEN = `inset 1px 1px 0 ${SHADOW}, inset -1px -1px 0 #FFFFFF, inset 2px 2px 0 #000000`;
const DROP = '4px 4px 0 rgba(0, 0, 0, 0.45)';

// the start glyph, a watch with a waving four-pane window. it is our own artwork
const START_GLYPH = 'url("data:image/svg+xml,' + encodeURIComponent(
  '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16">'
  + '<rect x="5" y="0" width="6" height="16" rx="1" fill="#555555"/>'
  + '<rect x="1" y="2" width="14" height="12" rx="3" fill="#000000"/>'
  + '<path d="M3 4.6Q5.2 3.6 7.5 4.2V7.6Q5.2 7 3 8Z" fill="#FF5500"/>'
  + '<path d="M8.5 4.2Q10.8 4.8 13 3.8V7.2Q10.8 8.2 8.5 7.6Z" fill="#55AA00"/>'
  + '<path d="M3 9Q5.2 8 7.5 8.6V12Q5.2 11.4 3 12.4Z" fill="#0055FF"/>'
  + '<path d="M8.5 8.6Q10.8 9.2 13 8.2V11.6Q10.8 12.6 8.5 12Z" fill="#FFAA00"/>'
  + '</svg>'
) + '")';

const TITLE_FONT = `'${pageFont.title}', monospace`;
const BODY_FONT = `'${pageFont.body}', monospace`;

export default {
  name: 'ideChrome',

  // never instantiated: no config item is this type, so this markup is never built. it only exists
  // because a component has to carry a template and a manipulator to register
  template: '<div class="ide-chrome" data-manipulator-target></div>',

  manipulator: 'val',

  style: [
    pageFont.rule,

    // the dotted teal desktop the windows sit on, and the body font across the whole page
    'body {',
    `  background-color: ${DESKTOP} !important;`,
    `  background-image: radial-gradient(${DOTS} 1px, transparent 1px) !important;`,
    '  background-size: 12px 12px !important;',
    '  color: #000000 !important;',
    '}',
    'body, .component, .component .label, .component .description, .component .value, .component input, .component select, .component button {',
    `  font-family: ${BODY_FONT} !important;`,
    '}',

    // the page heading and intro make one window, the header, with the start glyph in its title bar
    '#main-form > .component-heading, #main-form > .component-text {',
    `  background: ${FACE} !important;`,
    '  outline: 1px solid #000000;',
    '  margin: 0 !important;',
    '}',
    '#main-form > .component-heading {',
    '  margin-top: 16px !important;',
    '  padding: 3px 3px 0 !important;',
    `  box-shadow: inset 1px 1px 0 #FFFFFF, inset -1px 0 0 #000000, inset -2px 0 0 ${SHADOW} !important;`,
    '}',
    '#main-form > .component-heading h4, .section > .component-heading:first-child h4 {',
    '  display: flex !important;',
    '  align-items: center;',
    '  gap: 6px;',
    '  margin: 0 !important;',
    '  padding: 0 4px !important;',
    `  background: ${TITLE} !important;`,
    '  color: #FFFFFF !important;',
    `  font-family: ${TITLE_FONT} !important;`,
    '  font-size: 20px !important;',
    '  font-weight: 400 !important;',
    '  line-height: 24px !important;',
    '  text-transform: none !important;',
    '  letter-spacing: 0 !important;',
    '}',
    '#main-form > .component-heading h4 { font-size: 22px !important; line-height: 26px !important; }',
    '#main-form > .component-heading h4:before {',
    "  content: '';",
    '  width: 18px;',
    '  height: 18px;',
    `  background: ${START_GLYPH} center / contain no-repeat;`,
    '}',
    '#main-form > .component-text {',
    '  padding: 10px 11px 11px !important;',
    `  box-shadow: inset 1px 0 0 #FFFFFF, inset -1px -1px 0 #000000, inset -2px -2px 0 ${SHADOW}, ${DROP} !important;`,
    '  margin-bottom: 16px !important;',
    '}',
    '#main-form > .component-text p, .section .component-text p { color: #000000 !important; font-size: 13px !important; line-height: 1.35 !important; }',

    // each section is a grey window with a black outline and a drop shadow
    '.section {',
    `  background: ${FACE} !important;`,
    `  box-shadow: ${RAISED}, ${DROP} !important;`,
    '  outline: 1px solid #000000;',
    '  border: none !important;',
    '  border-radius: 0 !important;',
    '  padding: 3px !important;',
    '  margin: 0 0 16px !important;',
    '}',
    '.section .component { border-color: transparent !important; background: transparent !important; }',
    '.section > .component-heading:first-child { padding: 0 !important; margin: 0 0 6px !important; background: transparent !important; }',
    // the first row under a title bar starts close to it, and a section with no title bar, such as
    // the supported plugins panel, keeps only its own small padding
    '.section > .component-heading:first-child + .component { padding-top: 2px !important; }',
    '.section > .component-heading:first-child + .component > label { padding-top: 4px !important; }',
    '.section > .component-plugin-panel:first-child { margin-top: 0 !important; padding: 8px !important; }',

    // the section title's minimize box, a small bevelled _ at its right end
    '.section > .component-heading:first-child h4:after {',
    "  content: '_';",
    '  margin-left: auto;',
    '  width: 18px;',
    '  height: 16px;',
    '  line-height: 12px;',
    '  text-align: center;',
    `  background: ${FACE};`,
    `  box-shadow: ${RAISED};`,
    '  color: #000000;',
    '}',

    // labels and descriptions in black
    '.component .label { color: #000000 !important; font-size: 15px !important; }',
    '.component .description { color: #000000 !important; font-size: 13px !important; line-height: 1.35 !important; }',

    // a select is its label over a sunken white field with a bevelled drop button
    '.component-select label { display: block !important; }',
    '.component-select .label { display: block !important; margin-bottom: 6px !important; }',
    // a theme picker's description sits between its dropdown and the picture, moved there by field-help.ts
    '.pf-theme > label { padding-bottom: 6px !important; }',
    '.pf-theme > .description { margin: 0 !important; }',
    '.component-select .value {',
    '  display: block !important;',
    '  height: 30px !important;',
    '  line-height: 30px !important;',
    '  padding: 0 32px 0 8px !important;',
    '  overflow: hidden;',
    '  white-space: nowrap;',
    '  text-overflow: ellipsis;',
    '  background: #FFFFFF !important;',
    `  box-shadow: ${SUNKEN};`,
    '  color: #000000 !important;',
    '  font-size: 15px !important;',
    '}',
    '.component-select .value:after {',
    "  content: '\\25BC' !important;",
    '  position: absolute;',
    '  right: 2px !important;',
    '  top: 2px !important;',
    '  width: 24px;',
    '  height: 26px;',
    '  margin: 0 !important;',
    '  border: none !important;',
    '  line-height: 26px;',
    '  text-align: center;',
    `  background: ${FACE};`,
    `  box-shadow: ${RAISED};`,
    '  color: #000000;',
    '  font-size: 10px;',
    '}',

    // a text input is the same sunken field
    '.component-input input, .pf-loc-search input {',
    '  background: #FFFFFF !important;',
    `  box-shadow: ${SUNKEN} !important;`,
    '  border: none !important;',
    '  border-radius: 0 !important;',
    '  color: #000000 !important;',
    '  font-size: 15px !important;',
    '}',

    // a toggle is a sunken checkbox before its label, ticked when on
    '.component-toggle label { flex-direction: row-reverse !important; justify-content: flex-end !important; gap: 10px; }',
    '.component-toggle .graphic {',
    '  width: 20px !important;',
    '  height: 20px !important;',
    '  background: #FFFFFF;',
    `  box-shadow: ${SUNKEN};`,
    '}',
    '.component-toggle .graphic .slide, .component-toggle .graphic .marker { display: none !important; }',
    '.component-toggle input:checked + .graphic:after {',
    "  content: '\\2713';",
    '  position: absolute;',
    '  left: 0;',
    '  right: 0;',
    '  top: 0;',
    '  text-align: center;',
    `  font-family: ${TITLE_FONT};`,
    '  font-size: 20px;',
    '  line-height: 20px;',
    '  color: #000000;',
    '}',

    // a theme picker is the same sunken dropdown, with the shot of the choice centred under it in a
    // sunken black frame
    '.pf-theme .pf-theme-preview {',
    '  display: flex !important;',
    '  justify-content: center;',
    '  width: fit-content;',
    '  margin: 10px auto 8px !important;',
    '  padding: 4px !important;',
    '  background: #000000 !important;',
    `  box-shadow: ${SUNKEN};`,
    '}',
    '.pf-theme .pf-theme-img { width: 200px !important; max-width: 100% !important; border: none !important; }',

    // the supported plugins panel and the location results are sunken white boxes, and the
    // framework's own rows take these colours
    ':root {',
    `  --pf-accent: ${TITLE};`,
    '  --pf-on-accent: #FFFFFF;',
    `  --pf-note-accent: ${TITLE};`,
    `  --pf-note-info-accent: ${SHADOW};`,
    '  --pf-surface: #FFFFFF;',
    '  --pf-ink: #000000;',
    `  --pf-line: ${FACE};`,
    '}',
    '.pf-plugins, .pf-loc-search .pf-loc-list {',
    '  background: #FFFFFF !important;',
    `  box-shadow: ${SUNKEN} !important;`,
    '  border: none !important;',
    '  border-radius: 0 !important;',
    '  color: #000000 !important;',
    '}',
    `.pf-loc-search .pf-loc-item:hover, .pf-loc-search .pf-loc-item:active { background: ${TITLE}; color: #FFFFFF; }`,

    // Clay's footer keeps the submit centred where it puts it. it becomes a big bevelled SAVE
    // button, and the label Clay writes is hidden under the one drawn here
    '.component-submit { text-align: center !important; padding: 4px 0 6px !important; }',
    ".component-submit button, button[type='submit'] {",
    '  display: inline-flex !important;',
    '  align-items: center;',
    '  justify-content: center;',
    '  width: auto !important;',
    '  height: 44px !important;',
    '  padding: 0 28px !important;',
    `  background: ${FACE} !important;`,
    `  box-shadow: ${RAISED}, ${DROP} !important;`,
    '  outline: 1px solid #000000;',
    '  border: none !important;',
    '  border-radius: 0 !important;',
    '  color: #000000 !important;',
    '  font-size: 0 !important;',
    '  -webkit-tap-highlight-color: transparent !important;',
    '}',
    ".component-submit button:after, button[type='submit']:after {",
    "  content: 'SAVE';",
    `  font-family: ${TITLE_FONT};`,
    '  font-size: 30px;',
    '  line-height: 1;',
    '}',
    `.component-submit button:active, button[type='submit']:active { box-shadow: ${SUNKEN} !important; }`,
  ].join('\n'),
};
