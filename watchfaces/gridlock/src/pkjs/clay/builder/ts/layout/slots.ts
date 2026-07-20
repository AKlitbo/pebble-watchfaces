/**
 * The two personal layout slots behind the Saved Layouts button. Each slot has
 * a status line and a Save and a Load button. Save snapshots the current layout
 * wire string, Load reads it back. Overwriting a filled slot, or loading over
 * the layout on screen, arms the button for a second tap first so a stray tap
 * cannot wipe either one.
 *
 * Both slots live in the hidden gl-store input on the page (see
 * hidden-store-component), so Clay saves them to the phone and seeds them back
 * next time. The config webview blocks its own localStorage, so this is the way
 * the slots survive a visit. It also blocks native confirm/alert dialogs, which
 * is why the guard is an inline two-tap on the button, never window.confirm.
 *
 * This is a Clay builder piece. esbuild bundles it into the layout component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

/** how long an armed Save/Load button waits for the second tap before it resets */
const ARM_RESET_MS = 2500;

/** One saved slot: the layout wire string and when it was saved. */
interface SavedSlot {
  layout: string;
  ts: number;
}

/** The two slots keyed by side, either one possibly still empty. */
interface SlotStore {
  a?: SavedSlot;
  b?: SavedSlot;
}

/** How buildSlotsPanel reaches the builder to read and apply a layout. */
export interface SlotsPanelOpts {
  getCurrent: () => string;
  onLoad: (layout: string) => void;
}

/** Finds the hidden store input both slots share, or null if the page has none. */
function storeInput(): HTMLInputElement | null {
  return document.querySelector('.gl-store');
}

/** Reads both slots back, treating a missing store or junk value as no slots. */
function readSlots(): SlotStore {
  const input = storeInput();
  if (!input) {
    return {};
  }
  try {
    return JSON.parse(input.value || '') || {};
  } catch (error) {
    return {};
  }
}

/** Writes both slots into the hidden store so Clay saves them, reporting success. */
function writeSlots(store: SlotStore): boolean {
  const input = storeInput();
  if (!input) {
    return false;
  }
  input.value = JSON.stringify(store);
  // nudge Clay to pick up the new value the way a typed field would
  input.dispatchEvent(new Event('change'));
  return true;
}

/** Turns a saved timestamp into a short "MM/DD HH:MM" stamp for the status line. */
function stamp(ms: number): string {
  const date = new Date(ms);
  function pad(value: number): string {
    return value < 10 ? '0' + value : String(value);
  }
  return pad(date.getMonth() + 1) + '/' + pad(date.getDate()) + ' ' + pad(date.getHours()) + ':' + pad(date.getMinutes());
}

/** Builds one slot's row: the name, its status and the Save/Load pair. */
function buildSlotRow(panel: HTMLElement, key: 'a' | 'b', label: string, opts: SlotsPanelOpts): void {
  const row = document.createElement('div');
  row.className = 'lb-slot-row';

  const head = document.createElement('div');
  head.className = 'lb-slot-head';

  const name = document.createElement('span');
  name.className = 'lb-slot-name';
  name.textContent = label;
  head.appendChild(name);

  const status = document.createElement('span');
  status.className = 'lb-slot-status';
  head.appendChild(status);

  row.appendChild(head);

  const buttons = document.createElement('div');
  buttons.className = 'lb-io-btns';

  const saveButton = document.createElement('button');
  saveButton.type = 'button';
  saveButton.className = 'lb-io-btn';
  saveButton.textContent = 'Save';
  buttons.appendChild(saveButton);

  const loadButton = document.createElement('button');
  loadButton.type = 'button';
  loadButton.className = 'lb-io-btn primary';
  loadButton.textContent = 'Load';
  buttons.appendChild(loadButton);

  row.appendChild(buttons);
  panel.appendChild(row);

  // one timer per row disarms whichever button was primed if the second tap
  // never comes
  let armed: HTMLButtonElement | null = null;
  let armTimer: ReturnType<typeof setTimeout> | null = null;

  // repaint the status line and gate Load off an empty slot
  function refresh(): void {
    const slot = readSlots()[key];
    if (slot) {
      status.textContent = 'saved ' + stamp(slot.ts);
      loadButton.disabled = false;
    } else {
      status.textContent = 'empty';
      loadButton.disabled = true;
    }
  }

  // primes a button for its second tap, showing the given label until it resets
  function arm(button: HTMLButtonElement, prompt: string): void {
    disarm();
    armed = button;
    button.textContent = prompt;
    button.classList.add('armed');
    armTimer = setTimeout(disarm, ARM_RESET_MS);
  }

  // clears any primed button back to its normal label
  function disarm(): void {
    if (armTimer) {
      clearTimeout(armTimer);
      armTimer = null;
    }
    if (armed) {
      armed.classList.remove('armed');
      armed = null;
    }
    saveButton.textContent = 'Save';
    loadButton.textContent = 'Load';
  }

  // flashes a button label for a beat so a tap always shows it did something
  function flash(button: HTMLButtonElement, text: string): void {
    const original = button.textContent;
    button.textContent = text;
    setTimeout(function () {
      button.textContent = original;
    }, 1200);
  }

  function doSave(): void {
    const store = readSlots();
    store[key] = { layout: opts.getCurrent(), ts: Date.now() };
    if (writeSlots(store)) {
      refresh();
      flash(saveButton, 'Saved');
    } else {
      status.textContent = 'storage unavailable';
    }
  }

  saveButton.addEventListener('click', function () {
    // a filled slot wants a confirming second tap, an empty one just saves
    if (armed === saveButton) {
      disarm();
      doSave();
    } else if (readSlots()[key]) {
      arm(saveButton, 'Overwrite?');
    } else {
      doSave();
    }
  });

  loadButton.addEventListener('click', function () {
    if (!readSlots()[key]) {
      return;
    }
    if (armed === loadButton) {
      disarm();
      const slot = readSlots()[key];
      if (slot) {
        opts.onLoad(slot.layout);
      }
    } else {
      arm(loadButton, 'Load?');
    }
  });

  refresh();
}

/**
 * Fills the Saved Layouts panel with a heading and both slot rows. The caller
 * passes getCurrent so a Save reads the live layout, and onLoad so a Load hands
 * the stored string back for the builder to apply.
 */
export function buildSlotsPanel(panel: HTMLElement, opts: SlotsPanelOpts): void {
  const title = document.createElement('div');
  title.className = 'lb-title';
  title.textContent = 'Saved Layouts';
  panel.appendChild(title);

  buildSlotRow(panel, 'a', 'Slot A', opts);
  buildSlotRow(panel, 'b', 'Slot B', opts);
}
