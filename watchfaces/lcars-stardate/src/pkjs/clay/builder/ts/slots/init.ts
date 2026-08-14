/**
 * Wiring: find the page's pieces, paint the face and the palette, and hang the hooks Clay's
 * manipulator calls.
 *
 * This is the only piece that touches Clay. Everything above it is plain functions over the four
 * slot values, which is what makes them testable outside the config webview.
 */

import { SLOT_COUNT, ID_EMPTY, isTall } from './geometry';
import { formatId, readSlots, sanitize, writeStores } from './codec';
import { SLOT_PRESETS } from './presets';
import { buildReadoutList, fillVisual, readoutById } from './visuals';
import type { Readout, Thumbs } from './visuals';
import { installDrag } from './drag';

/** What Clay binds initialize to. */
interface ClayContext {
  $element: HTMLElement[];
  config?: { moduleOptions?: unknown[]; moduleThumbnails?: Thumbs };
  trigger: (event: string) => void;
}

export function init(this: ClayContext): void {
  const root = this.$element[0];
  const cfg = this.config || {};
  // the change is fired through the bound trigger rather than through `this`
  // the nested callbacks below do not carry it
  const fireChange = this.trigger.bind(this);

  const READOUTS: Readout[] = buildReadoutList((cfg.moduleOptions || []) as never[]);
  const THUMBS: Thumbs = cfg.moduleThumbnails || {};

  const hidden = root.querySelector('.sb-value') as HTMLInputElement;
  const slotsWrap = root.querySelector('.sb-slots') as HTMLElement;
  const palette = root.querySelector('.sb-palette') as HTMLElement;

  const slotEls: HTMLElement[] = [];
  for (let slot = 0; slot < SLOT_COUNT; slot++) {
    slotEls.push(root.querySelector('.sb-slot[data-slot="' + slot + '"]') as HTMLElement);
  }

  let slots = readSlots(root, hidden.value);

  /** Paint the four panels and push the values back out. */
  function render(): void {
    // the left column merges into one box when it is showing the sensors block
    slotsWrap.classList.toggle('tall', isTall(slots[0]));

    for (let slot = 0; slot < SLOT_COUNT; slot++) {
      const readout = slots[slot] === ID_EMPTY ? null : readoutById(READOUTS, slots[slot]);
      fillVisual(slotEls[slot], readout, THUMBS, false);
      slotEls[slot].classList.toggle('filled', Boolean(readout));
    }

    publish();
  }

  /**
   * Hand the values to Clay: the upper-left through this component's own value, the rest through
   * their stores. Only fire the change when something actually moved, so opening the page does
   * not mark the settings dirty on its own.
   */
  function publish(): void {
    const own = formatId(slots[0]);

    if (hidden.value !== own) {
      hidden.value = own;
      fireChange('change');
    }

    writeStores(root, slots);
  }

  function setSlots(next: number[]): void {
    slots = sanitize(next);
    render();
  }

  const drag = installDrag({
    root: root,
    readouts: READOUTS,
    thumbs: THUMBS,
    slotEls: slotEls,
    getSlots: function () { return slots; },
    setSlots: setSlots,
  });

  // --- the face ---

  for (let slot = 0; slot < SLOT_COUNT; slot++) {
    (function (index) {
      slotEls[index].addEventListener('pointerdown', function (event) {
        drag.fromSlot(index, event as PointerEvent);
      });
    })(slot);
  }

  // --- the palette ---

  READOUTS.forEach(function (readout) {
    const cell = root.ownerDocument.createElement('div');
    cell.className = 'sb-pal' + (isTall(readout.value) ? ' tall' : '');
    cell.title = readout.label;
    fillVisual(cell, readout, THUMBS, true);

    cell.addEventListener('pointerdown', function (event) {
      drag.fromPalette(readout.value, event as PointerEvent);
    });

    palette.appendChild(cell);
  });

  // --- actions ---

  const presetBtns = root.querySelectorAll('.sb-preset');
  for (let i = 0; i < presetBtns.length; i++) {
    presetBtns[i].addEventListener('click', function (event) {
      const id = (event.currentTarget as HTMLElement).getAttribute('data-preset') || '';
      if (SLOT_PRESETS[id]) {
        setSlots(SLOT_PRESETS[id]);
      }
    });
  }

  const clear = root.querySelector('.sb-btn-clear');
  if (clear) {
    clear.addEventListener('click', function () {
      setSlots([ID_EMPTY, ID_EMPTY, ID_EMPTY, ID_EMPTY]);
    });
  }

  // --- the hooks Clay's manipulator calls ---
  // it speaks the upper-left slot only which is this component's message key
  // the other three are stores it knows nothing about so they get read and
  // written around it

  interface Hooked extends HTMLElement { _sbSet?: (value: string) => void; _sbGet?: () => string }
  const hooked = root as Hooked;

  hooked._sbSet = function (value: string): void {
    slots = readSlots(root, value);
    render();
  };

  hooked._sbGet = function (): string {
    return formatId(slots[0]);
  };

  render();

  // the stores may be built after this component and then the read above saw
  // nothing. pick them up once the page has finished. the mosaic builder keeps
  // the same safety net
  setTimeout(function () {
    const late = readSlots(root, hidden.value);
    const moved = late.some(function (id, slot) { return id !== slots[slot]; });

    if (moved) {
      slots = late;
      render();
    }
  }, 0);
}
