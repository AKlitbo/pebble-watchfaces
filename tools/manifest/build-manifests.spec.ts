/**
 * Specs for the manifest builder's pure steps.
 *
 * buildMedia owns the per-target media list: which entry gets the menuIcon flag, and the
 * deep-clone that keeps one target's flag from bleeding onto the other. buildManifest is the
 * package.json wire shape pebble build reads. main()'s file I/O is glue, left to the build.
 */

import { describe, test, expect } from 'vitest';
import { buildMedia, buildManifest } from './build-manifests';
import type { SharedAppinfo } from './build-manifests';

// a fresh config per test so the mutation check can't be masked by an earlier test
function makeConfig(): SharedAppinfo {
  return {
    displayName: 'Test',
    uuid: 'uuid-1',
    sdkVersion: '3',
    enableMultiJS: true,
    targetPlatforms: ['emery'],
    capabilities: ['health'],
    messageKeys: ['ALPHA', 'BETA'],
    resources: {
      media: [
        { type: 'bitmap', name: 'ICON_ONE', file: 'one.png' },
        { type: 'bitmap', name: 'ICON_TWO', file: 'two.png' },
      ],
    },
  };
}

describe('buildMedia', () => {
  /** The app's launcher icon is chosen by this flag, so a missed mark ships an app with no menu icon. */
  test('marks the target menu icon entry and leaves the rest alone', () => {
    const config = makeConfig();
    const target = { name: 'app', watchface: false, menuIcon: 'ICON_TWO' };

    const result = buildMedia(config, target);

    expect(result.find((item) => item.name === 'ICON_TWO').menuIcon).toBe(true);
    expect(result.find((item) => item.name === 'ICON_ONE').menuIcon).toBeUndefined();
  });

  /** A menu icon naming a missing resource must fail the build, not ship an app pointing at nothing. */
  test('throws when the menu icon is not in the media list', () => {
    const config = makeConfig();
    const target = { name: 'app', watchface: false, menuIcon: 'ICON_MISSING' };

    const call = () => buildMedia(config, target);

    expect(call).toThrow(/ICON_MISSING is not in the media list/);
  });

  /** main() reuses one config across both targets, so a mutation would leak the app's menu icon onto the watchface. */
  test('does not mutate the input media list', () => {
    const config = makeConfig();
    const target = { name: 'app', watchface: false, menuIcon: 'ICON_TWO' };

    buildMedia(config, target);

    expect(config.resources.media.find((item) => item.name === 'ICON_TWO').menuIcon).toBeUndefined();
  });

  /** The watchface target declares no menu icon, so nothing in its media list should be flagged. */
  test('leaves media unmarked when the target has no menu icon', () => {
    const config = makeConfig();
    const target = { name: 'face', watchface: true };

    const result = buildMedia(config, target);

    expect(result.some((item) => item.menuIcon)).toBe(false);
  });
});

describe('buildManifest', () => {
  /** author and version come from the root package, not the build config, so a wrong source ships a mislabeled release. */
  test('takes name from the target and author/version from the root package', () => {
    const config = makeConfig();
    const rootPkg = { author: 'Null Syntax', version: '1.2.3' };
    const target = { name: 'stardate-app', watchface: false };

    const result = buildManifest(config, rootPkg, target);

    expect(result.name).toBe('stardate-app');
    expect(result.author).toBe('Null Syntax');
    expect(result.version).toBe('1.2.3');
  });

  /** The watchface flag lives at pebble.watchapp.watchface, and wrong nesting installs an app as a face or vice versa. */
  test('nests the watchface flag under pebble.watchapp', () => {
    const config = makeConfig();
    const rootPkg = { author: 'x', version: '0' };
    const target = { name: 'face', watchface: true };

    const result = buildManifest(config, rootPkg, target);

    expect(result.pebble.watchapp.watchface).toBe(true);
  });

  /** The media must route through buildMedia so the menu icon is marked in the manifest pebble reads. */
  test('routes media through buildMedia so the menu icon is marked', () => {
    const config = makeConfig();
    const rootPkg = { author: 'x', version: '0' };
    const target = { name: 'app', watchface: false, menuIcon: 'ICON_ONE' };

    const result = buildManifest(config, rootPkg, target);

    expect(result.pebble.resources.media.find((item) => item.name === 'ICON_ONE').menuIcon).toBe(true);
  });
});
