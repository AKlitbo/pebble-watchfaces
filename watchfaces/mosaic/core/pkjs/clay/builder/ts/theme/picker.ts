/**
 * The colour picker popup: the live example panel, the Mono | Vibrant
 * toggle, the 64 colour grid, and the channel stepping arrows. Colours stage
 * inside the popup and only Apply commits them, tapping outside cancels.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { createOverlayHost } from '../../../../../../../../lib/ts/clay/builder/ts/shared/overlay';
import { argbToCss } from './palette';
import { allSizesHidden } from './model';
import { paintSwatch, buildExampleBox } from './preview';
import type { Channel, ChannelKey, ColorMap, ColorRecord, FlagMap, PaletteEntry, ThemeModule } from '../types';

/** What the picker cannot own itself, handed in by the theme builder. */
export interface PickerEnv {
  channels: Channel[];
  palette: PaletteEntry[];
  argbByName: Record<string, number>;
  getColors(): ColorMap;
  getHeaderless(): FlagMap;
  getBorderless(): FlagMap;
  setChannel(moduleValue: number, channelKey: ChannelKey, byte: number | null): void;
}

/** The picker's open/close pair. */
export interface Picker {
  open(module: ThemeModule, channelKey: ChannelKey, swatchEls: Record<string, HTMLElement>): void;
  close(): void;
}

/**
 * Makes the picker with its surroundings wired in.
 *
 * The env carries what the picker cannot own itself: the channels list, the
 * palette and its name lookup, getters for the live colour and flag maps
 * (the map objects get swapped on reset and import, so they are always
 * fetched fresh), and setChannel to commit one channel.
 */
