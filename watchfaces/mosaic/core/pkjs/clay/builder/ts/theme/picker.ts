/**
 * The colour picker popup: the live example panel, the Mono | Vibrant
 * toggle, the 64 colour grid, and the channel stepping arrows. Colours stage
 * inside the popup and only Apply commits them, tapping outside cancels.
 * It opens on one module's row, or on every panel at once from the bulk bar.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { createOverlayHost } from '../../../../../../paf/ts/clay/builder/ts/shared/overlay';
import { argbToCss } from './palette';
import { allSizesHidden, vibrantByteFor } from './model';
import { paintSwatch, buildExampleBox } from './preview';
import type {
  Channel,
  ChannelKey,
  ChannelPick,
  ChannelPicks,
  ColorMap,
  ColorRecord,
  FlagMap,
  PaletteEntry,
  ThemeModule,
} from '../types';

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

/** The picker's open and close handles. */
export interface Picker {
  open(module: ThemeModule, channelKey: ChannelKey, swatchEls: Record<string, HTMLElement>): void;
  openAll(modules: ThemeModule[], onApply: (picks: ChannelPicks) => void): void;
  close(): void;
}

// what one opening of the picker needs, so the one module and every panel modes share the popup
interface Session {
  // the colours the popup starts staged on
  staged: ColorRecord;
  startOn: ChannelKey;
  // the title line reads "<titlePrefix>: Accent"
  titlePrefix: string;
  previewLabel: string;
  hideHeader: boolean;
  hideBorder: boolean;
  // the colour the Vibrant option previews, or null to leave the option out
  vibrantByte: number | null;
  // the Vibrant option's dot, as a css background
  vibrantDot: string;
  // every panel mode stages 'vibrant' rather than one byte, and lights nothing on a channel it
  // has not touched, since each panel keeps its own colour there
  everyPanel: boolean;
  commit(staged: ColorRecord, picks: ChannelPicks): void;
}

/**
 * Makes the picker with its surroundings wired in.
 *
 * The env carries what the picker cannot own itself: the channels list, the
 * palette and its name lookup, getters for the live colour and flag maps
 * (the map objects get swapped on reset and import, so they are always
 * fetched fresh), and setChannel to commit one channel.
 *
 * @param env What the picker reads and writes through.
 * @return The open and close handles.
 */
