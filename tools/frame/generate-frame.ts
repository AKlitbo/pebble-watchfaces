/**
 * generate-frame.ts - bake this face's LCARS HTML chrome into a background bitmap.
 *
 * Renders frame/<name>.html in Firefox at a supersampled deviceScaleFactor, strips the
 * live readouts (the app draws those at runtime), then resizes to the platform's native
 * screen size with a lanczos3 kernel and writes a PNG. Firefox (not Chromium) because
 * Chromium clips the LCARS elbow carve-outs that use z-index:-1 pseudo-elements.
 *
 * The build does not bake frames; the PNGs under resources/images/ are committed. Run this
 * by hand to re-bake one during design:
 *   npm run gen:frame -- lower-decks
 *   npm run gen:frame -- classic --scale 4 --out resources/images/background.png
 */
import path from 'node:path';
import fs from 'node:fs';
import { firefox } from 'playwright';
import sharp from 'sharp';
import { faceDir } from '../faces.ts';

/** Native screen size per Pebble platform (px). */
interface Dims {
  w: number;
  h: number;
}

export const PLATFORM_DIMS: Record<string, Dims> = {
  emery: { w: 200, h: 228 },
  gabbro: { w: 260, h: 260 },
  chalk: { w: 180, h: 180 },
  basalt: { w: 144, h: 168 },
  aplite: { w: 144, h: 168 },
  diorite: { w: 144, h: 168 },
  flint: { w: 144, h: 168 },
};

const ROOT = path.resolve(import.meta.dirname, '..', '..');

/**
 * The per-face render knobs, loaded from watchfaces/<face>/frame/frame.config.json.
 *
 * A face either bakes each theme from its own frame/<name>.html (supportsTheme: false, one
 * HTML per look) or swaps a palette over one HTML (supportsTheme: true, a theme_<name>.css
 * per look). clearTextSelectors have their text emptied and hideSelectors are hidden, so the
 * bake is pure chrome. The "bareBackgroundBase" frame bakes to background.png; every other
 * base lands at background-<base>.png.
 *
 * hideSelectors drop out of the flow, so use them for something nothing else is positioned
 * against. unpaintSelectors keep their box and lose only their pixels, which is what a piece of
 * chrome the watch draws for itself needs: the bake has to leave the space exactly where it was.
 *
 * maxColors caps how many colours the bake may keep. 16 is the number that matters: the SDK packs
 * a 16-colour bitmap at four bits per pixel and anything above it at eight, which doubles the heap
 * the watch needs to hold the frame. See capColors.
 */
export interface FaceConfig {
  defaultFrame: string;
  defaultScale: number;
  supportsTheme: boolean;
  bareBackgroundBase: string | null;
  clearTextSelectors: string[];
  hideSelectors: string[];
  unpaintSelectors?: string[];
  maxColors?: number;
}

/** The paths generate-frame reads and writes for one face, all under watchfaces/<face>/. */
interface FaceDirs {
  appinfo: string;
  frameDir: string;
  cssDir: string;
  imagesDir: string;
  config: string;
}

function faceDirs(face: string): FaceDirs {
  const base = faceDir(face);
  return {
    appinfo: path.join(base, 'config', 'pebble.appinfo.json'),
    frameDir: path.join(base, 'frame'),
    cssDir: path.join(base, 'frame', 'css'),
    imagesDir: path.join(base, 'resources', 'images'),
    config: path.join(base, 'frame', 'frame.config.json'),
  };
}

function loadFaceConfig(dirs: FaceDirs): FaceConfig {
  return JSON.parse(fs.readFileSync(dirs.config, 'utf8'));
}

/** Resolve the native screen size from the appinfo's first target platform (emery default). */
export function faceScreenSize(appinfoPath: string): Dims {
  try {
    const appinfo = JSON.parse(fs.readFileSync(appinfoPath, 'utf8'));
    const platform: unknown = appinfo.targetPlatforms && appinfo.targetPlatforms[0];
    if (typeof platform === 'string' && PLATFORM_DIMS[platform]) {
      return PLATFORM_DIMS[platform];
    }
  } catch {
    // no appinfo yet - fall through to the default
  }

  return PLATFORM_DIMS.emery;
}

// the watch never sees the anti-aliased bake. the SDK snaps every channel to one of four levels
// on its way to the Pebble-64 palette, so what decides the packed bit depth is how many of those
// 64 a frame lands on, not how many colours the PNG holds
function snapChannel(value: number): number {
  return Math.round(value / 85) * 85;
}

