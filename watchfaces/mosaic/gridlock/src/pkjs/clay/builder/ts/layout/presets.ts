/**
 * The starter layouts behind the preset buttons, one LAYOUT wire string each.
 * The strings live in src/data/layout-presets.json so the dev tap walk and
 * this builder read the same table.
 *
 * This is a Clay builder piece. esbuild bundles it (and inlines the JSON) into
 * the component's initialize, which runs in the config webview.
 */

import layoutPresets from '../../../../../data/layout-presets.json';

export const LAYOUT_PRESETS: Record<string, string> = layoutPresets;
