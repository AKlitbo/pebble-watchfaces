/**
 * The bits of the LAYOUT wire format that are the family's rather than any one
 * face's. Each face codes and decodes against its own grid, but they agree on
 * what nothing looks like.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

/**
 * What an empty grid sends instead of an empty string.
 *
 * The watch keeps an empty string only for a setting whose default is empty, and
 * every layout has a default, so "" would leave the old layout in place. A single
 * character that parses to no blocks clears it.
 */
export const EMPTY_LAYOUT = '0';
