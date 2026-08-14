/**
 * Build manifest for the layoutBuilder component. The clay builder generator
 * reads this to stitch the pieces below into the self contained
 * layout-component.g.js Clay ships into its config webview.
 *
 * Every mosaic face builds the same component from this one recipe. The pieces
 * below resolve per face, so a name found in the face's own builder wins over
 * the family's copy and both win over lib's. That is how the same list picks
 * up each face's grid rules, wire format and starter layouts.
 *
 * Piece order only groups the banners for reading. Function declarations
 * hoist across the assembled initialize, so order never changes behaviour.
 */

import type { Manifest } from '../../../../../../tools/clay-components/generate-components.ts';

export default {
  name: 'layoutBuilder',
  hookPrefix: '_lb',
  template: 'html/layout.html',
  styles: ['css/layout.css'],
  pieces: [
    'ts/shared/thumbs',
    'ts/shared/overlay',
    'ts/shared/io-panel',
    'ts/layout/wire',
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
