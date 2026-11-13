/**
 * Appearance rows both faces in the family put in their own Appearance sections.
 *
 * Each face lays out its Appearance section itself, since the theme and its colour rows differ. A
 * row both of them carry under the same key is built once here rather than written out twice and
 * drifting.
 */

import type { ClayConfigItem } from '../../paf/ts/clay/types';

/**
 * The header font picker. The values line up with the watch's own font list.
 *
 * @return The Header Font select, Share Tech Mono by default.
 */
export function headerFontItem(): ClayConfigItem {
  return {
    type: 'select',
    messageKey: 'APPEARANCE_HEADER_FONT',
    label: 'Header Font',
    description: 'The font for the header at the top of each panel.',
    defaultValue: 0,
    options: [
      { label: 'Share Tech Mono (Default)', value: 0 },
      { label: 'LECO', value: 1 },
      { label: 'Press Start 2P', value: 2 },
      { label: 'Pixelify Sans', value: 3 },
      { label: 'Aldrich', value: 4 },
      { label: 'Kode Mono', value: 5 },
      { label: 'Electrolize', value: 6 },
      { label: 'Quantico', value: 7 },
    ],
  };
}

/**
 * The panel corner picker.
 *
 * @return The Panel Style select, square corners by default.
 */
export function panelStyleItem(): ClayConfigItem {
  return {
    type: 'select',
    messageKey: 'APPEARANCE_PANEL_STYLE',
    label: 'Panel Style',
    description: 'Square or rounded corners on every panel.',
    defaultValue: 0,
    options: [
      { label: 'Classic (Default)', value: 0 },
      { label: 'Rounded', value: 1 },
    ],
  };
}

export default { headerFontItem, panelStyleItem };
