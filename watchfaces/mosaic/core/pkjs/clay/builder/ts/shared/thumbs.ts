/**
 * Looks up a module's real panel screenshot. Thumbnails come in keyed by the
 * module's label then by size, so both builders resolve a shot the same way.
 *
 * This is a Clay builder piece. esbuild bundles it into a component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { Thumbs } from '../types';

/** The screenshot for a label at one size, or null when there is none. */
export function thumbByLabel(thumbs: Thumbs, label: string, size: string): string | null {
  const byModule = thumbs[label];
  return (byModule && byModule[size]) || null;
}
