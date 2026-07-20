/**
 * The layout builder's wiring: reads the config Clay hands in, keeps the
 * placed blocks, renders the grid and the tabbed palette, and hooks up the
 * buttons, the drag engine, and the manipulator handles.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize and calls init there with the component as this, so it reaches
 * siblings through the imports below and sticks to browser APIs.
 */

import { GRID_ROWS, GRID_COLS, sizeKey, occupancyGrid } from './geometry';
import { serializeLayout, parseLayoutString } from './codec';
import { LAYOUT_PRESETS } from './presets';
import { buildModuleList, modInfo, thumbFor, fillBlockVisual } from './visuals';
import { createDragEngine } from './drag';
import { createOverlayHost } from '../shared/overlay';
import { buildIoPanel } from '../shared/io-panel';
import { buildSlotsPanel } from './slots';
import type { Block, ClayComponentInstance, ModuleInfo, RawModule, Thumbs } from '../types';

/** The set/get handles init hangs on the root element for the manipulator. */
interface LbHooks {
  _lbSet(value: string): void;
  _lbGet(): string;
}

/**
 * Builds the layout editor into the component's element and hangs the set/get
 * hooks the manipulator reads. Clay serialises this and re-runs it inside the
 * config webview, so everything it needs has to be inlined by the generator.
 */
