/**
 * Build manifest for the themeBuilder component. The clay builder generator
 * reads this to stitch the pieces below into the self contained
 * theme-component.g.js Clay ships into its config webview.
 *
 * Piece order only groups the banners for reading. Function declarations
 * hoist across the assembled initialize, so order never changes behaviour.
 */

import type { Manifest } from '../../../../../../tools/clay-components/generate-components.ts';

export default {
  name: 'themeBuilder',
  hookPrefix: '_tb',
  template: 'html/theme.html',
  styles: ['css/chrome.css', 'css/theme.css'],
  pieces: [
    'ts/shared/thumbs',
    'ts/shared/overlay',
    'ts/shared/io-panel',
    'ts/theme/palette',
    'ts/theme/codec',
    'ts/theme/model',
    'ts/theme/preview',
    'ts/theme/picker',
    'ts/theme/rows',
    'ts/theme/sheet',
    'ts/theme/init',
  ],
  output: 'src/pkjs/clay/theme-component.g.js',
  doc: [
    'Clay custom component for per-module appearance: colours plus header and',
    'border on/off toggles.',
    '',
    'Shows an "Edit Module Appearance" button. Tapping it opens a full screen',
    'editor that lists every module with four colour swatches (accent, value,',
    'icon, sub) and per size Header/Border toggles. Tapping a swatch opens the',
    '64 colour Pebble palette. A channel left alone stays mono (a striped',
    'swatch).',
    '',
    'The value it round trips is the APPEARANCE_CUSTOM_COLORS wire string the',
    'watch parses. The current sparse "~3" format is a colour section then "|"',
    'then a flag section. A colour record is 5 chars (id + four channels), a',
    'colour char is one url-safe base64 symbol holding the palette index 0..63',
    'or "." to leave that channel mono. A flag record is 3 chars (id + two',
    'packed flag chars). "0" means nothing set. The colours apply under the',
    'Custom theme, the header and border flags in every theme. The codec also',
    'reads and migrates the two older formats, see ts/theme/codec.ts.',
    '',
    'IMPORTANT: initialize and the manipulator run inside the Clay config',
    'webview, a separate JS context. They must be self contained, which is why',
    'the generator inlines every piece below into one initialize.',
  ],
} satisfies Manifest;
