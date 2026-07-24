/**
 * Specs for the pure parts of the frame generator. The render pipeline itself drives
 * Firefox + sharp and is left to integration use (npm run gen:frame).
 */
import fs from 'node:fs';
import path from 'node:path';
import { describe, test, expect, vi, afterEach } from 'vitest';
import { faceScreenSize, PLATFORM_DIMS } from './generate-frame';

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