export function createPicker(env: PickerEnv): Picker {
  const host = createOverlayHost('tb-pick-overlay', 'tb-pick', true); // tap outside = cancel

  /**
   * Opens the picker for one module starting on one channel. swatchEls maps
   * channel keys to that row's swatch elements so Apply can repaint them.
   */
  function open(module: ThemeModule, channelKey: ChannelKey, swatchEls: Record<string, HTMLElement>): void {
    const panel = host.open();

    const moduleValue = module.value;
    const committed: Partial<ColorRecord> = env.getColors()[moduleValue] || {};
    // a staged copy of all four channels. the arrows move between them and Apply writes them
    // all at once and Cancel throws them away so the whole panel edits as one with no
    // early commit
    const stagedColors: ColorRecord = {
      accent: committed.accent != null ? committed.accent : null,
      value: committed.value != null ? committed.value : null,
      icon: committed.icon != null ? committed.icon : null,
      subtitle: committed.subtitle != null ? committed.subtitle : null,
    };
    let active = channelKey;

    function labelFor(key: string): string {
      for (let i = 0; i < env.channels.length; i++) {
        if (env.channels[i].key === key) {
          return env.channels[i].label;
        }
      }

      return key;
    }

    // header row: ‹  Editing: X  ›  so you can step through the channels without leaving
    const nav = document.createElement('div');
    nav.className = 'tb-pick-nav';
    const prevButton = document.createElement('button');
    prevButton.type = 'button';
    prevButton.className = 'tb-nav-btn';
    prevButton.textContent = '‹';
    const titleLabel = document.createElement('div');
    titleLabel.className = 'tb-pick-title';
    const nextButton = document.createElement('button');
    nextButton.type = 'button';
    nextButton.className = 'tb-nav-btn';
    nextButton.textContent = '›';
    nav.appendChild(prevButton);
    nav.appendChild(titleLabel);
    nav.appendChild(nextButton);
    panel.appendChild(nav);

    const cells: Array<{ el: HTMLElement; argb: number }> = []; // for each palette cell, so the highlight can move on tap
    // the two ends of the Mono/Vibrant toggle. tracked so refreshHighlight can light the active one
    let monoOption: HTMLElement | null = null;
    let vibrantOption: HTMLElement | null = null;
    // this module's signature colour or null when it has none (then only Mono shows)
    const vibrantByte = module.vibrant != null ? env.argbByName[module.vibrant] : null;

    // this preview is about colours so keep the header and border on as long as any size
    // still shows them (only hide when the module drops them everywhere) so the accent
    // stays visible
    const hideHeader = module.alwaysHeaderless || allSizesHidden(module, env.getHeaderless());
    const hideBorder = allSizesHidden(module, env.getBorderless());
    const example = buildExampleBox(module, stagedColors, hideHeader, hideBorder);
    panel.appendChild(example.box);

    // light up the palette to match the active channel's staged colour
    function refreshHighlight(): void {
      const byte = stagedColors[active];
      if (monoOption) {
        monoOption.classList.toggle('sel', byte == null);
      }
      // a palette colour equal to the vibrant byte lights both the Vibrant option and its
      // grid cell which is fine. they are the same colour
      if (vibrantOption) {
        vibrantOption.classList.toggle('sel', vibrantByte != null && byte === vibrantByte);
      }
      for (let k = 0; k < cells.length; k++) {
        cells[k].el.classList.toggle('sel', byte != null && cells[k].argb === byte);
      }
    }

    // point the picker at a channel: update the label and the box cue and the highlight
    function setActive(key: ChannelKey): void {
      active = key;
      titleLabel.textContent = 'Editing: ' + labelFor(key);
      refreshHighlight();
    }

    // step to the previous/next channel and wrap around
    function step(delta: number): void {
      const order = env.channels.map(function (channel) { return channel.key; });
      const at = order.indexOf(active);
      setActive(order[(at + delta + order.length) % order.length]);
    }
    prevButton.addEventListener('click', function () { step(-1); });
    nextButton.addEventListener('click', function () { step(1); });

    // stage a colour (null = mono) for the active channel: repaint that region with no commit
    function stage(byte: number | null): void {
      stagedColors[active] = byte;
      example.paint(active, byte);
      refreshHighlight();
    }

    // a Mono | Vibrant toggle sits above the palette. Mono drops the channel back to
    // default. Vibrant snaps it to the module's signature colour. the Vibrant half only
    // shows when the module has one so a colourless module still just offers Mono
    const mvRow = document.createElement('div');
    mvRow.className = 'tb-mv';

    monoOption = document.createElement('div');
    monoOption.className = 'tb-mv-opt';
    monoOption.innerHTML = '<span class="tb-mv-ind tb-cell-mono"></span>Mono';
    monoOption.addEventListener('click', function () { stage(null); });
    mvRow.appendChild(monoOption);

    if (vibrantByte != null) {
      vibrantOption = document.createElement('div');
      vibrantOption.className = 'tb-mv-opt';
      const vibrantDot = document.createElement('span');
      vibrantDot.className = 'tb-mv-ind';
      vibrantDot.style.background = argbToCss(vibrantByte);
      vibrantOption.appendChild(vibrantDot);
      vibrantOption.appendChild(document.createTextNode('Vibrant'));
      vibrantOption.addEventListener('click', function () { stage(vibrantByte); });
      mvRow.appendChild(vibrantOption);
    }

    panel.appendChild(mvRow);

    const grid = document.createElement('div');
    grid.className = 'tb-grid';
    for (let index = 0; index < env.palette.length; index++) {
      (function (entry: PaletteEntry) {
        const cell = document.createElement('div');
        cell.className = 'tb-cell';
        cell.style.background = entry.css;
        cell.title = entry.name;
        cell.addEventListener('click', function () { stage(entry.argb); });
        grid.appendChild(cell);
        cells.push({ el: cell, argb: entry.argb });
      })(env.palette[index]);
    }
    panel.appendChild(grid);

    const bar = document.createElement('div');
    bar.className = 'tb-pick-bar';

    const cancelButton = document.createElement('button');
    cancelButton.type = 'button';
    cancelButton.className = 'tb-io-btn';
    cancelButton.textContent = 'Cancel';
    cancelButton.addEventListener('click', host.close);
    bar.appendChild(cancelButton);

    const applyButton = document.createElement('button');
    applyButton.type = 'button';
    applyButton.className = 'tb-io-btn primary';
    applyButton.textContent = 'Apply';
    applyButton.addEventListener('click', function () {
      // commit every channel at once and repaint the row's swatches to match
      env.channels.forEach(function (channel) {
        env.setChannel(moduleValue, channel.key, stagedColors[channel.key]);
        paintSwatch(swatchEls[channel.key], stagedColors[channel.key]);
      });
      host.close();
    });
    bar.appendChild(applyButton);

    panel.appendChild(bar);

    // start on the channel the user tapped to open the picker
    setActive(active);
  }

  return { open: open, close: host.close };
}