export function createPicker(env: PickerEnv): Picker {
  const host = createOverlayHost('tb-pick-overlay', 'tb-pick', true); // tap outside = cancel

  // builds the popup for one session: the example box, the Mono | Vibrant toggle, the palette
  // grid, and the Cancel | Apply bar
  function openSession(session: Session): void {
    const panel = host.open();
    const stagedColors = session.staged;
    // the channels touched since the popup opened, and what each was set to
    const picks: ChannelPicks = {};
    let active = session.startOn;

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
    const vibrantByte = session.vibrantByte;
    const example = buildExampleBox(
      { label: session.previewLabel },
      stagedColors,
      session.hideHeader,
      session.hideBorder
    );

    panel.appendChild(example.box);

    // light up the palette to match the active channel's staged colour. in every panel mode an
    // untouched channel lights nothing, since it is not one colour across the panels
    function refreshHighlight(): void {
      const current: ChannelPick | undefined = session.everyPanel ? picks[active] : stagedColors[active];

      if (monoOption) {
        monoOption.classList.toggle('sel', current === null);
      }

      // a palette colour equal to the vibrant byte lights both the Vibrant option and its
      // grid cell which is fine. they are the same colour
      if (vibrantOption) {
        vibrantOption.classList.toggle(
          'sel',
          session.everyPanel ? current === 'vibrant' : vibrantByte != null && current === vibrantByte
        );
      }

      for (let k = 0; k < cells.length; k++) {
        cells[k].el.classList.toggle('sel', typeof current === 'number' && cells[k].argb === current);
      }
    }

    // point the picker at a channel: update the label and the box cue and the highlight
    function setActive(key: ChannelKey): void {
      active = key;
      titleLabel.textContent = session.titlePrefix + ': ' + labelFor(key);
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

    // stage a colour (null = mono) for the active channel: repaint that region with no commit.
    // pick is what Apply hands on, and only differs from byte for Vibrant in every panel mode
    function stage(byte: number | null, pick: ChannelPick): void {
      stagedColors[active] = byte;
      picks[active] = pick;
      example.paint(active, byte);
      refreshHighlight();
    }

    // a Mono | Vibrant toggle sits above the palette. Mono drops the channel back to
    // default. Vibrant snaps it to the module's signature colour, or each panel's own in
    // every panel mode. the Vibrant half only shows when there is one so a colourless
    // module still just offers Mono
    const mvRow = document.createElement('div');

    mvRow.className = 'tb-mv';

    monoOption = document.createElement('div');
    monoOption.className = 'tb-mv-opt';
    monoOption.innerHTML = '<span class="tb-mv-ind tb-cell-mono"></span>Mono';
    monoOption.addEventListener('click', function () { stage(null, null); });
    mvRow.appendChild(monoOption);

    if (vibrantByte != null) {
      vibrantOption = document.createElement('div');
      vibrantOption.className = 'tb-mv-opt';
      const vibrantDot = document.createElement('span');

      vibrantDot.className = 'tb-mv-ind';
      vibrantDot.style.background = session.vibrantDot;
      vibrantOption.appendChild(vibrantDot);
      vibrantOption.appendChild(document.createTextNode('Vibrant'));
      vibrantOption.addEventListener('click', function () {
        stage(vibrantByte, session.everyPanel ? 'vibrant' : vibrantByte);
      });
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
        cell.addEventListener('click', function () { stage(entry.argb, entry.argb); });
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
      session.commit(stagedColors, picks);
      host.close();
    });
    bar.appendChild(applyButton);

    panel.appendChild(bar);

    // start on the channel the user tapped to open the picker
    setActive(active);
  }

  /**
   * Opens the picker for one module starting on one channel. swatchEls maps
   * channel keys to that row's swatch elements so Apply can repaint them.
   */
  function open(module: ThemeModule, channelKey: ChannelKey, swatchEls: Record<string, HTMLElement>): void {
    const committed: Partial<ColorRecord> = env.getColors()[module.value] || {};
    // this module's signature colour or null when it has none (then only Mono shows)
    const vibrantByte = vibrantByteFor(module, env.argbByName);

    openSession({
      // a staged copy of all four channels. the arrows move between them and Apply writes them
      // all at once and Cancel throws them away so the whole panel edits as one with no
      // early commit
      staged: {
        accent: committed.accent != null ? committed.accent : null,
        value: committed.value != null ? committed.value : null,
        icon: committed.icon != null ? committed.icon : null,
        subtitle: committed.subtitle != null ? committed.subtitle : null,
      },
      startOn: channelKey,
      titlePrefix: 'Editing',
      previewLabel: module.label,
      // this preview is about colours so keep the header and border on as long as any size
      // still shows them (only hide when the module drops them everywhere) so the accent
      // stays visible
      hideHeader: module.alwaysHeaderless || allSizesHidden(module, env.getHeaderless()),
      hideBorder: allSizesHidden(module, env.getBorderless()),
      vibrantByte: vibrantByte,
      vibrantDot: vibrantByte != null ? argbToCss(vibrantByte) : '',
      everyPanel: false,
      commit: function (staged) {
        // commit every channel at once and repaint the row's swatches to match
        env.channels.forEach(function (channel) {
          env.setChannel(module.value, channel.key, staged[channel.key]);
          paintSwatch(swatchEls[channel.key], staged[channel.key]);
        });
      },
    });
  }

  /**
   * Opens the picker for every panel at once, starting on Accent. Only the
   * channels touched reach onApply, so the rest keep each panel's own colour.
   */
  function openAll(modules: ThemeModule[], onApply: (picks: ChannelPicks) => void): void {
    // Vibrant means a different colour on each panel, so its dot blends the first few signature
    // colours and the example shows the first one
    const vibrantCss: string[] = [];
    let firstVibrant: number | null = null;

    for (let index = 0; index < modules.length && vibrantCss.length < 4; index++) {
      const byte = vibrantByteFor(modules[index], env.argbByName);

      if (byte == null) {
        continue;
      }

      if (firstVibrant == null) {
        firstVibrant = byte;
      }

      if (vibrantCss.indexOf(argbToCss(byte)) < 0) {
        vibrantCss.push(argbToCss(byte));
      }
    }

    openSession({
      // the example starts plain and a channel shows its colour once it has been picked
      staged: { accent: null, value: null, icon: null, subtitle: null },
      startOn: 'accent',
      titlePrefix: 'All Panels',
      previewLabel: 'All Panels',
      hideHeader: false,
      hideBorder: false,
      vibrantByte: firstVibrant,
      vibrantDot: vibrantCss.length > 1
        ? 'linear-gradient(90deg, ' + vibrantCss.join(', ') + ')'
        : vibrantCss[0] || '',
      everyPanel: true,
      commit: function (_staged, picks) {
        onApply(picks);
      },
    });
  }

  return { open: open, openAll: openAll, close: host.close };
}
