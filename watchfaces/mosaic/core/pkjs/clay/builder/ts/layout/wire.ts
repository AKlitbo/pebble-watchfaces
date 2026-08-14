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
 * The watch discards an empty cstring rather than storing it (see
 * settings_apply_inbox), so "" cannot say "I cleared this" — the old layout
 * would just stay. A single character that parses to no blocks can.
 */
export const EMPTY_LAYOUT = '0';
