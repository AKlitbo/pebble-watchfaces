/**
 * The moduleOptions fixture the mosaic builder specs mount against.
 *
 * Shaped like config.ts builds it, small enough to reason about but covering the interesting
 * shapes. It has a themeRows family with a per row alwaysHeaderless (Digital Clock), a plain
 * multi size module (Battery), a themeHidden module that hides behind another row (Forecast
 * 4-Day), a vibrant colour (Battery) and a 2x4 (Month Grid).
 *
 * Beside the family rather than in lib with the mount harness: every field here is grid and
 * panel vocabulary, which is the line lib does not cross.
 */

import type { RawModule } from '../types';

export const moduleOptionsFixture: RawModule[] = [
  { label: 'Empty', value: 0, sizes: [] },
  {
    label: 'Digital Clock',
    value: 1,
    sizes: ['1x4', '2x2'],
    icon: '🕐',
    color: '#37474f',
    themeRows: [
      { size: '1x4', thumb: 'Digital Clock', alwaysHeaderless: true },
      { size: '2x2', thumb: 'Time + Date' },
    ],
  },
  { label: 'Battery', value: 2, sizes: ['1x2', '2x2'], icon: '🔋', color: '#4caf50', vibrant: 'Green' },
  { label: 'Weather', value: 3, sizes: ['1x2', '2x2'], icon: '⛅', color: '#2196f3' },
  { label: 'Steps', value: 6, sizes: ['1x2', '2x2'], icon: '👟', color: '#ff9800' },
  {
    label: 'Forecast (Hourly)',
    value: 25,
    sizes: ['1x4', '2x4'],
    icon: '🌦️',
    color: '#607d8b',
    themeLabel: 'Hourly/Daily Forecast',
    themeRows: [
      { size: '1x4', thumb: 'Forecast (Hourly)' },
      { size: '2x4', thumb: 'Forecast (4-Day)' },
    ],
  },
  { label: 'Forecast (4-Day)', value: 26, sizes: ['1x4', '2x4'], icon: '📅', color: '#607d8b', themeHidden: true },
  { label: 'Month Grid', value: 49, sizes: ['2x4'], icon: '🗓️', color: '#795548' },
];
