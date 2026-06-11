#!/usr/bin/env node
/*
 * generate-frame.js - render an LCARS HTML frame into a Pebble background bitmap.
 *
 * Frame sources live under:
 *   watchfaces/<face>/frame/
 *
 * The frame is rendered in Firefox, captured at 4x resolution, then
 * downscaled to Pebble's native 200x228 display and written as a PNG resource.
 *
 * Firefox is required because Chromium incorrectly clips the LCARS elbow
 * carve-outs that use z-index:-1 pseudo-elements.
 *
 * Dynamic text and data are cleared before capture. The Pebble app draws
 * localized labels and live values at runtime.
 *
 * Output:
 *   watchfaces/<face>/resources/images/background*.png
 *
 * Re-run after editing:
 *   npm run gen:frame
 */
'use strict';

const path = require('path');
const fs = require('fs');
const { firefox } = require('playwright');
const sharp = require('sharp');

const SCREEN_W = 200;
const SCREEN_H = 228;

const REPO = path.resolve(__dirname, '..');

// args: [face] [frame] [--out <path>]
const argv = process.argv.slice(2);

let outOverride = null;
const positional = [];

for (let i = 0; i < argv.length; i++) {
  if (argv[i] === '--out') {
    outOverride = argv[++i];
    continue;
  }

  positional.push(argv[i]);
}

const FACE = positional[0] || 'stardate-emery';

// accept frame names with or without ".html"
const FRAME = (positional[1] || 'classic-standard')
  .replace(/\.html$/i, '');

const FACE_DIR = path.join(REPO, 'watchfaces', FACE);
const HTML = path.join(FACE_DIR, 'frame', `${FRAME}.html`);

// classic-standard -> background.png
// nemesis-blue-standard -> background-nemesis-blue.png
const base = FRAME.replace(/-standard$/, '');

const defaultOut =
  base === 'classic'
    ? 'background.png'
    : `background-${base}.png`;

const OUT = outOverride
  ? path.resolve(REPO, outOverride)
  : path.join(FACE_DIR, 'resources', 'images', defaultOut);

if (!fs.existsSync(HTML)) {
  console.error(
    `Frame HTML not found: ${HTML}\n` +
    'Usage: node tools/generate-frame.js [face] [frame] [--out <path>]'
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