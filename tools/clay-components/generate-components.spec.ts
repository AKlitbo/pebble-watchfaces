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
import { listFaces } from '../faces';
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

// the helper tests only need one real recipe to work against. gridlock is the reference
const ROOTS = rootsFor('gridlock');

/**
 * Every face that ships a Clay builder, with the manifests it builds.
 *
 * Discovered rather than listed, so a new face carrying a builder is guarded the day it lands
 * instead of the day someone remembers to add it here. A face with no builder yields no
 * manifests and contributes no tests.
 */
// listFaces yields a family-relative path (mosaic/gridlock) but the generator is handed a bare name
const BUILDER_FACES = listFaces()
  .map((rel) => path.basename(rel))
  .map((face) => ({ face, roots: rootsFor(face) }))
  .map((entry) => ({ ...entry, manifests: findManifests(entry.roots) }))
  .filter((entry) => entry.manifests.length > 0);

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
  /**
   * The staleness checks below are generated from what the discovery found, so a discovery that
   * quietly returned nothing would leave this suite green with nothing in it. Three faces ship a
   * builder today, and losing one has to fail here rather than pass silently.
   */
  test('every face carrying a builder is covered', () => {
    const result = BUILDER_FACES.map((entry) => entry.face);

    expect(result.length).toBeGreaterThanOrEqual(3);
    expect(result).toContain('gridlock');
    expect(result).toContain('sidereel');
    expect(result).toContain('lcars-stardate');
  });

  BUILDER_FACES.forEach(({ face, roots, manifests }) => {
    manifests.forEach((manifestPath) => {
      const name = manifestPath.replace(/\\/g, '/').split('/').pop();

      /** A piece edit without gen:clay would ship a config page that ignores the change. */
      test(`${face}: ${name} output is not stale`, async () => {
        const built = await buildComponentSource(manifestPath, roots);

        const committed = fs.readFileSync(built.output, 'utf8').replace(/\r\n/g, '\n');

        expect(committed).toBe(built.source);
      });
    });
  });
});
