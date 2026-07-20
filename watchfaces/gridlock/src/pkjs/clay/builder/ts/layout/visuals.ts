/**
 * How the layout builder draws a module: the merged module list, looking one
 * up by value, finding its screenshot, and filling a block shaped element
 * with either the real shot or the emoji fallback.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { thumbByLabel } from '../shared/thumbs';
import type { ModuleInfo, RawModule, Thumbs } from '../types';

/**
 * The placeable modules with their icon and colour riding along. config.ts
 * merges those in from module-meta keyed by label, so the mapping survives an
 * id drift. Module 0 (Empty) is dropped since it is not placeable.
 */
export function buildModuleList(rawModules: RawModule[]): ModuleInfo[] {
  return rawModules
    .filter(function (m) {
      return m.value !== 0;
    })
    .map(function (m) {
      return {
        value: m.value,
        label: m.label,
        icon: m.icon || '·',
        color: m.color || '#999',
        sizes: m.sizes || [],
        // themeRows maps a size to the module whose screenshot it shows (the 2x2 digital
        // clock's shot lives under "Time + Date"). thumbFor needs it to find that thumbnail
        themeRows: m.themeRows,
      };
    });
}

/** The module for a value, or a placeholder when the value is unknown. */
export function modInfo(modules: ModuleInfo[], value: number): ModuleInfo {
  for (let i = 0; i < modules.length; i++) {
    if (modules[i].value === value) {
      return modules[i];
    }
  }

  return { value: value, label: 'Unknown', icon: '?', color: '#999', sizes: [] };
}

/**
 * The real panel screenshot for a module at a size, or null to fall back to
 * the emoji block. Thumbnails are keyed by label so the value resolves to its
 * name first, and a themeRows family can file a size's shot under another
 * module's label.
 */
export function thumbFor(thumbs: Thumbs, modules: ModuleInfo[], value: number, key: string): string | null {
  const mod = modInfo(modules, value);
  let label = mod.label;
  if (mod.themeRows) {
    for (let i = 0; i < mod.themeRows.length; i++) {
      if (mod.themeRows[i].size === key && mod.themeRows[i].thumb) {
        label = mod.themeRows[i].thumb as string;
        break;
      }
    }
  }

  return thumbByLabel(thumbs, label, key);
}

/**
 * Fills a block shaped element with either its real panel screenshot or the
 * emoji plus name fallback. Shared by the placed block and the drag ghost,
 * which render the same visual. A full width shot stretches to fill.
 */
export function fillBlockVisual(el: HTMLElement, thumb: string | null, display: Pick<ModuleInfo, 'color' | 'icon' | 'label'>, w: number): void {
  if (thumb) {
    // drop the panel padding and rounding so the real shot fills the cell (the
    // panel already carries its own square border)
    el.style.background = '#000';
    el.style.padding = '0';
    el.style.borderRadius = '0';

    const img = document.createElement('img');
    img.className = 'lb-block-img';
    img.src = thumb;
    // the watch's 1x4 row is a touch taller than a 1x2 so a square-row cell would
    // letterbox it. stretch the full-width shot to fill instead (barely visible)
    if (w === 4) {
      img.style.objectFit = 'fill';
    }
    el.appendChild(img);
  } else {
    el.style.background = display.color;

    const icon = document.createElement('div');
    icon.className = 'lb-icon';
    icon.textContent = display.icon;
    icon.style.color = '#fff';
    el.appendChild(icon);

    if (display.label) {
      const name = document.createElement('div');
      name.className = 'lb-name';
      name.textContent = display.label;
      name.style.color = '#fff';
      el.appendChild(name);
    }
  }
}
