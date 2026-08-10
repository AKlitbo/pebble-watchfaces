/**
 * Builds a Clay custom component the way Clay does, for jsdom specs.
 *
 * A root element comes from the component's template, then set/get/initialize
 * are bound to a context exposing $element, config, and a trigger spy. This is
 * the same wiring Clay itself does inside the config webview, so a spec that
 * drives the mounted component exercises the real user flows.
 */

import { vi } from 'vitest';
import type { Mock } from 'vitest';
import type { RawModule } from '../types';

/** How a Clay custom component reads and writes its wire string, bound to a context. */
interface Manipulator {
  set(this: BoundContext, value: string): void;
  get(this: BoundContext): string;
}

/** A Clay custom component definition, shaped the way its generated .g.js exports it. */
export interface ClayComponentDefinition {
  template: string;
  manipulator: Manipulator;
  initialize(this: BoundContext): void;
}

/** The context Clay binds a component's methods to, the way the config webview does. */
export interface BoundContext {
  $element: HTMLElement[];
  config: Record<string, unknown>;
  trigger: Mock;
  set(value: string): void;
  get(): string;
  initialize(): void;
}

/** The bound context and root element a mounted component exposes to a spec. */
export interface Mounted {
  ctx: BoundContext;
  root: HTMLElement;
  // a spec can stash extra handles (e.g. the rendered rows) on the mount
  [key: string]: unknown;
}

/** Mounts a component and returns the bound context plus its root element. */
export function mount(component: ClayComponentDefinition, config?: Record<string, unknown>): Mounted {
  const holder = document.createElement('div');
  holder.innerHTML = component.template;
  const root = holder.firstChild as HTMLElement;

  const ctx = { $element: [root], config: config || {}, trigger: vi.fn() } as BoundContext;
  ctx.set = component.manipulator.set.bind(ctx);
  ctx.get = component.manipulator.get.bind(ctx);
  ctx.initialize = component.initialize.bind(ctx);

  return { ctx, root };
}

/**
 * A moduleOptions fixture shaped like config.ts builds it, small enough to
 * reason about but covering the interesting shapes. It has a themeRows family
 * with a per row alwaysHeaderless (Digital Clock), a plain multi size module
 * (Battery), a themeHidden module that hides behind another row (Forecast
 * 4-Day), a vibrant colour (Battery) and a 2x4 (Month Grid).
 */
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
