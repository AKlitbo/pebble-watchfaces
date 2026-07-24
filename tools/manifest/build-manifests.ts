/**
 * Generate targets/<face>/package.json from watchfaces/<face>/config/pebble.appinfo.json.
 *
 * pebble.appinfo.json holds the Pebble appinfo (uuid, messageKeys, the whole resource
 * list) plus the per-face build identity: the release name, the watchface flag, and (for
 * an app) which bundled icon is the launcher menu icon. The author/version come from the
 * root package.json, so there is one place for each fact.
 *
 * Each face builds to its own staging sandbox targets/<face>/, so there is one manifest
 * per face (the watchface/watchapp target-type split collapsed into the face name). The
 * manifest is a gitignored build input written as plain JSON — nobody reads it by hand.
 * `pebble build` needs package.json to exist before it runs, so each build regenerates it
 * via build.sh.
 *
 * Usage: node tools/manifest/build-manifests.ts <face>
 */
import fs from 'node:fs';
import path from 'node:path';

const ROOT = path.resolve(import.meta.dirname, '..', '..');
const ROOT_PKG = path.join(ROOT, 'package.json');
// every face's wscript is identical (waf_helpers keys off the sandbox dir name), so it is
// generated from one template rather than committed per face - a face missing its wscript
// makes `pebble build` report "This project is very outdated" instead of anything useful
const WSCRIPT_TEMPLATE = path.join(ROOT, 'tools', 'waf', 'wscript.template');

/** watchfaces/<face>/config/pebble.appinfo.json for a face. */
function appinfoPath(face: string): string {
  return path.join(ROOT, 'watchfaces', face, 'config', 'pebble.appinfo.json');
}

/**
 * One entry in config/pebble.appinfo.json's resources.media: a bitmap or font the SDK
 * packs. generate-icons.ts rewrites the bitmap rows, so it reads this shape too.
 */
export type MediaEntry = { type: string; name: string; file?: string; menuIcon?: boolean; [key: string]: unknown };

/** The per-face build identity: the bits that make a manifest a watchface or a watchapp. */
type Target = { name: string; watchface: boolean; menuIcon?: string };

/**
 * The shared Pebble fields common to every face — what building a manifest reads. The
 * per-face build identity (name/watchface/menuIcon) rides alongside these in the file but
 * is split out into a Target before buildManifest sees it.
 */
export type SharedAppinfo = {
  displayName: string;
  uuid: string;
  sdkVersion: string;
  enableMultiJS: boolean;
  targetPlatforms: string[];
  capabilities: unknown;
  messageKeys: unknown;
  resources: { media: MediaEntry[] };
};

/**
 * The whole appinfo file: the shared Pebble fields plus this face's build identity and its
 * own version. Faces version independently (each keeps its own CHANGELOG.md), so the version
 * lives here rather than in the root package.json.
 */
type Appinfo = SharedAppinfo & Target & { version?: string };

/** The facts each manifest copies out of the root package.json. */
type RootPkg = { author: string; version: string };

/** Builds one target's media list, marking its menu icon if it declares one. */
export function buildMedia(config: SharedAppinfo, target: Target): MediaEntry[] {
  const media: MediaEntry[] = JSON.parse(JSON.stringify(config.resources.media));
  if (target.menuIcon) {
    const entry = media.find((item) => item.name === target.menuIcon);
    if (!entry) {
      throw new Error(`menuIcon ${target.menuIcon} is not in the media list`);
    }
    entry.menuIcon = true;
  }
  return media;
}

/** Builds one target's whole package.json manifest from the shared config and the root package. */
export function buildManifest(config: SharedAppinfo, rootPkg: RootPkg, target: Target) {
  return {
    name: target.name,
    author: rootPkg.author,
    version: rootPkg.version,
    private: true,
    pebble: {
      displayName: config.displayName,
      uuid: config.uuid,
      sdkVersion: config.sdkVersion,
      enableMultiJS: config.enableMultiJS,
      targetPlatforms: config.targetPlatforms,
      watchapp: { watchface: target.watchface },
      capabilities: config.capabilities,
      messageKeys: config.messageKeys,
      resources: { media: buildMedia(config, target) },
    },
  };
}

/** Writes targets/<face>/package.json from that face's appinfo. */
function main() {
  const face = process.argv[2];
  if (!face) {
    console.error('usage: build-manifests.ts <face>');
    process.exit(1);
  }

  const config: Appinfo = JSON.parse(fs.readFileSync(appinfoPath(face), 'utf8'));
  const rootPkg: RootPkg = JSON.parse(fs.readFileSync(ROOT_PKG, 'utf8'));

  const target: Target = { name: config.name, watchface: config.watchface, menuIcon: config.menuIcon };
  // the face's own version wins. the root package.json is the fallback and still owns the author
  const version = config.version || rootPkg.version;
  const manifest = buildManifest(config, { author: rootPkg.author, version }, target);

  const outDir = path.join(ROOT, 'targets', face);
  fs.mkdirSync(outDir, { recursive: true });
  const out = path.join(outDir, 'package.json');
  fs.writeFileSync(out, JSON.stringify(manifest, null, 2) + '\n');

  // the waf entry point has to exist before `pebble build` runs in this sandbox
  const wscript = path.join(outDir, 'wscript');
  fs.copyFileSync(WSCRIPT_TEMPLATE, wscript);

  console.log(`Wrote ${path.relative(ROOT, out)} and wscript (${target.name} ${version}, watchface=${target.watchface}).`);
}

if (import.meta.main) {
  main();
}