export function init(this: ClayComponentInstance): void {
  // eslint-disable-next-line @typescript-eslint/no-this-alias
  const self = this;
  const root = self.$element[0] as HTMLElement & LbHooks;
  const cfg = self.config || {};
  const rawModules: RawModule[] = cfg.moduleOptions || [];
  const THUMBS: Thumbs = cfg.moduleThumbnails || {}; // value -> { sizeKey -> data url } of real panel shots

  const hidden = root.querySelector('.lb-value') as HTMLInputElement;
  const gridEl = root.querySelector('.lb-grid') as HTMLElement;
  const paletteContainer = root.querySelector('.lb-palette-container') as HTMLElement;

  let blocks: Block[] = [];

  const MODULES = buildModuleList(rawModules);

  function render(): void {
    gridEl.innerHTML = '';
    const grid = occupancyGrid(blocks);

    for (let r = 0; r < GRID_ROWS; r++) {
      for (let c = 0; c < GRID_COLS; c++) {
        const cell = document.createElement('div');
        cell.className = 'lb-cell' + (grid[r][c] !== null ? ' occupied' : '');
        cell.style.gridRow = String(r + 1);
        cell.style.gridColumn = String(c + 1);
        cell.dataset.row = String(r);
        cell.dataset.col = String(c);
        gridEl.appendChild(cell);
      }
    }

    for (let i = 0; i < blocks.length; i++) {
      (function (idx) {
        const b = blocks[idx];
        const info = modInfo(MODULES, b.module);
        const el = document.createElement('div');
        el.className = 'lb-block' + (b.h >= 2 ? ' big' : '');
        el.style.gridRow = b.row + 1 + ' / span ' + b.h;
        el.style.gridColumn = b.col + 1 + ' / span ' + b.w;

        fillBlockVisual(el, thumbFor(THUMBS, MODULES, b.module, sizeKey(b.w, b.h) || ''), info, b.w);

        el.addEventListener('pointerdown', function (e) {
          engine.armBlockDrag(idx, e as PointerEvent);
        });

        gridEl.appendChild(el);
      })(i);
    }

    hidden.value = serializeLayout(blocks);
    self.trigger('change');

    const placedModules = blocks.map(function (b) {
      return b.module;
    });
    const palItems = root.querySelectorAll<HTMLElement>('.lb-pal-item');
    for (let k = 0; k < palItems.length; k++) {
      const modVal = parseInt(palItems[k].dataset.value || '', 10);
      if (placedModules.indexOf(modVal) !== -1) {
        palItems[k].classList.add('placed');
      } else {
        palItems[k].classList.remove('placed');
      }
    }
  }

  const engine = createDragEngine({
    gridEl: gridEl,
    getBlocks: function () {
      return blocks;
    },
    render: render,
    modInfo: function (value): ModuleInfo {
      return modInfo(MODULES, value);
    },
    thumbFor: function (value, key) {
      return thumbFor(THUMBS, MODULES, value, key);
    },
  });

  // tabbed palette: one tab per size group. only the active group's modules show
  const sizeGroups = [
    { key: '1x2', title: '1x2' },
    { key: '1x4', title: '1x4' },
    { key: '2x2', title: '2x2' },
    { key: '2x4', title: '2x4' },
  ];

  const drawerWrap = document.createElement('div');
  drawerWrap.className = 'lb-drawer-wrap';

  const tabBar = document.createElement('div');
  tabBar.className = 'lb-tabs';

  const panes = document.createElement('div');
  panes.className = 'lb-drawer-panes';

  function activateGroup(tab: HTMLElement, pane: HTMLElement): void {
    const allTabs = tabBar.querySelectorAll('.lb-tab');
    for (let i = 0; i < allTabs.length; i++) {
      allTabs[i].classList.remove('active');
    }
    const allPanes = panes.querySelectorAll('.lb-drawer-pane');
    for (let j = 0; j < allPanes.length; j++) {
      allPanes[j].classList.remove('active');
    }
    tab.classList.add('active');
    pane.classList.add('active');
  }

  let firstGroup = true;

  sizeGroups.forEach(function (sg) {
    const groupMods = MODULES.filter(function (m) {
      return m.sizes.indexOf(sg.key) !== -1;
    });
    if (groupMods.length === 0) {
      return;
    }

    const tab = document.createElement('button');
    tab.type = 'button';
    tab.className = 'lb-tab';
    tab.textContent = sg.title;

    const pane = document.createElement('div');
    pane.className = 'lb-drawer-pane';

    const drawer = document.createElement('div');
    drawer.className = 'lb-drawer';

    groupMods.forEach(function (m) {
      const el = document.createElement('div');
      el.className = 'lb-pal-item';
      el.dataset.value = String(m.value);
      const palThumb = thumbFor(THUMBS, MODULES, m.value, sg.key);
      if (palThumb) {
        el.style.background = '#000';
        el.innerHTML = '<img class="lb-pal-img" src="' + palThumb + '">';
      } else {
        el.style.background = m.color;
        el.innerHTML = '<div class="lb-pal-icon">' + m.icon + '</div>';
      }

      el.addEventListener('pointerdown', function (e) {
        engine.startDragFromPalette(m, sg.key, e as PointerEvent);
      });
      drawer.appendChild(el);
    });

    pane.appendChild(drawer);

    tab.addEventListener('click', function () {
      activateGroup(tab, pane);
    });

    if (firstGroup) {
      tab.classList.add('active');
      pane.classList.add('active');
      firstGroup = false;
    }

    tabBar.appendChild(tab);
    panes.appendChild(pane);
  });

  drawerWrap.appendChild(tabBar);
  drawerWrap.appendChild(panes);
  paletteContainer.appendChild(drawerWrap);

  (root.querySelector('.lb-btn-clear') as HTMLElement).addEventListener('click', function () {
    blocks = [];
    render();
  });

  const presetBtns = root.querySelectorAll<HTMLElement>('.lb-preset');
  for (let i = 0; i < presetBtns.length; i++) {
    presetBtns[i].addEventListener('click', function (e) {
      const id = (e.target as HTMLElement).getAttribute('data-preset') || '';
      if (LAYOUT_PRESETS[id]) {
        blocks = parseLayoutString(LAYOUT_PRESETS[id]);
        render();
      }
    });
  }

  const overlay = createOverlayHost('lb-overlay', 'lb-panel', true);

  const ioBtn = root.querySelector('.lb-btn-io');
  if (ioBtn) {
    ioBtn.addEventListener('click', function () {
      const panel = overlay.open();
      buildIoPanel(panel, {
        title: 'Import / Export Layout',
        css: { title: 'lb-title', textarea: 'lb-io-textarea', buttons: 'lb-io-btns', button: 'lb-io-btn' },
        value: serializeLayout(blocks),
        copyResetMs: 2000,
        onApply: function (text) {
          blocks = parseLayoutString(text);
          render();
          overlay.close();
        },
      });
    });
  }

  const slotsBtn = root.querySelector('.lb-btn-slots');
  if (slotsBtn) {
    slotsBtn.addEventListener('click', function () {
      const panel = overlay.open();
      buildSlotsPanel(panel, {
        getCurrent: function () {
          return serializeLayout(blocks);
        },
        onLoad: function (text) {
          blocks = parseLayoutString(text);
          render();
          overlay.close();
        },
      });
    });
  }

  // expose set/get to the manipulator
  root._lbSet = function (value) {
    blocks = parseLayoutString(value);
    render();
  };
  root._lbGet = function () {
    return serializeLayout(blocks);
  };

  root._lbSet(hidden.value || '');
}
