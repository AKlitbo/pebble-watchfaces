/**
 * The starter arrangements behind the preset buttons, which for now is just the one the face
 * ships with.
 *
 * Kept as data next to the face rather than in here, so a new arrangement is a JSON edit. esbuild
 * inlines the import when the component is bundled, which is what lets the config webview read it
 * without reaching for a file it cannot see.
 */

import slotPresets from '../../../../../data/slot-presets.json';

/** preset id -> the four slot ids, in slot order. */
export const SLOT_PRESETS: Record<string, number[]> = slotPresets;
