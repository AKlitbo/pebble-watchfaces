#!/usr/bin/env node
/*
 * generate-frame.js - render an LCARS HTML frame into a Pebble background bitmap.

 * Face-local tooling: lives in this face's tools/ and operates on this face.
 *
 * Frame sources live under:
 *   <face>/frame/
 *
 * The frame is rendered in Firefox, captured at 4x resolution, then downscaled
 * to the face's native screen size (resolved from its targetPlatform) and
 * written as a PNG resource.
 *
 * Firefox is required because Chromium incorrectly clips the LCARS elbow
 * carve-outs that use z-index:-1 pseudo-elements.
 *
 * Dynamic text and data are cleared before capture. The Pebble app draws
 * localized labels and live values at runtime.
 *
 * Output:
 *   <face>/resources/images/background*.png
 *
 * Re-run after editing:
 *   npm run gen:frame
 */
'use strict';

const path = require('path');
const fs = require('fs');
const { firefox } = require('playwright');
const sharp = require('sharp');

// native screen size per Pebble platform (px). round platforms are square and
// drawn as an inscribed circle.
const PLATFORM_DIMS = {
  emery: { w: 200, h: 228 },
  gabbro: { w: 260, h: 260 },
  chalk: { w: 180, h: 180 },
  basalt: { w: 144, h: 168 },
  aplite: { w: 144, h: 168 },
  diorite: { w: 144, h: 168 },
  flint: { w: 144, h: 168 }
};

// resolve a face's native size from its package.json targetPlatforms (emery default)
function faceScreenSize(faceDir) {
  try {
    const pkg = JSON.parse(fs.readFileSync(path.join(faceDir, 'package.json'), 'utf8'));
    const platform = pkg.pebble && pkg.pebble.targetPlatforms && pkg.pebble.targetPlatforms[0];
    if (platform && PLATFORM_DIMS[platform]) {
      return PLATFORM_DIMS[platform];
    }
  } catch (err) {
    // no package.json yet (a face being scaffolded) - fall through to the default
  }

  return PLATFORM_DIMS.emery;
}

// this tool lives at <face>/tools, so the face is one level up and the repo
// root three. REPO is only used to resolve --out and for tidy log paths
const FACE_DIR = path.resolve(__dirname, '..');
const REPO = path.resolve(__dirname, '..', '..', '..');
const FACE = path.basename(FACE_DIR);

// args: [frame] [--out <path>] [--preview]
const argv = process.argv.slice(2);

let outOverride = null;
let preview = false;
const positional = [];

for (let i = 0; i < argv.length; i++) {
  if (argv[i] === '--out') {
    outOverride = argv[++i];
    continue;
  }

  // --preview keeps the live text (labels/time/values) and drops a mockup under
  // .tmp/mockups so you can see the whole watch; the default bake is chrome-only
  if (argv[i] === '--preview') {
    preview = true;
    continue;
  }

  positional.push(argv[i]);
}

// accept frame names with or without ".html"
const FRAME = (positional[0] || 'classic-standard')
  .replace(/\.html$/i, '');

const HTML = path.join(FACE_DIR, 'frame', `${FRAME}.html`);

const { w: SCREEN_W, h: SCREEN_H } = faceScreenSize(FACE_DIR);

// classic-standard -> background.png
// nemesis-blue-standard -> background-nemesis-blue.png
const base = FRAME.replace(/-standard$/, '');

const defaultOut =
  base === 'classic'
    ? 'background.png'
    : `background-${base}.png`;

const OUT = outOverride
  ? path.resolve(REPO, outOverride)
  : preview
    ? path.join(REPO, '.tmp', 'mockups', `${FACE}-${FRAME}-preview.png`)
    : path.join(FACE_DIR, 'resources', 'images', defaultOut);

if (!fs.existsSync(HTML)) {
  console.error(
    `Frame HTML not found: ${HTML}\n` +
    'Usage: node tools/generate-frame.js [frame] [--out <path>]'
  );
  process.exit(1);
}

(async () => {
  const browser = await firefox.launch();

  const page = await browser.newPage({
    viewport: {
      width: 1920,
      height: 1080
    },
    // 4x supersampling here so sharp can downscale cleanly later to prevent aliased edges on the pebble display
    deviceScaleFactor: 4
  });

  await page.goto(
    // playwright requires forward slashes for file uris even on windows
    'file://' + HTML.replace(/\\/g, '/'),
    { waitUntil: 'networkidle' }
  );

  // the shipped frame is chrome-only: the app draws labels/time/values at runtime.
  // --preview skips this so the mockup shows the whole watch.
  if (!preview) {
    await page.evaluate(() => {
      document
        .querySelectorAll('.time, .banner, .data')
        .forEach((el) => {
          el.textContent = '';
        });

      document
        .querySelectorAll('.lcars-text-bar h3')
        .forEach((el) => {
          el.style.display = 'none';
        });
    });
  }

  // networkidle doesnt guarantee custom webfonts are fully painted so we manually await the font api
  await page.evaluate(() => document.fonts && document.fonts.ready);
  await page.waitForTimeout(200);

  const screenshot = await page.locator('.viewport').screenshot();

  await browser.close();

  fs.mkdirSync(path.dirname(OUT), { recursive: true });

  await sharp(screenshot)
    .resize(SCREEN_W, SCREEN_H, {
      // lanczos3 kernel prevents nasty moire patterns when downscaling the sharp lcars geometry
      kernel: 'lanczos3'
    })
    .png()
    .toFile(OUT);

  console.log(
    `Rendered ${path.relative(REPO, HTML)} -> ` +
    `${path.relative(REPO, OUT)} (${SCREEN_W}x${SCREEN_H})`
  );
})().catch((err) => {
  console.error(err);
  process.exit(1);
});