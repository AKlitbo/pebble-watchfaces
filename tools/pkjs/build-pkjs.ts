/**
 * Builds targets/<face>/emit/, the only tree the Pebble bundler reads for a face.
 *
 * Four steps that have to happen in this order, which is why they live in one tool
 * rather than an && chain in package.json:
 *
 *   clean    tsc never prunes its outDir, so a deleted or renamed .ts would leave its old
 *            .js behind. waf globs emit/**\/*.js with no filter, so that zombie ships: it
 *            spends bytes against the 65535 cap, and a stale require still resolves
 *            against it, which makes a half-finished rename look fine locally while CI
 *            (always building a fresh emit/) disagrees.
 *   compile  watchfaces/<face>/src/pkjs + lib/ts -> emit/, as CommonJS for the SDK's bundler.
 *   copy     the *.g.js Clay components are committed, not tsc output, so tsc never puts
 *            them in emit/. The emitted index.js requires them by relative path, so they
 *            have to land beside it.
 *   vendor   ical.js ships from node_modules rather than the source tree, so it lands the
 *            same way for the same reason.
 *
 * emit/ is written straight into the face's waf staging sandbox (targets/<face>/) so the
 * native build never has to stage it. tsc roots at the repo root (lib/ts sits outside any
 * one face), so the tree keeps its source shape: emit/watchfaces/<face>/src/pkjs/index.js
 * beside emit/lib/ts/**. waf_helpers.build_face computes the js entry from the face to match.
 *
 * Run via `npm run build:pkjs -- <face>`, and by build.sh before every Pebble build.
 */
import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { createRequire } from 'node:module';

const requireHost = createRequire(import.meta.url);

const ROOT = path.resolve(import.meta.dirname, '..', '..');
const PKJS_BASE_TSCONFIG = path.join(ROOT, 'config', 'tsconfig.pkjs.json');

// the package is ESM behind an `exports` map and the SDK bundles with webpack 1, which reads
// neither, so asking for it by name would resolve its ESM build and break. the package ships a
// prebuilt ES5 CommonJS file for exactly this, and it lands where lib/ts/calendar/icaljs.d.ts
// says it does. keeping it a file of its own is also what MPL 2.0 asks of a larger work
const ICALJS_FROM = path.join(ROOT, 'node_modules', 'ical.js', 'dist', 'ical.es5.min.cjs');

/** The per-face paths this tool reads and writes, all derived from the face name. */
export interface FacePaths {
  faceSrc: string;   // watchfaces/<face>/src/pkjs
  sandbox: string;   // targets/<face>
  emit: string;      // targets/<face>/emit
  emitPkjs: string;  // targets/<face>/emit/watchfaces/<face>/src/pkjs
  icaljsTo: string;  // targets/<face>/emit/lib/ts/calendar/icaljs.js
  tsconfig: string;  // targets/<face>/tsconfig.pkjs.json (generated)
  skipDir: string;   // watchfaces/<face>/src/pkjs/clay/builder
}

export function facePaths(face: string): FacePaths {
  const faceSrc = path.join(ROOT, 'watchfaces', face, 'src', 'pkjs');
  const sandbox = path.join(ROOT, 'targets', face);
  const emit = path.join(sandbox, 'emit');
  return {
    faceSrc,
    sandbox,
    emit,
    emitPkjs: path.join(emit, 'watchfaces', face, 'src', 'pkjs'),
    icaljsTo: path.join(emit, 'lib', 'ts', 'calendar', 'icaljs.js'),
    tsconfig: path.join(sandbox, 'tsconfig.pkjs.json'),
    skipDir: path.join(faceSrc, 'clay', 'builder'),
  };
}

/** Empties the face's emit tree. It is entirely derived and gitignored, so this is always safe. */
export function cleanEmit(p: FacePaths): void {
  fs.rmSync(p.emit, { recursive: true, force: true });
}

