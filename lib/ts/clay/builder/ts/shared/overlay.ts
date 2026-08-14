/**
 * The one overlay pattern both builders use: a dimmed backdrop with a panel on
 * top of it, both appended straight to body so a parent transform can never
 * break the fixed centring.
 *
 * This is a Clay builder piece. esbuild bundles it into a component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

/** An open/close pair for one kind of overlay, open returning the fresh panel. */
export interface OverlayHost {
  open(): HTMLElement;
  close(): void;
}

/**
 * Makes an open/close pair for one kind of overlay.
 *
 * Opening again first closes whatever this host already had up, so a host can
 * never stack two of its own panels. Tapping the backdrop closes the pair when
 * dismissOnTap is set, which is how the popups treat "tap outside" as cancel.
 */
export function createOverlayHost(overlayClass: string, panelClass: string, dismissOnTap: boolean): OverlayHost {
  let backdrop: HTMLElement | null = null;
  let panel: HTMLElement | null = null;

  function close(): void {
    if (backdrop && backdrop.parentNode) {
      backdrop.parentNode.removeChild(backdrop);
    }
    if (panel && panel.parentNode) {
      panel.parentNode.removeChild(panel);
    }

    backdrop = null;
    panel = null;
  }

  function open(): HTMLElement {
    close();

    backdrop = document.createElement('div');
    backdrop.className = overlayClass;
    if (dismissOnTap) {
      backdrop.addEventListener('click', close);
    }

    panel = document.createElement('div');
    panel.className = panelClass;

    document.body.appendChild(backdrop);
    document.body.appendChild(panel);

    return panel;
  }

  return { open: open, close: close };
}
