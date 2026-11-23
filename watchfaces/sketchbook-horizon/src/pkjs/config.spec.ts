/**
 * Specs for the settings page's scene list.
 *
 * The Scene picker's values and the scene keys in resources/theme-previews.json have to agree, or a
 * scene shows its palette names with no pictures under them. paf check reads only the manifest, so
 * nothing else compares the two.
 */
import fs from 'node:fs';
import path from 'node:path';
import { expect, test } from 'vitest';

import { sceneOptions } from './config';

const manifestPath = path.resolve(import.meta.dirname, '../../resources/theme-previews.json');
const manifest = JSON.parse(fs.readFileSync(manifestPath, 'utf8'));

/** A scene with no key in the manifest gets no previews, so its palette picker draws empty boxes. */
test('gives every scene on the picker a set of previews, and no more', () => {
  const keys = Object.keys(manifest.mapsByKey['scene-palette'].keys);

  const scenes = sceneOptions.map((scene) => String(scene.value));

  expect(keys).toEqual(scenes);
});
