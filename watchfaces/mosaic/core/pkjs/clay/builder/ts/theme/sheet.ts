/**
 * The full screen editor sheet: the legend, the bulk sweep bars, the module
 * list, the bottom Reset / Import-Export / Done bar, and the import/export
 * popup.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { createOverlayHost } from '../../../../../../paf/ts/clay/builder/ts/shared/overlay';
import { buildIoPanel } from '../../../../../../paf/ts/clay/builder/ts/shared/io-panel';
import { showImportNote } from '../shared/import-note';
import { flagOn, setFlag } from './codec';
import type { FlagMap, ThemeModule } from '../types';

/** What the sheet cannot own itself, handed in by the theme builder. */
export interface SheetEnv {
  modules: ThemeModule[];
  buildRow(module: ThemeModule): HTMLElement;
  getHeaderless(): FlagMap;
  getBorderless(): FlagMap;
  serialize(): string;
  /** Applies an imported string, or refuses text that holds nothing and reports false. */
  applyImport(text: string): boolean;
  /** Puts back a string serialize gave earlier, which Undo after Reset uses. */
  restore(text: string): void;
  /** Calls the listener whenever the table is written, so the sheet can drop its Undo. */
  watchEdits(listener: () => void): void;
  resetAll(): void;
  persist(): void;
  openColourAll(onApplied: () => void): void;
  picker: { close(): void };
  onDone(): void;
}

/**
 * Makes the editor sheet with its surroundings wired in.
 *
 * The env carries the module list and row builder, getters for the live flag
 * maps (the map objects get swapped on reset and import, so they are always
 * fetched fresh), the wire string in and out (serialize and applyImport),
 * resetAll to blank the whole table, persist() to write the string after a
 * sweep, openColourAll to open the every panel picker, the picker so closing
 * the sheet also drops an open picker, and onDone for the close handshake
 * back to Clay.
 */
