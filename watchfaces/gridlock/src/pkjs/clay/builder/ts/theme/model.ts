/**
 * Turns the raw moduleOptions config.ts hands in into the module list the
 * appearance editor shows: which modules get a row, what each row is called,
 * and the per size sub rows with their screenshots and flag targets.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { flagOn } from './codec';
import type { FlagMap, RawModule, SizeRow, ThemeModule } from '../types';

/**
 * Label to raw option lookup, so a themeRows sub row can find the real module
 * it represents (and that module's own id and alwaysHeaderless, since flags
 * are per panel).
 */
export function buildOptionByLabel(rawModules: RawModule[]): Record<string, RawModule> {
  const optionByLabel: Record<string, RawModule> = {};
  for (let i = 0; i < rawModules.length; i++) {
    if (rawModules[i]) {
      optionByLabel[rawModules[i].label] = rawModules[i];
    }
  }

  return optionByLabel;
}

/**
 * The per size rows the appearance editor shows for a module, each
 * { size, thumbLabel, value, alwaysHeaderless }. themeRows lets a shared
 * theme family name each row's size and which module's screenshot it uses
 * (stock 2x2 plus watchlist 2x4). Otherwise every placeable size becomes a
 * row showing this module's own screenshot. value and alwaysHeaderless come
 * from the panel the row represents so its H/B toggle targets that panel's
 * own flags.
 */
export function sizeRowsFor(option: RawModule, optionByLabel: Record<string, RawModule>): SizeRow[] {
  if (option.themeRows) {
    return option.themeRows.map(function (row) {
      const src = optionByLabel[row.thumb || ''] || option;
      // alwaysHeaderless is per panel: a themeRows entry can set it outright (the digital
      // clock's 1x4 bar has no header but its 2x2 block does so only the bar drops the H
      // toggle). otherwise it comes from the module the row's screenshot belongs to
      const panelHeaderless = row.alwaysHeaderless != null
        ? row.alwaysHeaderless
        : (src.alwaysHeaderless || option.alwaysHeaderless);
      return {
        size: row.size,
        thumbLabel: row.thumb || option.label,
        value: src.value,
        alwaysHeaderless: Boolean(panelHeaderless),
      };
    });
  }

  return (option.sizes || []).map(function (size) {
    return {
      size: size,
      thumbLabel: option.label,
      value: option.value,
      alwaysHeaderless: Boolean(option.alwaysHeaderless),
    };
  });
}

/**
 * Every module the editor lists (Empty and the themeHidden ones skipped),
 * each with its display label, icon, vibrant colour, and size rows. A module
 * only counts as always headerless when every one of its panels is (the
 * digital clock's 1x4 bar is, but its 2x2 block has a real header), so the
 * colour example keeps its header whenever any size can show one.
 */
export function buildThemeModules(rawModules: RawModule[]): ThemeModule[] {
  const optionByLabel = buildOptionByLabel(rawModules);
  const modules: ThemeModule[] = [];
  for (let i = 0; i < rawModules.length; i++) {
    const option = rawModules[i];
    // themeHidden modules share another module's theming row (the forecast panels share
    // one "Forecast" entry) so they get no row of their own. themeLabel renames the shared row
    if (!option || option.value === 0 || option.themeHidden) {
      continue;
    }

    const sizeRows = sizeRowsFor(option, optionByLabel);
    modules.push({
      value: option.value,
      label: option.themeLabel || option.label,
      // themeLabel renames the row but thumbnails are keyed by the real label so keep it
      // for the lookup. the forecast row survives on the hourly panel (4-day is themeHidden)
      // so it shows the hourly shot
      thumbLabel: option.label,
      icon: option.icon || '·',
      vibrant: option.vibrant,
      sizeRows: sizeRows,
      alwaysHeaderless: sizeRows.length > 0 && sizeRows.every(function (row) {
        return row.alwaysHeaderless;
      }),
    });
  }

  return modules;
}

/**
 * True when every size the module supports has this flag set. The colour
 * preview uses it to decide whether the header or border is dropped across
 * the board.
 */
export function allSizesHidden(module: ThemeModule, map: FlagMap): boolean {
  if (!module.sizeRows || module.sizeRows.length === 0) {
    return false;
  }
  for (let s = 0; s < module.sizeRows.length; s++) {
    const row = module.sizeRows[s];
    if (!flagOn(map, row.value, row.size)) {
      return false;
    }
  }

  return true;
}
