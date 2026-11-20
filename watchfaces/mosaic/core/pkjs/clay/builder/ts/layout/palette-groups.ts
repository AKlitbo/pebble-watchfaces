/**
 * The layout builder's group tabs, the order they show in and the icon on each. The framework's
 * palette draws them, and this list is the mosaic family's own.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's initialize, which runs in
 * the config webview, so it sticks to browser APIs.
 */

import type { PaletteGroup } from '../../../../../../paf/ts/clay/builder/ts/shared/palette';
import type { ModuleGroup } from '../types';

// keyed by every ModuleGroup, so a group added to the type without a tab here fails to compile
// rather than quietly dropping its panels into Other. the key order is the tab order
const GROUP_ICONS: Record<ModuleGroup, string> = {
  'Clock': '⏰',
  'Date': '📅',
  'Progress': '📊',
  'Weather': '🌤',
  'Sky': '🌗',
  'Health': '❤️',
  'Goals': '🎯',
  'Calendar & Stocks': '📈',
  'Status': '🔋',
};

/** The groups in the order their tabs show. */
export const PALETTE_GROUPS: PaletteGroup[] = (Object.keys(GROUP_ICONS) as ModuleGroup[]).map(function (name) {
  return { name: name, icon: GROUP_ICONS[name] };
});
