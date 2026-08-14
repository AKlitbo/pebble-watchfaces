/**
 * Shared types for the Clay builder pieces. These are type-only, so esbuild
 * strips every import of them and nothing here reaches the bundled component.
 */

export type { Block, SizeKey } from '../../../types';

// the builder shapes that carry no grid vocabulary belong to lib. re-exported so the mosaic
// pieces keep naming one types module rather than two
export type { Thumbs, ClayComponentInstance } from '../../../../../../../lib/ts/clay/builder/ts/types';
import type { RawOption } from '../../../../../../../lib/ts/clay/builder/ts/types';

/** One themeRows entry: a size mapped to the module whose screenshot it shows. */
export interface ThemeRow {
  size: string;
  thumb?: string;
  alwaysHeaderless?: boolean;
}

/** A module option as Clay hands it in, matching config.ts MODULE_OPTIONS. */
export interface RawModule extends RawOption {
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
