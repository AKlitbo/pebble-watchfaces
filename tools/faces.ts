/**
 * Finding a face's source directory.
 *
 * Faces sit either straight under watchfaces/ or one level deeper inside a family folder that
 * also holds the code those faces share (watchfaces/sketchbook/{core,ridgeline,...}). A face is
 * simply a directory holding config/pebble.appinfo.json, which is what keeps a family's core/
 * from being mistaken for one and means nothing has to be registered anywhere.
 *
 * A face's *name* stays unique across the repo whatever depth it sits at, so everything
 * downstream of here (targets/<face>/, the CI matrix, release tags) is unaffected by the shape.
 */
import fs from 'fs';
import path from 'path';

const ROOT = path.resolve(import.meta.dirname, '..');
const WATCHFACES = path.join(ROOT, 'watchfaces');
const APPINFO = path.join('config', 'pebble.appinfo.json');

/** Whether a directory is a face rather than, say, a family's shared core. */
function isFace(dir: string): boolean {
  return fs.existsSync(path.join(dir, APPINFO));
}

/**
 * Every face in the repo, as a path relative to watchfaces/ (e.g. `radar-array`,
 * `sketchbook/ridgeline`). Sorted, so build order is stable.
 */
export function listFaces(): string[] {
  const found: string[] = [];

  for (const entry of fs.readdirSync(WATCHFACES, { withFileTypes: true })) {
    if (!entry.isDirectory()) {
      continue;
    }

    if (isFace(path.join(WATCHFACES, entry.name))) {
      found.push(entry.name);
      continue;
    }

    // a family folder: its faces are the children that carry an appinfo
    for (const child of fs.readdirSync(path.join(WATCHFACES, entry.name), { withFileTypes: true })) {
      if (child.isDirectory() && isFace(path.join(WATCHFACES, entry.name, child.name))) {
        found.push(`${entry.name}/${child.name}`);
      }
    }
  }

  return found.sort();
}

/** A face's path relative to watchfaces/, by name. Throws when there is no such face. */
export function faceRelative(face: string): string {
  const match = listFaces().find((rel) => path.basename(rel) === face);
  if (!match) {
    throw new Error(`no such face: ${face} (nothing under watchfaces/ carries its ${APPINFO})`);
  }
  return match;
}

/** A face's absolute source directory, by name. */
export function faceDir(face: string): string {
  return path.join(WATCHFACES, faceRelative(face));
}
