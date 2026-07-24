/**
 * Specs for the emit builder.
 *
 * The copy step runs between tsc and the Pebble build to place any committed Clay *.g.js
 * components into emit/ beside the compiled output. LCARS ships no such components (its
 * config page uses the shared, compiled locationsearch component from lib/ts/clay, not a
 * self-contained .g.js bundle), so findGenerated legitimately returns nothing. These specs
 * pin that invariant: whatever it finds must be a .g.js under src/pkjs and never a
 * clay/builder piece.
 */

import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect } from 'vitest';
import { findGenerated, facePaths } from './build-pkjs';

const STARDATE = facePaths('lcars-stardate');

describe('findGenerated', () => {
  /** Everything it returns is a committed .g.js component (LCARS currently has none). */
  test('returns only .g.js component names', () => {
    const result = findGenerated(STARDATE);

    expect(result.every((name) => name.endsWith('.g.js'))).toBe(true);
  });

  /** Each one has to exist where the copy will read it, or the copy throws mid-build. */
  test('every name it returns resolves under the face src/pkjs', () => {
    const result = findGenerated(STARDATE).filter(
      (name) => !fs.existsSync(path.join(STARDATE.faceSrc, name))
    );

    expect(result).toEqual([]);
  });

  /**
   * clay/builder holds the pieces components are stitched from. They are bundled into
   * the .g.js already, so copying them would ship the sources twice.
   */
  test('skips the builder pieces', () => {
    const result = findGenerated(STARDATE).filter((name) => name.split(path.sep).join('/').startsWith('clay/builder/'));

    expect(result).toEqual([]);
  });
});
