/**
 * Local Clay preview.
 *
 * Builds a face's real settings page (the same @rebble/clay the watch build uses, with its own
 * custom components and every normal item) into a plain HTML file you can open in a browser. Lets
 * you click around the config UI without loading it onto the watch after every change, and — the
 * reason it earns its keep — shows you the console error when the page refuses to render, which on
 * the watch just looks like a settings screen that never appears.
 *
 * Run once:   node tools/dev/clay-preview.ts <face>
 * Keep fresh: node tools/dev/clay-preview.ts <face> --watch   (then refresh the browser)
 * As a user:  node tools/dev/clay-preview.ts <face> --settings=saved.json
 *
 * Opens to: tools/dev/clay-preview.html
 *
 * --settings seeds the page from a { messageKey: value } file, which is what "open the settings on
 * a watch you have already configured" actually means. Without it every preview is a brand new
 * install, and a bug that only bites returning users renders perfectly.
 *
 * The Save button does nothing here: it talks to the phone. This is for eyeballing the page.
 */
import fs from 'node:fs';
import path from 'node:path';
import Module from 'node:module';
import { createRequire } from 'node:module';
import { facePaths, compile, copyGenerated, writeTsconfig } from '../pkjs/build-pkjs.ts';
import { faceRelative } from '../faces.ts';

const requireHost = createRequire(import.meta.url);

const ROOT = path.resolve(import.meta.dirname, '..', '..');
const OUT = path.join(import.meta.dirname, 'clay-preview.html');

const face = process.argv[2];
if (!face || face.startsWith('--')) {
  console.error('usage: node tools/dev/clay-preview.ts <face> [--watch] [--platform=emery|gabbro]');
  process.exit(1);
}

const rel = faceRelative(face);
const paths = facePaths(face);

// the bundle pokes at a few host globals while it builds the page, so give it harmless stand-ins
// under plain node. a defined Pebble (not "pypkjs") is what makes generateUrl hand back a
// self-contained data url instead of a hosted-previewer link. these are deliberately partial so
// they go through a loose host view rather than pretending to be a real Navigator or Storage
const host = global as unknown as Record<string, unknown>;

if (typeof host.navigator === 'undefined') { host.navigator = { userAgent: 'node' }; }
if (typeof host.window === 'undefined') { host.window = {}; }
// which watch the page should think it was opened from. Clay filters items by their declared
// capabilities against this, so it is what shows the round build losing its rectangle-only
// controls without a watch in hand. firmware is stubbed alongside the platform because Clay
// reads both when it resolves a capability
const platformArg = process.argv.slice(3).find((arg) => arg.startsWith('--platform='));
const platform = platformArg ? platformArg.slice('--platform='.length) : 'emery';

if (typeof host.Pebble === 'undefined') {
  host.Pebble = {
    platform,
    addEventListener: function () {},
    getAccountToken: function () { return ''; },
    getWatchToken: function () { return ''; },
    getActiveWatchInfo: function () {
      return { platform, firmware: { major: 4, minor: 3, patch: 0 } };
    },
  };
}
// clay reads the values it seeds the page with straight out of localStorage under this key, so
// pre-loading it is all "open the settings again later" takes
const seed = process.argv.slice(3).find((arg) => arg.startsWith('--settings='));

if (typeof host.localStorage === 'undefined') {
  const store: Record<string, string> = {};
  if (seed) {
    store['clay-settings'] = fs.readFileSync(seed.slice('--settings='.length), 'utf8');
  }
  host.localStorage = {
    getItem: function (key: string) { return key in store ? store[key] : null; },
    setItem: function (key: string, value: string) { store[key] = String(value); },
  };
}

// the watch build aliases "message_keys" to a generated file which does not exist under plain
// node, so stand in a map built from the appinfo. the real ids do not matter for rendering
const appinfo = path.join(ROOT, 'watchfaces', rel, 'config', 'pebble.appinfo.json');
const declared: string[] = JSON.parse(fs.readFileSync(appinfo, 'utf8')).messageKeys || [];
const messageKeysStub: Record<string, number> = {};
declared.forEach((name, i) => { messageKeysStub[name] = 10000 + i; });

// _load is a private Node internal so it is not in the public module types
const moduleInternal = Module as unknown as {
  _load: (request: string, ...args: unknown[]) => unknown;
};
const origLoad = moduleInternal._load;
moduleInternal._load = function (this: unknown, request: string, ...args: unknown[]) {
  if (request === 'message_keys') { return messageKeysStub; }
  return origLoad.apply(this, [request, ...args]);
};