// one Pebble-64 colour as a single number, so the buckets can key a Map
// the >>> 0 matters: a red channel of 255 shifts into the sign bit and would otherwise come back
// negative, which no longer matches the same colour read out of a Uint32Array
function bucketKey(red: number, green: number, blue: number, alpha: number): number {
  return ((snapChannel(red) << 24) | (snapChannel(green) << 16) | (snapChannel(blue) << 8) |
    (alpha >= 128 ? 255 : 0)) >>> 0;
}

function bucketChannels(key: number): number[] {
  return [(key >>> 24) & 255, (key >>> 16) & 255, (key >>> 8) & 255, key & 255];
}

/**
 * @brief Fold a bake down to at most `limit` Pebble-64 colours.
 *
 * A 16-colour bitmap packs at four bits per pixel and a 17-colour one at eight, so one stray
 * colour doubles the heap the watch needs to hold the frame. A full-screen frame is big enough
 * that the difference is the difference between the image loading and not loading at all.
 *
 * The strays are anti-aliasing crumbs off the curved chrome, a handful of pixels each, so the
 * fix is to drop the least-used bucket into its nearest neighbour until the count comes down.
 * Only the pixels sitting in a dropped bucket are repainted. Every other pixel keeps the value
 * the resize gave it, which is what leaves the anti-aliasing alone.
 *
 * @param rgba The resized bake, four bytes per pixel.
 * @param limit The most colours to keep.
 * @return The same buffer with the stray pixels repainted.
 */
export function capColors(rgba: Uint8Array, limit: number): Uint8Array {
  const counts = new Map<number, number>();
  const keys = new Uint32Array(rgba.length / 4);

  for (let i = 0; i < keys.length; i++) {
    const key = bucketKey(rgba[i * 4], rgba[i * 4 + 1], rgba[i * 4 + 2], rgba[i * 4 + 3]);
    keys[i] = key;
    counts.set(key, (counts.get(key) || 0) + 1);
  }

  const remap = new Map<number, number>();
  while (counts.size > limit) {
    // ties break on the key so a re-bake of the same art always folds the same way
    let rarest = 0;
    let rarestCount = Infinity;
    for (const [key, count] of counts) {
      if (count < rarestCount || (count === rarestCount && key < rarest)) {
        rarest = key;
        rarestCount = count;
      }
    }

    const [red, green, blue, alpha] = bucketChannels(rarest);
    let nearest = 0;
    let nearestDistance = Infinity;
    for (const key of counts.keys()) {
      if (key === rarest) {
        continue;
      }
      const [r2, g2, b2, a2] = bucketChannels(key);
      const distance = (r2 - red) ** 2 + (g2 - green) ** 2 + (b2 - blue) ** 2 + (a2 - alpha) ** 2;
      if (distance < nearestDistance) {
        nearest = key;
        nearestDistance = distance;
      }
    }

    remap.set(rarest, nearest);
    counts.set(nearest, counts.get(nearest)! + rarestCount);
    counts.delete(rarest);
  }

  if (remap.size === 0) {
    return rgba;
  }

  for (let i = 0; i < keys.length; i++) {
    let target = remap.get(keys[i]);
    if (target === undefined) {
      continue;
    }

    // a bucket can be folded into one that later folds again, so follow the chain to the end
    while (remap.has(target)) {
      target = remap.get(target)!;
    }

    const [red, green, blue, alpha] = bucketChannels(target);
    rgba[i * 4] = red;
    rgba[i * 4 + 1] = green;
    rgba[i * 4 + 2] = blue;
    rgba[i * 4 + 3] = alpha;
  }

  return rgba;
}

/** Every theme_<name>.css under frame/css by name. */
function discoverThemes(cssDir: string): string[] {
  return fs
    .readdirSync(cssDir)
    .map((file) => file.match(/^theme_(.+)\.css$/))
    .filter((match): match is RegExpMatchArray => match !== null)
    .map((match) => match[1])
    .sort();
}

/** Parsed command-line options. */
interface Options {
  frame: string;
  scale: number;
  theme: string | null;
  outOverride: string | null;
}

export function parseArgs(argv: string[], face: FaceConfig): Options {
  let outOverride: string | null = null;
  let scale = face.defaultScale;
  let theme: string | null = null;
  const positional: string[] = [];

  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--out') {
      outOverride = argv[++i];
    } else if (argv[i] === '--scale') {
      scale = parseInt(argv[++i], 10) || face.defaultScale;
    } else if (argv[i] === '--theme') {
      theme = argv[++i];
    } else {
      positional.push(argv[i]);
    }
  }

  if (!face.supportsTheme) {
    theme = null;
  }

  const frame = (positional[0] || face.defaultFrame).replace(/\.html$/i, '');
  return { frame, scale, theme, outOverride };
}

