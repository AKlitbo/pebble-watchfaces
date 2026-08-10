/**
 * The theme builder's wiring: reads the config Clay hands in, keeps the
 * colour and flag maps, and hooks the sheet, picker, and rows together with
 * the manipulator handles.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize and calls init there with the component as this, so it reaches
 * siblings through the imports below and sticks to browser APIs.
 */

import { PEBBLE_COLORS_CSV, buildPalette, buildArgbByName } from './palette';
import { serializeAppearance, parseAppearance } from './codec';
import { buildThemeModules } from './model';
import { createPicker } from './picker';
import { createRowBuilder } from './rows';
import { createSheet } from './sheet';
import type { Channel, ChannelKey, ClayComponentInstance, ColorMap, FlagMap, RawModule, Thumbs } from '../types';

/** The set/get handles init hangs on the root element for the manipulator. */
interface TbHooks {
  _tbSet(value: string): void;
  _tbGet(): string;
}

/**
 * Builds the theme editor into the component's element and hangs the set/get
 * hooks the manipulator reads. Clay serialises this and re-runs it inside the
 * config webview, so everything it needs has to be inlined by the generator.
 */
export function init(this: ClayComponentInstance): void {
  // eslint-disable-next-line @typescript-eslint/no-this-alias
  const self = this;
  const root = self.$element[0] as HTMLElement & TbHooks;
  const config = self.config || {};
  const rawModules: RawModule[] = config.moduleOptions || [];
  const THUMBS: Thumbs = config.moduleThumbnails || {}; // label -> { sizeKey -> data url } of real panel shots

  const hidden = root.querySelector('.tb-value') as HTMLInputElement;
  const editButton = root.querySelector('.tb-edit-btn') as HTMLElement;

  const modules = buildThemeModules(rawModules);
  const palette = buildPalette(PEBBLE_COLORS_CSV);
  const argbByName = buildArgbByName(palette);

  const channels: Channel[] = [
    { key: 'accent', label: 'Accent' },
    { key: 'value', label: 'Value' },
    { key: 'icon', label: 'Icon' },
    { key: 'subtitle', label: 'Sub' },
  ];

  // module value -> { accent, value, icon, subtitle } each an argb byte or null for mono
  let colors: ColorMap = {};
  // module value -> { "1x2": true, ... } for the sizes whose header is hidden. per size so
  // a module placed at two sizes can keep its header on one and drop it on the other
  let headerless: FlagMap = {};
  // module value -> { "1x2": true, ... } for the sizes whose outer border is hidden
  let borderless: FlagMap = {};

  function persist(): void {
    hidden.value = serializeAppearance(colors, headerless, borderless);
  }

  function setChannel(moduleValue: number, channelKey: ChannelKey, byte: number | null): void {
    if (!colors[moduleValue]) {
      colors[moduleValue] = { accent: null, value: null, icon: null, subtitle: null };
    }
    colors[moduleValue][channelKey] = byte;
    persist();
  }

  const picker = createPicker({
    channels: channels,
    palette: palette,
    argbByName: argbByName,
    getColors: function () { return colors; },
    getHeaderless: function () { return headerless; },
    getBorderless: function () { return borderless; },
    setChannel: setChannel,
  });

  const rowBuilder = createRowBuilder({
    thumbs: THUMBS,
    channels: channels,
    getColors: function () { return colors; },
    getHeaderless: function () { return headerless; },
    getBorderless: function () { return borderless; },
    persist: persist,
    openPicker: picker.open,
  });

  const sheet = createSheet({
    modules: modules,
    buildRow: rowBuilder.buildRow,
    getHeaderless: function () { return headerless; },
    getBorderless: function () { return borderless; },
    serialize: function () {
      return serializeAppearance(colors, headerless, borderless);
    },
    applyImport: function (text) {
      const parsed = parseAppearance(text);
      colors = parsed.colors;
      headerless = parsed.headerless;
      borderless = parsed.borderless;
      persist();
    },
    resetAll: function () {
      colors = {};
      headerless = {};
      borderless = {};
    },
    persist: persist,
    picker: picker,
    onDone: function () {
      persist();
      self.trigger('change');
    },
  });

  editButton.addEventListener('click', sheet.open);

  // expose set/get to the manipulator
  root._tbSet = function (value) {
    const parsed = parseAppearance(value);
    colors = parsed.colors;
    headerless = parsed.headerless;
    borderless = parsed.borderless;
    persist();
  };
  root._tbGet = function () {
    return serializeAppearance(colors, headerless, borderless);
  };

  root._tbSet(hidden.value || '');
}
