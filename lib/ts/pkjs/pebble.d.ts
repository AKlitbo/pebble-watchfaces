/**
 * PebbleKit JS runtime globals and the shapes the app talks to. Ambient (global),
 * so app.ts uses these without importing. Only the surface app.ts touches is typed.
 * There is no upstream @types for PebbleKit JS.
 */

/** The AppMessage dict: message-key -> value the watch decodes. */
type AppMessageDict = Record<string, number | string | number[]>;

/** The event the 'appmessage' listener receives. */
interface PebbleAppMessageEvent {
  payload?: Record<string, number | string>;
}

/** The event the 'webviewclosed' listener receives (the saved settings JSON). */
interface PebbleWebviewClosedEvent {
  response?: string;
}

/** The PebbleKit JS bridge object the app sends messages and listens through. */
interface PebbleBridge {
  sendAppMessage(dict: AppMessageDict, onSuccess?: () => void, onError?: () => void): void;
  openURL(url: string): void;
  addEventListener(type: 'ready' | 'showConfiguration', handler: () => void): void;
  addEventListener(type: 'appmessage', handler: (event: PebbleAppMessageEvent) => void): void;
  addEventListener(type: 'webviewclosed', handler: (event: PebbleWebviewClosedEvent) => void): void;
}

declare const Pebble: PebbleBridge;

/** The Clay class the deferred require('@rebble/clay/...') returns. */
interface ClayConstructor {
  new (config: unknown, customClay?: unknown, options?: { autoHandleEvents?: boolean }): {
    registerComponent(component: unknown): void;
    getSettings(json: string): AppMessageDict;
    generateUrl(): string;
  };
}

// the CommonJS loader for the deferred Clay / message_keys lookups (they load
// lazily so the specs can skip them). message_keys is a face-generated map whose
// keys are present-or-undefined and used both as computed keys and in presence
// checks so its value is genuinely dynamic
declare function require(id: string): any;
