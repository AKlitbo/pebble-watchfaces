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