/** Where a given theme's PNG lands. */
export function outFor(
  opts: Options,
  themeName: string | null,
  themeCount: number,
  face: FaceConfig,
  imagesDir: string
): string {
  if (opts.outOverride && themeCount === 1) {
    return path.resolve(ROOT, opts.outOverride);
  }

  const base = opts.frame;
  let name: string;
  if (themeName) {
    name = `background-${themeName}.png`;
  } else if (face.bareBackgroundBase && base === face.bareBackgroundBase) {
    name = 'background.png';
  } else {
    name = `background-${base}.png`;
  }

  return path.join(imagesDir, name);
}

async function main(): Promise<void> {
  const argv = process.argv.slice(2);
  const face = argv[0];
  if (!face || face.startsWith('-')) {
    console.error('usage: generate-frame.ts <face> [frame] [--scale N] [--theme name|all] [--out path]');
    process.exit(1);
  }

  const dirs = faceDirs(face);
  const faceCfg = loadFaceConfig(dirs);
  const opts = parseArgs(argv.slice(1), faceCfg);
  const { w: screenW, h: screenH } = faceScreenSize(dirs.appinfo);

  const html = path.join(dirs.frameDir, `${opts.frame}.html`);
  if (!fs.existsSync(html)) {
    console.error(`Frame HTML not found: ${html}`);
    process.exit(1);
  }
  const fileUrl = 'file://' + html.replace(/\\/g, '/');

  let themes: (string | null)[];
  if (opts.theme === 'all') {
    themes = discoverThemes(dirs.cssDir);
    if (themes.length === 0) {
      console.error(`No theme_*.css files found in ${dirs.cssDir}`);
      process.exit(1);
    }
  } else if (opts.theme) {
    themes = [opts.theme];
  } else {
    themes = [null];
  }

  const browser = await firefox.launch();
  const page = await browser.newPage({
    viewport: { width: 1920, height: 1080 },
    deviceScaleFactor: opts.scale,
  });

  const clearSel = faceCfg.clearTextSelectors.join(', ');
  const hideSel = faceCfg.hideSelectors.join(', ');
  const unpaintSel = (faceCfg.unpaintSelectors || []).join(', ');

  for (const themeName of themes) {
    await page.goto(fileUrl, { waitUntil: 'networkidle' });

    if (themeName) {
      const themeCss = path.join(dirs.cssDir, `theme_${themeName}.css`);
      if (!fs.existsSync(themeCss)) {
        throw new Error(`Theme stylesheet not found: ${themeCss}`);
      }
      await page.evaluate(() => {
        document.querySelectorAll('link[href*="theme_"]').forEach((link) => link.remove());
      });
      await page.addStyleTag({ path: themeCss });
    }

    await page.evaluate(
      ({ clear, hide, unpaint }: { clear: string; hide: string; unpaint: string }) => {
        if (clear) {
          document.querySelectorAll(clear).forEach((el) => {
            el.textContent = '';
          });
        }
        if (hide) {
          document.querySelectorAll(hide).forEach((el) => {
            (el as HTMLElement).style.display = 'none';
          });
        }
        // visibility rather than display so the box still takes up its space
        // it also takes the element's ::before and ::after along with it
        // which is where LCARS hides its end notches
        if (unpaint) {
          document.querySelectorAll(unpaint).forEach((el) => {
            (el as HTMLElement).style.visibility = 'hidden';
          });
        }
      },
      { clear: clearSel, hide: hideSel, unpaint: unpaintSel }
    );

    // networkidle does not guarantee custom webfonts are painted, so await the font api
    await page.evaluate(() => document.fonts && document.fonts.ready);
    await page.waitForTimeout(200);

    const screenshot = await page.locator('.viewport').screenshot();
    const out = outFor(opts, themeName, themes.length, faceCfg, dirs.imagesDir);
    fs.mkdirSync(path.dirname(out), { recursive: true });

    const resized = sharp(screenshot)
      // lanczos3 prevents moire when downscaling the sharp chrome geometry
      .resize(screenW, screenH, { kernel: 'lanczos3' });

    if (faceCfg.maxColors) {
      const raw = await resized.ensureAlpha().raw().toBuffer();
      await sharp(capColors(raw, faceCfg.maxColors), {
        raw: { width: screenW, height: screenH, channels: 4 },
      })
        .png()
        .toFile(out);
    } else {
      await resized.png().toFile(out);
    }

    console.log(
      `Rendered ${path.relative(ROOT, html)}${themeName ? ` [${themeName}]` : ''} -> ` +
        `${path.relative(ROOT, out)} (${screenW}x${screenH})`
    );
  }

  await browser.close();
}

if (import.meta.main) {
  main().catch((err) => {
    console.error(err);
    process.exit(1);
  });
}
