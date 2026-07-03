#!/usr/bin/env node
/*
 * generate-frame-common.js - shared engine behind each face's generate-frame.js.
 *
 * Every face bakes its HTML chrome into a Pebble background bitmap the same way:
 * render frame/<name>.html in Firefox at the face's deviceScaleFactor, strip the
 * live readouts (the app draws those at runtime), then resize to the face's native
 * screen size with a lanczos3 kernel and write a PNG. A scale above 1 supersamples
 * for clean downscaling of curved chrome; true scale (1) bakes axis-aligned chrome
 * pixel-faithfully.
 *
 * Firefox (not Chromium) because Chromium clips the LCARS elbow carve-outs that
 * use z-index:-1 pseudo-elements.
 *
 * A face supplies only the parts that differ via run(config):
 *   toolsDir            __dirname of the calling wrapper (resolves the face dir)
 *   defaultFrame        frame name when none is passed on the CLI
 *   defaultScale        deviceScaleFactor (supersample factor); --scale overrides
 *   supportsTheme       whether --theme <name>|all swaps a palette stylesheet
 *   bareBackgroundBase  base name that bakes to background.png instead of
 *                       background-<base>.png (else null)
 *   clearTextSelectors  selectors whose elements have their text cleared
 *   hideSelectors       selectors whose elements are hidden (display:none)
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

// walk up from the face dir to the repo root. the nearest ancestor holding both
// watchfaces/ and tools/. worked out at run time not hard-coded so the tool survives a move
function findRepoRoot(startDir) {
  let dir = startDir;

  while (true) {
    if (fs.existsSync(path.join(dir, 'watchfaces')) && fs.existsSync(path.join(dir, 'tools'))) {
      return dir;
    }

    const parent = path.dirname(dir);
    if (parent === dir) {
      break;  // hit the filesystem root without a match
    }

    dir = parent;
  }

  return startDir;  // fallback - keep going rather than crash
}

// every theme_<name>.css under frame/css by name
function discoverThemes(cssDir) {
  return fs.readdirSync(cssDir)
    .map((file) => file.match(/^theme_(.+)\.css$/))
    .filter(Boolean)
    .map((match) => match[1])
    .sort();
}

// swap the linked palette stylesheet for the requested theme
async function applyTheme(page, cssDir, themeName) {
  const themeCss = path.join(cssDir, `theme_${themeName}.css`);
  if (!fs.existsSync(themeCss)) {
    throw new Error(`Theme stylesheet not found: ${themeCss}`);
  }

  await page.evaluate(() => {
    document.querySelectorAll('link[href*="theme_"]').forEach((link) => link.remove());
  });

  await page.addStyleTag({ path: themeCss });
}

// strip the live readouts so the bake is pure graphics chrome (the app draws the
// labels/time/values and fills any gauges at runtime). selectors come as arrays
// querySelectorAll wants one comma-joined string and an empty list is a no-op
async function clearLiveLayers(page, clearTextSelectors, hideSelectors) {
  const clearSel = clearTextSelectors.join(', ');
  const hideSel = hideSelectors.join(', ');

  await page.evaluate(({ clearSel, hideSel }) => {
    if (clearSel) {
      document.querySelectorAll(clearSel).forEach((el) => { el.textContent = ''; });
    }

    if (hideSel) {
      document.querySelectorAll(hideSel).forEach((el) => { el.style.display = 'none'; });
    }
  }, { clearSel, hideSel });
}

function run(config) {
  const faceDir = path.resolve(config.toolsDir, '..');
  const repo = findRepoRoot(faceDir);
  const cssDir = path.join(faceDir, 'frame', 'css');
  const { w: screenW, h: screenH } = faceScreenSize(faceDir);

  const themeUsage = config.supportsTheme ? ' [--theme <name>|all]' : '';
  const usage = `Usage: node tools/generate-frame.js [frame]${themeUsage} [--out <path>] [--scale <n>]`;

  // args: [frame] [--out <path>] [--scale <n>] [--theme <name>|all]
  const argv = process.argv.slice(2);

  let outOverride = null;
  let scale = config.defaultScale;
  let theme = null;
  const positional = [];

  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--out') {
      outOverride = argv[++i];
      continue;
    }

    if (argv[i] === '--scale') {
      scale = parseInt(argv[++i], 10) || config.defaultScale;
      continue;
    }

    // --theme <name> swaps the palette stylesheet and --theme all bakes every theme
    if (argv[i] === '--theme') {
      theme = argv[++i];
      continue;
    }

    positional.push(argv[i]);
  }

  if (!config.supportsTheme) {
    theme = null;  // faces without themes ignore a stray --theme
  }

  // accept frame names with or without ".html"
  const frame = (positional[0] || config.defaultFrame).replace(/\.html$/i, '');
  const html = path.join(faceDir, 'frame', `${frame}.html`);
  const fileUrl = 'file://' + html.replace(/\\/g, '/');  // playwright wants forward slashes

  if (!fs.existsSync(html)) {
    console.error(`Frame HTML not found: ${html}\n${usage}`);
    process.exit(1);
  }

  // the set to render is a named theme or every theme or null (the HTML's linked theme)
  let themes;
  if (theme === 'all') {
    themes = discoverThemes(cssDir);
    if (themes.length === 0) {
      console.error(`No theme_*.css files found in ${cssDir}`);
      process.exit(1);
    }
  } else if (theme) {
    themes = [theme];
  } else {
    themes = [null];
  }

  // base drops a trailing -standard. a face can map one base to the bare
  // background.png (bareBackgroundBase). everything else lands at background-<base>.png
  const base = frame.replace(/-standard$/, '');

  function outFor(themeName) {
    if (outOverride && themes.length === 1) {
      return path.resolve(repo, outOverride);
    }

    let name;
    if (themeName) {
      name = `background-${themeName}.png`;
    } else if (config.bareBackgroundBase && base === config.bareBackgroundBase) {
      name = 'background.png';
    } else {
      name = `background-${base}.png`;
    }

    return path.join(faceDir, 'resources', 'images', name);
  }

  async function renderTheme(page, themeName) {
    await page.goto(fileUrl, { waitUntil: 'networkidle' });

    if (themeName) {
      await applyTheme(page, cssDir, themeName);
    }

    await clearLiveLayers(page, config.clearTextSelectors, config.hideSelectors);

    // networkidle doesnt guarantee custom webfonts are painted so await the font api
    await page.evaluate(() => document.fonts && document.fonts.ready);
    await page.waitForTimeout(200);

    const screenshot = await page.locator('.viewport').screenshot();
    const out = outFor(themeName);
    fs.mkdirSync(path.dirname(out), { recursive: true });

    await sharp(screenshot)
      // lanczos3 prevents moire when downscaling the sharp chrome geometry
      .resize(screenW, screenH, { kernel: 'lanczos3' })
      .png()
      .toFile(out);

    console.log(
      `Rendered ${path.relative(repo, html)}${themeName ? ` [${themeName}]` : ''} -> ` +
      `${path.relative(repo, out)} (${screenW}x${screenH})`
    );
  }

  (async () => {
    const browser = await firefox.launch();

    const page = await browser.newPage({
      viewport: {
        width: 1920,
        height: 1080
      },
      // supersample so sharp can downscale cleanly and avoid aliased edges
      deviceScaleFactor: scale
    });

    for (const themeName of themes) {
      await renderTheme(page, themeName);
    }

    await browser.close();
  })().catch((err) => {
    console.error(err);
    process.exit(1);
  });
}

module.exports = { run, PLATFORM_DIMS, faceScreenSize, findRepoRoot };
