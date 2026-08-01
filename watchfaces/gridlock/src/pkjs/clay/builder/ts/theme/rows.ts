/**
 * The editor list rows: one row per module with its shared colour swatches,
 * then one sub row per size with its screenshot and its header/border
 * toggles.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { thumbByLabel } from '../shared/thumbs';
import { flagOn, setFlag } from './codec';
import { paintSwatch } from './preview';
import type { Channel, ChannelKey, ColorMap, FlagMap, SizeRow, ThemeModule, Thumbs } from '../types';

/** What the row builder cannot own itself, handed in by the theme builder. */
export interface RowEnv {
  thumbs: Thumbs;
  channels: Channel[];
  getColors(): ColorMap;
  getHeaderless(): FlagMap;
  getBorderless(): FlagMap;
  persist(): void;
  openPicker(module: ThemeModule, channelKey: ChannelKey, swatchEls: Record<string, HTMLElement>): void;
}

/**
 * Makes the row builder with its surroundings wired in.
 *
 * The env carries the thumbnails, the channels list, getters for the live
 * colour and flag maps (the map objects get swapped on reset and import, so
 * they are always fetched fresh), persist() to write the wire string after a
 * toggle, and openPicker to hand a tapped swatch to the colour picker.
 */
export function createRowBuilder(env: RowEnv): { buildRow(module: ThemeModule): HTMLElement } {
  // one small square toggling a single flag (header or border) for one module at one
  // size. the letter says which flag and the colour says the state: green shown / grey hidden
  function buildFlagToggle(getMap: () => FlagMap, flagId: number, size: string, letter: string): HTMLElement {
    const toggle = document.createElement('div');
    toggle.className = 'tb-toggle';
    toggle.title = (letter === 'H' ? 'Header' : 'Border') + ' ' + size;
    const paint = function () {
      const shown = !flagOn(getMap(), flagId, size);
      toggle.textContent = letter;
      // signal green shown / muted graphite hidden
      toggle.style.background = shown ? '#7bd88f' : '#23262c';
      toggle.style.color = shown ? '#08210f' : '#666c76';
    };
    paint();
    toggle.addEventListener('click', function () {
      setFlag(getMap(), flagId, size, !flagOn(getMap(), flagId, size));
      paint();
      env.persist();
    });

    return toggle;
  }

  // one size sub row: this size's real panel shot in a fixed box then its label then its H/B
  // toggles. row is {size, thumbLabel, value, alwaysHeaderless} so a shared theme family
  // shows the right panel's shot per size and the always headerless panels (the digital
  // clock bar) have no header to hide so only get B
  function buildSizeRow(row: SizeRow): HTMLElement {
    const size = row.size;
    const sizeRow = document.createElement('div');
    sizeRow.className = 'sz';

    const shot = document.createElement('div');
    shot.className = 'sz-shot';
    const src = thumbByLabel(env.thumbs, row.thumbLabel, size);
    if (src) {
      const shotImg = document.createElement('img');
      shotImg.src = src;
      shotImg.alt = row.thumbLabel + ' ' + size;
      shot.appendChild(shotImg);
    } else {
      const noshot = document.createElement('div');
      noshot.className = 'sz-noshot';
      noshot.textContent = 'no shot';
      shot.appendChild(noshot);
    }
    sizeRow.appendChild(shot);

    const label = document.createElement('div');
    label.className = 'sz-lbl';
    label.textContent = size;
    sizeRow.appendChild(label);

    const toggles = document.createElement('div');
    toggles.className = 'sz-tog';
    // flags target the panel this sub row represents (row.value)
    if (!row.alwaysHeaderless) {
      toggles.appendChild(buildFlagToggle(env.getHeaderless, row.value, size, 'H'));
    }
    toggles.appendChild(buildFlagToggle(env.getBorderless, row.value, size, 'B'));
    sizeRow.appendChild(toggles);

    return sizeRow;
  }

  function buildRow(module: ThemeModule): HTMLElement {
    const row = document.createElement('div');
    row.className = 'tb-row';

    // the parent line: an icon then the name then the colours shared across every size
    const head = document.createElement('div');
    head.className = 'tb-row-head';

    const icon = document.createElement('div');
    icon.className = 'tb-row-ic';
    icon.textContent = module.icon;
    head.appendChild(icon);

    const name = document.createElement('div');
    name.className = 'tb-row-nm';
    name.textContent = module.label;
    head.appendChild(name);

    // one shared set of colour swatches for the whole module family
    const cols = document.createElement('div');
    cols.className = 'tb-chips';
    const swatchEls: Record<string, HTMLElement> = {};
    for (let index = 0; index < env.channels.length; index++) {
      (function (channel: Channel) {
        const chip = document.createElement('div');
        chip.className = 'tb-chip';

        const label = document.createElement('div');
        label.className = 'tb-chip-lbl';
        label.textContent = channel.label;
        chip.appendChild(label);

        const swatch = document.createElement('div');
        swatch.className = 'tb-swatch';
        const stored = env.getColors()[module.value] ? env.getColors()[module.value][channel.key] : null;
        paintSwatch(swatch, stored);
        swatch.addEventListener('click', function () {
          env.openPicker(module, channel.key, swatchEls);
        });
        chip.appendChild(swatch);
        swatchEls[channel.key] = swatch;

        cols.appendChild(chip);
      })(env.channels[index]);
    }
    head.appendChild(cols);
    row.appendChild(head);

    // one sub row per size the module (or its shared theme family) covers
    const sizeList = document.createElement('div');
    sizeList.className = 'sz-list';
    for (let sizeIndex = 0; sizeIndex < module.sizeRows.length; sizeIndex++) {
      sizeList.appendChild(buildSizeRow(module.sizeRows[sizeIndex]));
    }
    row.appendChild(sizeList);

    return row;
  }

  return { buildRow: buildRow };
}
