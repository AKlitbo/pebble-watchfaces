/**
 * Build manifest for the layoutBuilder component. The clay builder generator
 * reads this to stitch the pieces below into the self contained
 * layout-component.g.js Clay ships into its config webview.
 *
 * Piece order only groups the banners for reading. Function declarations
 * hoist across the assembled initialize, so order never changes behaviour.
 */

import type { Manifest } from '../../../tools/clay-components/generate-components.ts';

export default {
  name: 'layoutBuilder',
  hookPrefix: '_lb',
  template: 'html/layout.html',
  styles: ['css/layout.css'],
  pieces: [
    'ts/shared/thumbs',
    'ts/shared/overlay',
    'ts/shared/io-panel',
    'ts/layout/geometry',
    'ts/layout/codec',
    'ts/layout/presets',
    'ts/layout/visuals',
    'ts/layout/drag',
    'ts/layout/modes',
    'ts/layout/init',
  ],
  output: 'src/pkjs/clay/layout-component.g.js',
  doc: [
    'Clay custom component for the drag and drop layout builder.',
    '',
    'Shows the watch grid, a tabbed palette of modules, preset buttons and an',
    'import/export panel. The value it round trips is the LAYOUT wire string',
    'the watch reads, one "module,row,col,w,h" entry per placed block.',
    '',
    'IMPORTANT: initialize and the manipulator run inside the Clay config',
    'webview, a separate JS context. They must be self contained, which is why',
    'the generator inlines every piece below into one initialize.',
  ],
} satisfies Manifest;
