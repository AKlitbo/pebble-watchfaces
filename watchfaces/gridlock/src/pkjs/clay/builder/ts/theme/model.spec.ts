/**
 * Specs for the appearance editor's module model.
 *
 * The themeRows and themeHidden metadata decide which panels share a colour
 * row and which panel each flag toggle actually targets, exactly the wiring
 * a refactor of config.ts metadata could quietly break.
 */

import { describe, test, expect } from 'vitest';
import { buildThemeModules, sizeRowsFor, buildOptionByLabel, allSizesHidden } from './model';
import { setFlag } from './codec';
import { moduleOptionsFixture } from '../testing/harness';

const modules = buildThemeModules(moduleOptionsFixture);

describe('buildThemeModules', () => {
  /** A themeHidden module getting its own row would show duplicate swatches that fight each other. */
  test('skips Empty and the themeHidden modules', () => {
    const values = modules.map((module) => module.value);

    expect(values).toEqual([1, 2, 3, 6, 25, 49]);
  });

  /** Losing the real label would break every thumbnail lookup for a renamed row. */
  test('themeLabel renames the row but thumbLabel keeps the real label', () => {
    const forecast = modules.find((module) => module.value === 25);

    expect(forecast.label).toBe('Hourly/Daily Forecast');
    expect(forecast.thumbLabel).toBe('Forecast (Hourly)');
  });

  /** A module counted headerless while one size still has a header would drop that header in the preview. */
  test('a module is alwaysHeaderless only when every size row is', () => {
    const clock = modules.find((module) => module.value === 1);

    expect(clock.sizeRows.map((row) => row.alwaysHeaderless)).toEqual([true, false]);
    expect(clock.alwaysHeaderless).toBe(false);
  });
});

describe('sizeRowsFor', () => {
  /** A flag toggle keyed to the family id instead of the panel's own id would toggle the wrong panel. */
  test('a themeRows sub row targets the panel it pictures', () => {
    const optionByLabel = buildOptionByLabel(moduleOptionsFixture);
    const forecast = moduleOptionsFixture.find((option) => option.value === 25);

    const result = sizeRowsFor(forecast, optionByLabel);

    expect(result).toEqual([
      { size: '1x4', thumbLabel: 'Forecast (Hourly)', value: 25, alwaysHeaderless: false },
      { size: '2x4', thumbLabel: 'Forecast (4-Day)', value: 26, alwaysHeaderless: false },
    ]);
  });

  /** Without themeRows every placeable size still needs a row or those panels lose their toggles. */
  test('a plain module gets one row per size', () => {
    const battery = moduleOptionsFixture.find((option) => option.value === 2);

    const result = sizeRowsFor(battery, {});

    expect(result).toEqual([
      { size: '1x2', thumbLabel: 'Battery', value: 2, alwaysHeaderless: false },
      { size: '2x2', thumbLabel: 'Battery', value: 2, alwaysHeaderless: false },
    ]);
  });
});

describe('allSizesHidden', () => {
  /** The preview dropping its header while one size still shows one would hide the accent colour. */
  test('is true only when every size row has the flag', () => {
    const battery = modules.find((module) => module.value === 2);
    const map = {};

    setFlag(map, 2, '1x2', true);
    const partial = allSizesHidden(battery, map);
    setFlag(map, 2, '2x2', true);
    const full = allSizesHidden(battery, map);

    expect(partial).toBe(false);
    expect(full).toBe(true);
  });
});
