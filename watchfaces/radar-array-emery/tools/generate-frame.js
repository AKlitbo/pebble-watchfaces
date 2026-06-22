#!/usr/bin/env node
/**
 * Render the radar-array HTML frame into Pebble background bitmaps.
 *
 * Face-local tooling: lives in this face's tools/ and operates on this face.
 *
 * One frame/radar-array.html holds the layout, and each theme is a palette-only
 * stylesheet at frame/css/theme_<name>.css. The frame is rendered in Firefox at 4x,
 * downscaled to the face's native screen size, and written as a PNG. Dynamic text
 * and the battery are cleared before capture since the app draws those live.
 *
 * Usage:
 *   node tools/generate-frame.js [frame] [--theme <name>|all] [--out <path>] [--preview]
 *
 *   (no --theme)  -> images/background-radar-array.png (the theme linked in the HTML)
 *   --theme neon  -> images/background-neon.png
 *   --theme all   -> one background-<name>.png per frame/css/theme_*.css
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
const CSS_DIR = path.join(FACE_DIR, 'frame', 'css');

// args: [frame] [--theme <name>|all] [--out <path>] [--preview]
const argv = process.argv.slice(2);

let outOverride = null;
let preview = false;
let theme = null;
const positional = [];

for (let i = 0; i < argv.length; i++) {
  if (argv[i] === '--out') {
    outOverride = argv[++i];
    continue;
  }

  // --preview keeps the live text (labels/time/values) and drops a mockup under
  // .tmp/mockups so you can see the whole watch (the default bake is chrome-only)
  if (argv[i] === '--preview') {
    preview = true;
    continue;
  }

  // --theme <name> swaps the palette stylesheet, and --theme all bakes every theme
  if (argv[i] === '--theme') {
    theme = argv[++i];
    continue;
  }

  positional.push(argv[i]);
}

// accept frame names with or without ".html"
const FRAME = (positional[0] || 'radar-array').replace(/\.html$/i, '');
const HTML = path.join(FACE_DIR, 'frame', `${FRAME}.html`);
const FILE_URL = 'file://' + HTML.replace(/\\/g, '/');  // playwright wants forward slashes
const { w: SCREEN_W, h: SCREEN_H } = faceScreenSize(FACE_DIR);

if (!fs.existsSync(HTML)) {
  console.error(
    `Frame HTML not found: ${HTML}\n` +
    'Usage: node tools/generate-frame.js [frame] [--theme <name>|all] [--out <path>] [--preview]'
  );
  process.exit(1);
}

// every theme_<name>.css under frame/css, by name
function discoverThemes() {
  return fs.readdirSync(CSS_DIR)
    .map((file) => file.match(/^theme_(.+)\.css$/))
    .filter(Boolean)
    .map((match) => match[1])
    .sort();
}

// the set to render: a named theme, every theme, or null (the HTML's linked theme)
let themes;
if (theme === 'all') {
  themes = discoverThemes();
  if (themes.length === 0) {
    console.error(`No theme_*.css files found in ${CSS_DIR}`);
    process.exit(1);
  }
} else if (theme) {
  themes = [theme];
} else {
  themes = [null];
}

// base 'radar-array' keeps the default bitmap name (background-radar-array.png) for the
// untheme-d build, and a named theme lands at background-<name>.png
const base = FRAME.replace(/-standard$/, '');

function outFor(themeName) {
  if (preview) {
    const suffix = themeName ? `${FRAME}-${themeName}` : FRAME;
    return path.join(REPO, '.tmp', 'mockups', `${FACE}-${suffix}-preview.png`);
  }

  if (outOverride && themes.length === 1) {
    return path.resolve(REPO, outOverride);
  }

  const name = themeName ? `background-${themeName}.png` : `background-${base}.png`;
  return path.join(FACE_DIR, 'resources', 'images', name);
}

// swap the linked palette stylesheet for the requested theme
async function applyTheme(page, themeName) {
  const themeCss = path.join(CSS_DIR, `theme_${themeName}.css`);
  if (!fs.existsSync(themeCss)) {
    throw new Error(`Theme stylesheet not found: ${themeCss}`);
  }

  await page.evaluate(() => {
    document.querySelectorAll('link[href*="theme_"]').forEach((link) => link.remove());
  });

  await page.addStyleTag({ path: themeCss });
}

// strip the live readouts so the bake is pure graphics chrome (the app draws the
// labels/time/values and fills the battery at runtime)
async function clearLiveLayers(page) {
  await page.evaluate(() => {
    document
      .querySelectorAll('.time, .meridiem, .banner, .data, .lbl, .pwr-label, .tlabel')
      .forEach((el) => {
        el.textContent = '';
      });

    document
      .querySelectorAll('.batt')
      .forEach((el) => {
        el.style.display = 'none';
      });
  });
}

async function renderTheme(page, themeName) {
  await page.goto(FILE_URL, { waitUntil: 'networkidle' });

  if (themeName) {
    await applyTheme(page, themeName);
  }

  if (!preview) {
    await clearLiveLayers(page);
  }

  // networkidle doesnt guarantee custom webfonts are painted, so await the font api
  await page.evaluate(() => document.fonts && document.fonts.ready);
  await page.waitForTimeout(200);

  const screenshot = await page.locator('.viewport').screenshot();
  const out = outFor(themeName);
  fs.mkdirSync(path.dirname(out), { recursive: true });

  await sharp(screenshot)
    // lanczos3 prevents moire when downscaling the sharp scope geometry
    .resize(SCREEN_W, SCREEN_H, { kernel: 'lanczos3' })
    .png()
    .toFile(out);

  console.log(
    `Rendered ${path.relative(REPO, HTML)}${themeName ? ` [${themeName}]` : ''} -> ` +
    `${path.relative(REPO, out)} (${SCREEN_W}x${SCREEN_H})`
  );
}

(async () => {
  const browser = await firefox.launch();

  const page = await browser.newPage({
    viewport: {
      width: 1920,
      height: 1080
    },
    // 4x supersampling so sharp can downscale cleanly and avoid aliased edges
    deviceScaleFactor: 4
  });

  for (const themeName of themes) {
    await renderTheme(page, themeName);
  }

  await browser.close();
})().catch((err) => {
  console.error(err);
  process.exit(1);
});
