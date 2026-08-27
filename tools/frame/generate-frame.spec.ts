/**
 * Specs for the pure parts of the frame generator. The render pipeline itself drives
 * Firefox + sharp and is left to integration use (npm run gen:frame).
 */
import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect, vi, afterEach } from 'vitest';
import { capColors, faceScreenSize, PLATFORM_DIMS } from './generate-frame';

/** Flatten [r, g, b, a] pixels into the raw buffer sharp hands over. */
function raw(pixels: number[][]): Uint8Array {
  return Uint8Array.from(pixels.flat());
}

/** The distinct colours a raw buffer holds, as "r,g,b,a" strings. */
function colorsIn(buffer: Uint8Array): Set<string> {
  const colors = new Set<string>();
  for (let i = 0; i < buffer.length; i += 4) {
    colors.add(Array.from(buffer.slice(i, i + 4)).join(','));
  }

  return colors;
}

describe('PLATFORM_DIMS', () => {
  test('emery is the 200x228 target this face ships', () => {
    expect(PLATFORM_DIMS.emery).toEqual({ w: 200, h: 228 });
  });
});

describe('faceScreenSize', () => {
  afterEach(() => {
    vi.restoreAllMocks();
  });

  test('resolves the size from the appinfo first target platform', () => {
    vi.spyOn(fs, 'readFileSync').mockReturnValue(JSON.stringify({ targetPlatforms: ['chalk'] }));

    expect(faceScreenSize('whatever.json')).toEqual(PLATFORM_DIMS.chalk);
  });

  test('falls back to emery when the appinfo is missing or unreadable', () => {
    expect(faceScreenSize(path.join('no', 'such', 'appinfo.json'))).toEqual(PLATFORM_DIMS.emery);
  });

  test('falls back to emery for an unknown platform', () => {
    vi.spyOn(fs, 'readFileSync').mockReturnValue(JSON.stringify({ targetPlatforms: ['nope'] }));

    expect(faceScreenSize('whatever.json')).toEqual(PLATFORM_DIMS.emery);
  });
});

describe('capColors', () => {
  test('leaves the anti-aliasing alone when the bake is already under the cap', () => {
    const buffer = raw([
      [0, 0, 0, 255],
      [10, 200, 90, 255],
      [255, 255, 255, 255],
    ]);

    const result = capColors(buffer, 16);

    expect(Array.from(result)).toEqual([0, 0, 0, 255, 10, 200, 90, 255, 255, 255, 255, 255]);
  });

  test('folds the rarest colour into its nearest neighbour to meet the cap', () => {
    // three blacks, three whites, and one stray pixel that snaps to its own Pebble-64 bucket
    const buffer = raw([
      [0, 0, 0, 255],
      [0, 0, 0, 255],
      [0, 0, 0, 255],
      [255, 255, 255, 255],
      [255, 255, 255, 255],
      [255, 255, 255, 255],
      [250, 250, 200, 255],
    ]);

    const result = capColors(buffer, 2);

    expect(colorsIn(result)).toEqual(new Set(['0,0,0,255', '255,255,255,255']));
  });

  test('counts colours by their Pebble-64 bucket, not by their exact value', () => {
    // both pixels snap to the same bucket, so a cap of one leaves them where they are
    const buffer = raw([
      [250, 250, 250, 255],
      [255, 255, 255, 255],
    ]);

    const result = capColors(buffer, 1);

    expect(Array.from(result)).toEqual([250, 250, 250, 255, 255, 255, 255, 255]);
  });
});