/**
 * Writes the per-face pkjs tsconfig into the staging sandbox and returns its path.
 *
 * It roots at the repo root (so lib/ts, shared across faces, stays inside rootDir) and emits
 * into the sandbox's emit/. All paths are relative to the sandbox where the file is written.
 */
export function writeTsconfig(face: string, p: FacePaths): void {
  const tsconfig = {
    extends: path.relative(p.sandbox, PKJS_BASE_TSCONFIG).split(path.sep).join('/'),
    compilerOptions: { rootDir: '../..', outDir: 'emit' },
    include: [`../../watchfaces/${face}/src/pkjs/**/*.ts`, '../../lib/ts/**/*.ts'],
    exclude: [`../../watchfaces/${face}/src/pkjs/clay/builder/**`, '../../**/*.spec.ts'],
  };
  fs.mkdirSync(p.sandbox, { recursive: true });
  fs.writeFileSync(p.tsconfig, JSON.stringify(tsconfig, null, 2) + '\n');
}

/**
 * Compiles the pkjs runtime into the face's emit/.
 *
 * Runs tsc's own entry under this node rather than the node_modules/.bin shim, which on
 * Windows is a .cmd that would need a shell and bring its quoting rules along.
 */
export function compile(p: FacePaths): void {
  const result = spawnSync(process.execPath, [requireHost.resolve('typescript/bin/tsc'), '-p', p.tsconfig], {
    stdio: 'inherit',
  });

  // status is null when a signal killed it, so anything but a clean 0 has to fail here.
  // letting a compile error through would copy over a half-built emit/ and still exit 0
  if (result.status !== 0) {
    throw new Error(`tsc exited ${result.status === null ? `on signal ${result.signal}` : result.status}`);
  }
}

/** Every committed *.g.js under the face's src/pkjs/, as paths relative to that dir. */
export function findGenerated(p: FacePaths): string[] {
  const found: string[] = [];
  const walk = (current: string): void => {
    if (current === p.skipDir) {
      return;
    }
    for (const entry of fs.readdirSync(current, { withFileTypes: true })) {
      const full = path.join(current, entry.name);
      if (entry.isDirectory()) {
        walk(full);
      } else if (entry.name.endsWith('.g.js')) {
        found.push(path.relative(p.faceSrc, full));
      }
    }
  };
  walk(p.faceSrc);
  return found.sort();
}

/** Mirrors each generated component into emit/, keeping its path under the face's src/pkjs/. */
export function copyGenerated(p: FacePaths): string[] {
  const names = findGenerated(p);
  for (const name of names) {
    const to = path.join(p.emitPkjs, name);
    fs.mkdirSync(path.dirname(to), { recursive: true });
    fs.copyFileSync(path.join(p.faceSrc, name), to);
  }
  return names;
}

/**
 * Copies ical.js's prebuilt ES5 CommonJS file in beside the compiled calendar code.
 *
 * Throws when it is missing rather than letting the build carry on, because the require that
 * reaches for it would otherwise fail on the phone, where nobody is watching a build log.
 */
export function copyIcalJs(p: FacePaths): void {
  if (!fs.existsSync(ICALJS_FROM)) {
    throw new Error(`ical.js is missing at ${path.relative(ROOT, ICALJS_FROM)}, run npm install`);
  }
  fs.mkdirSync(path.dirname(p.icaljsTo), { recursive: true });
  fs.copyFileSync(ICALJS_FROM, p.icaljsTo);
}

function main(): void {
  const face = process.argv[2];
  if (!face) {
    console.error('usage: build-pkjs.ts <face>');
    process.exit(1);
  }

  const p = facePaths(face);
  cleanEmit(p);
  writeTsconfig(face, p);
  compile(p);
  const names = copyGenerated(p);
  copyIcalJs(p);

  const where = path.relative(ROOT, p.emitPkjs).split(path.sep).join('/');
  console.log(`built ${face} emit/ and copied ${names.length} generated components into ${where}/, plus ical.js`);
}

if (import.meta.main) {
  main();
}
