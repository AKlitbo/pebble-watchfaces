/**
 * Build manifest for the mosaicControls component. The clay builder generator
 * reads this to stitch the pieces below into the self contained
 * controls-component.g.js Clay ships into its config webview.
 *
 * The component exists to carry controls.css. Clay appends a component's style
 * to the page once, when the component is registered, so a stylesheet listed in
 * both builders' manifests would land on the page twice. Registered on its own,
 * it lands once and reaches both builders, whichever of them the page holds.
 */

import type { Manifest } from '../../../../paf/tools/clay-components/generate-components.ts';

export default {
  name: 'mosaicControls',
  hookPrefix: '_mc',
  template: 'html/controls.html',
  styles: ['css/controls.css'],
  pieces: ['ts/controls/init'],
  output: 'src/pkjs/clay/controls-component.g.js',
  doc: [
    'Clay custom component that carries the look every mosaic settings page',
    'control shares: the colour tokens and one rule per role, from controls.css.',
    '',
    'A face registers it beside its builders and never places it on the page.',
    "Clay adds a registered component's style to the page whether or not the",
    'page uses it, so the layout builder and the appearance editor both pick it',
    'up, and it appears once however many builders the page holds.',
  ],
} satisfies Manifest;
