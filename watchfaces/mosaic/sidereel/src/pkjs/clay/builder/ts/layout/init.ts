/**
 * The layout builder's wiring: reads the config Clay hands in, keeps the
 * placed blocks, renders the grid and the tabbed palette, and hooks up the
 * buttons, the drag engine, and the manipulator handles.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize and calls init there with the component as this, so it reaches
 * siblings through the imports below and sticks to browser APIs.
 */

import { GRID_ROWS, GRID_COLS, REEL_COL, isBlocked, sizeKey, canPlace, snapDrop, occupancyGrid } from './geometry';
import { serializeLayout, parseLayoutString, EMPTY_LAYOUT } from './codec';
import { LAYOUT_PRESETS } from './presets';
import { buildModuleList, modInfo, thumbFor, fillBlockVisual } from '../../../../../../../core/pkjs/clay/builder/ts/layout/visuals';
import { createDragEngine } from '../../../../../../../core/pkjs/clay/builder/ts/layout/drag';
import { createOverlayHost } from '../../../../../../../../../lib/ts/clay/builder/ts/shared/overlay';
import { buildIoPanel } from '../../../../../../../../../lib/ts/clay/builder/ts/shared/io-panel';
import { buildModesBar, readLibrary, seedLibrary, writeLibrary, storePresent, NIGHT_NONE } from '../../../../../../../core/pkjs/clay/builder/ts/layout/modes';
import type { ModesBar } from '../../../../../../../core/pkjs/clay/builder/ts/layout/modes';
import type { Block, ClayComponentInstance, ModuleInfo, RawModule, Thumbs } from '../../../../../../../core/pkjs/clay/builder/ts/types';

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
  const modesHost = root.querySelector('.lb-modes') as HTMLElement;

  let blocks: Block[] = [];

  // the four layouts and the day/night assignments. seeded from whatever the watch already had,
  // so an upgrade finds its existing design sitting in layout 1
  const library = seedLibrary(readLibrary(), hidden.value || '');

  // whether the library above came from the page or is just the empty default. Clay builds page
  // items in order and attaches each only after setting it, so if this component is ever built
  // before its stores the read above finds nothing — and writing that back would wipe four saved
  // layouts on the first edit. so: no writing until a read has actually worked
  let loaded = storePresent();

  /**
   * The only place the library is written, so the guard above cannot be routed around.
   *
   * Everything that changes the library — dragging, the presets, an import, a tab switch, an
   * assignment — comes through here. Before the store has been read, a write would be four blank
   * grids landing on top of the user's real ones.
   */
  function save(): void {
    if (loaded) {
      writeLibrary(library);
    }
  }

  /**
   * Push both wire values out: the day layout into the hidden input the manipulator reads, and
   * the night one into the page item the watch takes LAYOUT_NIGHT from.
   *
   * Called on every render whichever layout is being edited, because the grid on screen is only
   * one of four and the two that ship are whichever the assignments name. Writing only the
   * selected one would publish the wrong layout the moment you tabbed away from the day grid.
   */
  function publish(): void {
    library.layouts[selected()] = serializeLayout(blocks);
    save();

    hidden.value = library.layouts[library.day] || EMPTY_LAYOUT;
    self.trigger('change');

    const nightInput = document.querySelector('.gl-night') as HTMLInputElement | null;
    if (nightInput) {
      const value = library.night === NIGHT_NONE
        ? EMPTY_LAYOUT
        : library.layouts[library.night] || EMPTY_LAYOUT;
      // only when it moved, so dragging around the day grid does not spam Clay with saves
      if (nightInput.value !== value) {
        nightInput.value = value;
        nightInput.dispatchEvent(new Event('change'));
      }
    }
  }

  // the modes bar owns which layout is being edited. asking it, rather than keeping a second copy
  // here, is what stops the two drifting apart and writing an edit into the wrong layout
  let modes: ModesBar | null = null;
  function selected(): number {
    return modes ? modes.selected() : library.day;
  }

  const MODULES = buildModuleList(rawModules);

  function render(): void {
    gridEl.innerHTML = '';
    const grid = occupancyGrid(blocks);

    for (let r = 0; r < GRID_ROWS; r++) {
      for (let c = 0; c < GRID_COLS; c++) {
        const cell = document.createElement('div');
        // the parts of the watch the face draws itself are shown but never droppable, so the
        // grid reads as the whole screen instead of just the corner panels can go in
        let blocked = '';
        if (isBlocked(r, c)) {
          blocked = c >= REEL_COL ? ' blocked reel' : ' blocked pointer';
        }
        cell.className = 'lb-cell' + blocked + (grid[r][c] !== null ? ' occupied' : '');
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

    publish();

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
    // the grid rules are this face's, so the shared engine is told them rather than importing them
    geometry: {
      rows: GRID_ROWS,
      cols: GRID_COLS,
      sizeKey: sizeKey,
      canPlace: canPlace,
      snapDrop: snapDrop,
    },
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

  if (modesHost) {
    modes = buildModesBar(modesHost, library, {
      getCurrent: function () {
        return serializeLayout(blocks);
      },
      onSelect: function (layout) {
        blocks = parseLayoutString(layout);
        render();
      },
      onAssign: function () {
        publish();
      },
      save: save,
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

  // the manipulator speaks the day layout only, which is what LAYOUT means. setting it writes
  // into whichever layout is assigned to day rather than whatever happens to be on screen
  root._lbSet = function (value) {
    library.layouts[library.day] = value || EMPTY_LAYOUT;
    // only follow it onto the grid while the day layout is the one being edited. Clay seeds this
    // at open, and yanking the user off the tab they were on would be its own small betrayal
    if (selected() === library.day) {
      blocks = parseLayoutString(library.layouts[library.day]);
      render();
    } else {
      publish();
    }
  };
  root._lbGet = function () {
    return library.layouts[library.day] || EMPTY_LAYOUT;
  };

  blocks = parseLayoutString(library.layouts[library.day]);
  render();

  // and once the page has finished building, pick up anything that arrived late. this is the
  // safety net for the ordering above: if the stores were not there at initialize, they are now
  if (!loaded) {
    setTimeout(function () {
      if (!storePresent()) {
        return; // no store on this page at all, so there is nothing to lose
      }

      const saved = readLibrary();
      if (saved.layouts.some(function (layout) { return layout !== EMPTY_LAYOUT; })) {
        library.layouts = saved.layouts;
        library.day = saved.day;
        library.night = saved.night;
        blocks = parseLayoutString(library.layouts[library.day]);
        if (modes) {
          modes.refresh();
        }
      }

      loaded = true;
      render();
    }, 0);
  }
}
