/**
 * Specs for the clay-components generator.
 *
 * The generator bundles each component's pieces into one initialize the Clay
 * webview can run on its own, so the checks here pin the entry it hands esbuild,
 * the CSS squeeze, and the finished wrapper. The staleness check regenerates
 * every component and compares it to the committed file, so an edited piece
 * cannot ship without `npm run gen:clay`.
 */

import fs from 'node:fs';
import path from 'node:path';
import { createRequire } from 'node:module';
import { describe, test, expect } from 'vitest';
import {
  rootsFor,
  findManifests,
  findInitPiece,
  buildEntrySource,
  minifyCss,
  buildComponentSource,
} from './generate-components';

// the manifests are loaded by path, the same way the generator does it
const requireManifest = createRequire(import.meta.url);

// the generator is the family's, so it is always asked about one face. gridlock is the reference
const ROOTS = rootsFor('gridlock');

/** The first manifest and its directory, a real recipe to exercise the helpers against. */
function firstManifest() {
  const manifestPath = findManifests(ROOTS)[0];
  return { manifest: requireManifest(manifestPath).default, dir: path.dirname(manifestPath) };
}

describe('findInitPiece', () => {
  /** Without the init piece the bundle would have no entry to call, so a missing one must throw. */
  test('finds the manifest piece named init', () => {
    const { manifest } = firstManifest();

    const result = findInitPiece(manifest);

    expect(path.basename(result)).toBe('init');
  });

  /** A manifest with no init piece would bundle to a component that never wires itself up. */
  test('throws when no piece is named init', () => {
    const manifest = { name: 'broken', pieces: ['ts/layout/geometry'] };

    const call = () => findInitPiece(manifest);

    expect(call).toThrow(/no piece named init/);
  });
});

describe('buildEntrySource', () => {
  /** A dropped piece would tree shake out of the bundle and vanish from the config page. */
  test('requires every piece and re-exports the init piece', () => {
    const manifest = { pieces: ['ts/shared/thumbs', 'ts/layout/init'] };

    const result = buildEntrySource(manifest, 'ts/layout/init');

    expect(result).toContain('require("./ts/shared/thumbs");');
    expect(result).toContain('module.exports = require("./ts/layout/init");');
  });
});

describe('minifyCss', () => {
  /** An over-eager squeeze that eats a space inside a value breaks the rule on the config page. */
  test('strips comments and tightens punctuation without touching values', () => {
    const css = '/* a note */\n.a > .b:active {\n  color: #fff;\n  border: 1px solid rgba(0, 0, 0, 0.6);\n}\n';

    const result = minifyCss(css);

    expect(result).toBe('.a>.b:active{color:#fff;border:1px solid rgba(0,0,0,0.6)}');
  });
});

describe('buildComponentSource', () => {
  /** The manipulator calls init with the component as this, so the call has to survive into the wrapper. */
  test('wraps the bundle and calls init with the component this', async () => {
    const manifestPath = findManifests(ROOTS)[0];

    const result = await buildComponentSource(manifestPath, ROOTS);

    expect(result.source).toContain('__clayComponent.init.call(this);');
  });
});

describe('generated components', () => {
  findManifests(ROOTS).forEach((manifestPath) => {
    /** A piece edit without gen:clay would ship a config page that ignores the change. */
    test(`${manifestPath.replace(/\\/g, '/').split('/').pop()} output is not stale`, async () => {
      const built = await buildComponentSource(manifestPath, ROOTS);

      const committed = fs.readFileSync(built.output, 'utf8').replace(/\r\n/g, '\n');

      expect(committed).toBe(built.source);
    });
  });
});