const Clay = requireHost('@rebble/clay/src/js/index');

/** Pull the raw HTML out of the data url generateUrl hands back. */
function htmlFromDataUrl(url: string): string {
  const comma = url.indexOf(',');
  const head = url.slice(0, comma);
  const body = url.slice(comma + 1);
  return head.indexOf(';base64') !== -1
    ? Buffer.from(body, 'base64').toString('utf8')
    : decodeURIComponent(body);
}

/** Whether a module actually is a Clay component, rather than a lump of generated data. */
function isComponent(value: unknown): boolean {
  if (!value || typeof value !== 'object') {
    return false;
  }
  const candidate = value as { name?: unknown; manipulator?: unknown };
  return typeof candidate.name === 'string' && Boolean(candidate.manipulator);
}

/**
 * Every custom component the face's emit tree carries, ready to register.
 *
 * Picked by shape rather than by filename: the same clay/ folder holds generated *data* (the
 * thumbnail and vibrant tables), and handing one of those to registerComponent throws
 * "The manipulator must be defined", which reads like a broken component rather than a file that
 * was never a component at all.
 */
function components(emit: string): unknown[] {
  const found: unknown[] = [];

  function collect(dir: string, suffix: string): void {
    if (!fs.existsSync(dir)) {
      return;
    }
    for (const name of fs.readdirSync(dir)) {
      if (!name.endsWith(suffix)) {
        continue;
      }
      // the generated .g.js builders are plain CommonJS; the TS ones land on .default
      const mod = requireHost(path.join(dir, name)) as { default?: unknown };
      const candidate = mod.default || mod;
      if (isComponent(candidate)) {
        found.push(candidate);
      }
    }
  }

  collect(path.join(emit, 'watchfaces', ...rel.split('/'), 'src', 'pkjs', 'clay'), '.g.js');
  collect(path.join(emit, 'lib', 'ts', 'clay'), '-component.js');

  return found;
}

/** Renders the settings page to clay-preview.html so a browser can open it. */
function build(): void {
  // required per build so each pass picks up the freshly compiled emit/ tree. at module scope they
  // would stay bound to the first load and dropping the require cache would not budge them
  const configPath = path.join(paths.emit, 'watchfaces', ...rel.split('/'), 'src', 'pkjs', 'config.js');
  const config = (requireHost(configPath) as { default?: unknown }).default;

  const clay = new Clay(config, null, { autoHandleEvents: false });
  for (const component of components(paths.emit)) {
    clay.registerComponent(component);
  }

  // Clay only fills meta inside its showConfiguration handler, and generateUrl is called here
  // without one, so the page would carry activeWatchInfo: null and show every item whatever
  // --platform said. filling it is what makes the capability filter testable off-watch
  (clay as { meta?: unknown }).meta = {
    activeWatchInfo: { platform, firmware: { major: 4, minor: 3, patch: 0 } },
    accountToken: '',
    watchToken: '',
    userData: {},
  };

  const html = htmlFromDataUrl(clay.generateUrl());
  fs.writeFileSync(OUT, html);

  const stamp = new Date().toISOString().slice(11, 19);
  console.log('[' + stamp + '] wrote ' + OUT + ' (' + html.length + ' bytes) for ' + face);
}

writeTsconfig(face, paths);
compile(paths);
copyGenerated(paths);
build();

if (process.argv.indexOf('--watch') !== -1) {
  const watched = [
    path.join(ROOT, 'watchfaces', rel, 'src', 'pkjs'),
    path.join(ROOT, 'lib', 'ts', 'clay'),
  ].filter((dir) => fs.existsSync(dir));

  // fs.watch fires more than once per save and every pass shells out to a synchronous tsc, so
  // roll a burst of events into one rebuild
  let timer: ReturnType<typeof setTimeout> | null = null;

  const rebuild = (): void => {
    // node caches required modules, so drop them before the next build
    Object.keys(requireHost.cache).forEach((key) => { delete requireHost.cache[key]; });
    try {
      // no cleanEmit here: wiping emit/ would mean a cold tsc on every keystroke
      compile(paths);
      copyGenerated(paths);
      build();
    } catch (error) {
      console.error('build failed: ' + (error as Error).message);
    }
  };

  console.log('watching for changes. refresh the browser after each save.');
  console.log('note: edits under clay/builder need `npm run gen:gridlock:clay` first.');
  watched.forEach((dir) => {
    fs.watch(dir, { recursive: true }, () => {
      if (timer) { clearTimeout(timer); }
      timer = setTimeout(rebuild, 150);
    });
  });
}
