/**
 * Shared types for the Clay builder pieces. These are type-only, so esbuild
 * strips every import of them and nothing here reaches the bundled component.
 */

export type { Block, SizeKey } from '../../../types';

/** Label to { size key to data url } screenshot map both builders read. */
export type Thumbs = Record<string, Record<string, string>>;

/**
 * The component instance initialize runs as, with enough of Clay's shape for what the
 * builders touch. Not the component itself: that is ClayComponentDefinition, the object
 * the generated .g.js exports.
 */
export interface ClayComponentInstance {
  $element: HTMLElement[];
  config?: { moduleOptions?: RawModule[]; moduleThumbnails?: Thumbs };
  trigger(event: string): void;
}

/** One themeRows entry: a size mapped to the module whose screenshot it shows. */
export interface ThemeRow {
  size: string;
  thumb?: string;
  alwaysHeaderless?: boolean;
}

/** A module option as Clay hands it in, matching config.ts MODULE_OPTIONS. */
export interface RawModule {
  value: number;
  label: string;
  icon?: string;
  color?: string;
  sizes?: string[];
  themeRows?: ThemeRow[];
  themeLabel?: string;
  themeHidden?: boolean;
  alwaysHeaderless?: boolean;
  vibrant?: string;
}

/** The cleaned module the layout builder draws from. */
export interface ModuleInfo {
  value: number;
  label: string;
  icon: string;
  color: string;
  sizes: string[];
  themeRows?: ThemeRow[];
}

/** One of a panel's four colour channels. */
export type ChannelKey = 'accent' | 'value' | 'icon' | 'subtitle';

/** A module's four channel bytes, each an argb byte or null for mono. */
export interface ColorRecord {
  accent: number | null;
  value: number | null;
  icon: number | null;
  subtitle: number | null;
}

/** Module id keyed colour records. */
export type ColorMap = Record<number, ColorRecord>;

/** Module id keyed { size key: true } flag map, one for headerless and one for borderless. */
export type FlagMap = Record<number, Record<string, boolean>>;

/** One Pebble 64 palette colour: its argb byte, css hex, and name. */
export interface PaletteEntry {
  argb: number;
  css: string;
  name: string;
}

/** One editable colour channel: its key and the label the picker shows. */
export interface Channel {
  key: ChannelKey;
  label: string;
}

/** One per size row in the appearance editor, with its screenshot and flag target. */
export interface SizeRow {
  size: string;
  thumbLabel: string;
  value: number;
  alwaysHeaderless: boolean;
}

/** One module the appearance editor lists, with its size rows. */
export interface ThemeModule {
  value: number;
  label: string;
  thumbLabel: string;
  icon: string;
  vibrant?: string;
  sizeRows: SizeRow[];
  alwaysHeaderless: boolean;
}