export function createSheet(env: SheetEnv): { open(): void } {
  const sheetHost = createOverlayHost('tb-overlay', 'tb-sheet', false);
  const ioHost = createOverlayHost('tb-pick-overlay', 'tb-pick', true);
  let editorList: HTMLElement | null = null;

  // the bulk buttons' label painters. Reset, Undo, and an import change the flags under them, so
  // the list redraw repaints them too
  let bulkPainters: Array<() => void> = [];

  // (re)fill the module list in place. used on open and reset and import
  function renderList(): void {
    if (!editorList) {
      return;
    }

    while (editorList.firstChild) {
      editorList.removeChild(editorList.firstChild);
    }

    for (let index = 0; index < env.modules.length; index++) {
      editorList.appendChild(env.buildRow(env.modules[index]));
    }

    for (let index = 0; index < bulkPainters.length; index++) {
      bulkPainters[index]();
    }
  }

  // share a colour string with someone else or paste one in
  function openIO(): void {
    const panel = ioHost.open();

    buildIoPanel(panel, {
      title: 'Import / Export Colours',
      css: { title: 'tb-pick-title', textarea: 'tb-io-textarea', buttons: 'tb-io-btns', button: 'tb-io-btn' },
      value: env.serialize(),
      copyResetMs: 1500,
      onApply: function (text) {
        // a stray paste can read as no colours at all, and applying it would wipe every panel.
        // the popup stays open and says why
        if (!env.applyImport(text)) {
          showImportNote(panel, 'tb-io-note', 'tb-io-btns', 'No colours found in that text, so your panels are unchanged.');
          return;
        }

        renderList();
        ioHost.close();
      },
    });
  }

  function closeSheet(): void {
    env.picker.close();
    ioHost.close();
    sheetHost.close();
    editorList = null;
    // let the page behind scroll again
    document.documentElement.style.overflow = '';
    env.onDone();
  }

  // the bulk row under the legend: flip every module's border or header in one tap. each
  // button toggles both ways and its label says what the next tap does (All borders
  // On/Off). the flags are per size so the sweep covers every module at every size it
  // supports
  function buildBulkBar(): HTMLElement {
    const bar = document.createElement('div');

    bar.className = 'tb-bar';

    function mapFor(which: string): FlagMap {
      return which === 'header' ? env.getHeaderless() : env.getBorderless();
    }

    // is any sub row still on (its flag not set) for this flag kind. keys off the sub
    // row's own panel (row.value/row.size) and skips a header the sub row can't show so
    // it matches the per row toggles exactly (a themeRows family's 2x2 can represent a
    // different panel id)
    function anyStillOn(which: string): boolean {
      const map = mapFor(which);

      for (let index = 0; index < env.modules.length; index++) {
        const rows = env.modules[index].sizeRows;

        for (let sizeIndex = 0; sizeIndex < rows.length; sizeIndex++) {
          const subRow = rows[sizeIndex];

          if (which === 'header' && subRow.alwaysHeaderless) {
            continue;
          }

          if (!flagOn(map, subRow.value, subRow.size)) {
            return true;
          }
        }
      }

      return false;
    }

    // set every sub row's flag at once for this flag kind and key it off the sub row's own panel
    function setAll(which: string, on: boolean): void {
      const map = mapFor(which);

      for (let index = 0; index < env.modules.length; index++) {
        const rows = env.modules[index].sizeRows;

        for (let sizeIndex = 0; sizeIndex < rows.length; sizeIndex++) {
          const subRow = rows[sizeIndex];

          if (which === 'header' && subRow.alwaysHeaderless) {
            continue;
          }

          setFlag(map, subRow.value, subRow.size, on);
        }
      }
    }

    function makeBulkButton(label: string, which: string): HTMLElement {
      const button = document.createElement('button');

      button.type = 'button';
      button.className = 'tb-bar-btn ghost';

      const paint = function () {
        button.textContent = anyStillOn(which)
          ? 'All ' + label + ' Off'
          : 'All ' + label + ' On';
      };

      paint();
      bulkPainters.push(paint);
      button.addEventListener('click', function () {
        // any still on -> hide them all. otherwise show them all again
        setAll(which, anyStillOn(which));
        env.persist();
        paint();
        renderList();
      });
      return button;
    }

    bar.appendChild(makeBulkButton('borders', 'border'));
    bar.appendChild(makeBulkButton('headers', 'header'));

    return bar;
  }

  // the colour sweep above the border and header sweeps: one picker that sets a channel on
  // every panel at once. it gets a bar of its own so the three labels never squeeze on a phone
  function buildColourBar(): HTMLElement {
    const bar = document.createElement('div');

    bar.className = 'tb-bar';

    const button = document.createElement('button');

    button.type = 'button';
    button.className = 'tb-bar-btn ghost';
    button.textContent = 'Colour All Panels';
    button.addEventListener('click', function () {
      env.openColourAll(renderList);
    });
    bar.appendChild(button);

    return bar;
  }

  // the Reset / Import-Export / Done bar pinned at the bottom of the sheet
  function buildBar(isBottom: boolean): HTMLElement {
    const bar = document.createElement('div');

    bar.className = 'tb-bar' + (isBottom ? ' tb-bar-bottom' : '');

    const resetButton = document.createElement('button');

    // what the table held before the last Reset, so the same button can turn into Undo. any edit
    // after the Reset drops it, so Undo only ever steps back over the Reset itself
    let beforeReset: string | null = null;

    resetButton.type = 'button';
    resetButton.className = 'tb-bar-btn ghost';
    resetButton.textContent = 'Reset';

    env.watchEdits(function () {
      beforeReset = null;
      resetButton.textContent = 'Reset';
    });

    resetButton.addEventListener('click', function () {
      if (beforeReset !== null) {
        // putting the old table back counts as an edit, which turns the button back into Reset
        env.restore(beforeReset);
      } else {
        const saved = env.serialize();

        env.resetAll();
        env.persist();
        beforeReset = saved;
        resetButton.textContent = 'Undo';
      }

      renderList();
    });
    bar.appendChild(resetButton);

    const ioButton = document.createElement('button');

    ioButton.type = 'button';
    ioButton.className = 'tb-bar-btn io';
    ioButton.textContent = 'Import/Export';
    ioButton.addEventListener('click', openIO);
    bar.appendChild(ioButton);

    const doneButton = document.createElement('button');

    doneButton.type = 'button';
    doneButton.className = 'tb-bar-btn';
    doneButton.textContent = 'Done';
    doneButton.addEventListener('click', closeSheet);
    bar.appendChild(doneButton);

    return bar;
  }

  function open(): void {
    const panel = sheetHost.open();

    bulkPainters = [];

    const legend = document.createElement('div');

    legend.className = 'tb-legend';
    // a two column grid (fixed term column with flexible description) keeps every term word
    // and every description flush at one x. the dot sits in a fixed slot so H/B which
    // has no dot still lines up with the coloured channels
    legend.innerHTML =
      '<div class="tb-legend-ttl">What each control changes</div>' +
      '<div class="tb-legend-grid">' +
      '<div class="tb-lg-term"><span class="tb-lg-dot empty"></span>H / B</div>' +
      '<div class="tb-lg-desc">shows or hides the header strip or the panel outline at each size. Green is shown, grey is hidden</div>' +
      '<div class="tb-lg-term"><span class="tb-lg-dot"></span>Accent</div>' +
      '<div class="tb-lg-desc">the header bar, border, and progress bar, the same at every size</div>' +
      '<div class="tb-lg-term"><span class="tb-lg-dot"></span>Value</div>' +
      '<div class="tb-lg-desc">the big number or readout</div>' +
      '<div class="tb-lg-term"><span class="tb-lg-dot"></span>Icon</div>' +
      '<div class="tb-lg-desc">the icon</div>' +
      '<div class="tb-lg-term"><span class="tb-lg-dot"></span>Sub</div>' +
      '<div class="tb-lg-desc">the small caption line, such as OF 10,000</div>' +
      '</div>';

    editorList = document.createElement('div');
    editorList.className = 'tb-list';
    renderList();

    // the legend and the bulk sweep bars and the list all scroll together so the visible
    // list area stays as tall as possible on a small screen. only the footer stays pinned
    const scroller = document.createElement('div');

    scroller.className = 'tb-scroll';
    scroller.appendChild(legend);
    scroller.appendChild(buildColourBar());
    scroller.appendChild(buildBulkBar());
    scroller.appendChild(editorList);
    panel.appendChild(scroller);
    panel.appendChild(buildBar(true));

    // the sheet covers the whole screen so stop the page behind it scrolling
    document.documentElement.style.overflow = 'hidden';
  }

  return { open: open };
}
