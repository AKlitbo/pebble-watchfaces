/**
 * Shared Clay types for the pkjs runtime. Kept in lib (not imported from
 * src/pkjs) so lib stays independent as the copy-as-starter base.
 */

/** A value Clay stores for a setting: a scalar, or the { value } wrapper Clay
 * puts around some component values. */
export type ClayValue = string | number | boolean | { value: string | number | boolean };

/**
 * One Clay config item (a section, heading, select, toggle, input, custom
 * component, etc.). The fields both the config builder and the runtime touch are
 * declared. Custom-component props ride along as optional. No index signature, so
 * a face's own typed config array stays assignable to this.
 */
export interface ClayConfigItem {
  type: string;
  messageKey?: string;
  label?: string;
  description?: string;
  defaultValue?: string | number | boolean;
  options?: Array<{ label: string; value: string | number }>;
  items?: ClayConfigItem[];
  attributes?: Record<string, unknown>;
  // custom components (layoutBuilder / themeBuilder) carry these
  moduleOptions?: unknown;
  moduleThumbnails?: unknown;
}
