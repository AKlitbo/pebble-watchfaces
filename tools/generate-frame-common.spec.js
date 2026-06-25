/**
 * Specs for the shared frame-bake engine's path/size resolution.
 *
 * faceScreenSize and findRepoRoot are the pure parts of generate-frame-common:
 * they pick the bitmap's native dimensions and locate the repo root before any
 * browser launches. Pick the wrong size and every face bakes its chrome at the
 * wrong resolution, so the resolution rules and their fallbacks are pinned here.
 * The run() orchestrator drives Firefox and sharp and is left to integration use.
 */

import { describe, test, expect, beforeEach, afterEach } from 'vitest';
import fs from 'fs';
import os from 'os';
import path from 'path';
import { faceScreenSize, findRepoRoot, PLATFORM_DIMS } from './generate-frame-common.js';

let tmpRoot;

beforeEach(() => {
  tmpRoot = fs.mkdtempSync(path.join(os.tmpdir(), 'frame-common-'));
});

afterEach(() => {
  fs.rmSync(tmpRoot, { recursive: true, force: true });
});

// write a face package.json with the given first targetPlatform (omit for none)
function writeFacePackage(faceDir, platform) {
  fs.mkdirSync(faceDir, { recursive: true });
  
  const pkg = platform ? { pebble: { targetPlatforms: [platform] } } : { pebble: {} };
  fs.writeFileSync(path.join(faceDir, 'package.json'), JSON.stringify(pkg));
}

describe('faceScreenSize', () => {
  /** Baking at a non-emery platform's size: a face on gabbro must get 260x260, not the emery default. */
  test('returns the dimensions of the package targetPlatform', () => {
    const faceDir = path.join(tmpRoot, 'gabbro-face');
    writeFacePackage(faceDir, 'gabbro');

    const result = faceScreenSize(faceDir);

    expect(result).toEqual(PLATFORM_DIMS.gabbro);
  });

  /** A face scaffolded before its package.json exists must still bake at a real size, not crash. */
  test('falls back to emery when no package.json is present', () => {
    const faceDir = path.join(tmpRoot, 'bare-face');
    fs.mkdirSync(faceDir, { recursive: true });

    const result = faceScreenSize(faceDir);

    expect(result).toEqual(PLATFORM_DIMS.emery);
  });

  /** An unrecognized platform string must not bake at undefined width/height - fall back to emery. */
  test('falls back to emery when the platform is not in the table', () => {
    const faceDir = path.join(tmpRoot, 'unknown-face');
    writeFacePackage(faceDir, 'obsidian');

    const result = faceScreenSize(faceDir);

    expect(result).toEqual(PLATFORM_DIMS.emery);
  });

  /** A malformed package.json must degrade to the default rather than throw mid-bake. */
  test('falls back to emery when package.json is unparseable', () => {
    const faceDir = path.join(tmpRoot, 'broken-face');
    fs.mkdirSync(faceDir, { recursive: true });
    fs.writeFileSync(path.join(faceDir, 'package.json'), '{ not json');

    const result = faceScreenSize(faceDir);

    expect(result).toEqual(PLATFORM_DIMS.emery);
  });
});

describe('findRepoRoot', () => {
  /** Output paths resolve against the repo root, so the walk must find the ancestor holding watchfaces/ and tools/. */
  test('returns the nearest ancestor that holds both watchfaces and tools', () => {
    const repo = path.join(tmpRoot, 'repo');
    fs.mkdirSync(path.join(repo, 'watchfaces'), { recursive: true });
    fs.mkdirSync(path.join(repo, 'tools'), { recursive: true });

    const faceDir = path.join(repo, 'watchfaces', 'some-face');
    fs.mkdirSync(faceDir, { recursive: true });

    const result = findRepoRoot(faceDir);

    expect(result).toBe(repo);
  });

  /** A directory with only one of the two markers is not the root - the walk must climb past it. */
  test('skips an ancestor that has watchfaces but no tools', () => {
    const repo = path.join(tmpRoot, 'repo');
    fs.mkdirSync(path.join(repo, 'watchfaces'), { recursive: true });
    fs.mkdirSync(path.join(repo, 'tools'), { recursive: true });

    const decoy = path.join(repo, 'watchfaces', 'decoy');
    fs.mkdirSync(path.join(decoy, 'watchfaces'), { recursive: true });

    const faceDir = path.join(decoy, 'face');
    fs.mkdirSync(faceDir, { recursive: true });

    const result = findRepoRoot(faceDir);

    expect(result).toBe(repo);
  });

  /** With no matching ancestor the walk must hand back the start dir, not crash climbing past the filesystem root. */
  test('falls back to the start dir when no ancestor matches', () => {
    const faceDir = path.join(tmpRoot, 'loose', 'face');
    fs.mkdirSync(faceDir, { recursive: true });

    const result = findRepoRoot(faceDir);

    expect(result).toBe(faceDir);
  });
});
