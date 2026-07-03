#!/usr/bin/env node
/**
 * Rasterize each watchface's SVG glyphs into its own Pebble PNG resources.
 *
 * Sources are vendored under vendor/:
 *   weather-icons - Erik Flowers  (key "wi")
 *   uxwing - heart, feet, thermometer, ...  (key "ux")
 *   svgrepo - bluetooth on/slash  (key "sr")
 *
 * Each face declares what it needs in watchfaces/<face>/resources/icons.json,
 * mapping an icon name to its vendored svg and final pixel size:
 *
 *   { "wi-clear": { "svg": "wi/wi-day-sunny", "size": [24, 24] } }
 *
 * The icon name is the file basename; its Pebble resource id is derived from it
 * (wi-clear -> ICON_WI_CLEAR). Faces own their sizes, so the same condition can
 * ship at 24px on one face and 12px on another with no shared -sm/-md variants.
 *
 * Glyphs are forced to white because upstream SVGs do not define a consistent
 * fill or stroke color. Faces recolor the loaded bitmap's palette at runtime, so
 * one white master serves every tint. Each icon is rasterized at its final size:
 * Pebble clips bitmaps rather than scaling them at draw time.
 *
 * For every manifest this:
 *   1. renders each glyph into watchfaces/<face>/resources/icons/<name>.png
 *   2. rewrites that face's package.json media block so name -> file stays in sync
 *
 * Re-run after editing a manifest. Pass a face name to touch only that face and
 * leave the others alone; omit it to do every face:
 *   npm run gen:icons                    # all faces
 *   npm run gen:icons -- modular-emery   # just this one
 */
'use strict';

const fs = require('fs');
const path = require('path');
const sharp = require('sharp');

const ROOT = path.resolve(__dirname, '..');
const VENDOR = path.resolve(ROOT, 'vendor');
const FACES_DIR = path.resolve(ROOT, 'watchfaces');

// manifest svg key -> the vendor subdir holding those svgs
const VENDOR_DIRS = {
  wi: path.join('weather-icons', 'svg'),
  ux: 'uxwing',
  sr: 'svgrepo'
};

const WHITE = '#ffffff';

const TRANSPARENT = {
  r: 0,
  g: 0,
  b: 0,
  alpha: 0
};

function whiten(svgText) {
  // recolor any hard-coded black so stroke-style glyphs (which set their own color on the path) come through white
  let out = svgText.replace(
    /(fill|stroke)="(#000000|#000|black)"/gi,
    `$1="${WHITE}"`
  );

  // glyphs that never set a fill on the root default to black so force white there
  // glyphs that already set a root fill (like fill="none" on stroke icons) are left alone
  // so their outlines stay open instead of getting flooded white. poking white into the
  // root svg tag with a regex saves loading a whole dom parser just to change colours
  if (!/<svg\b[^>]*\bfill=/i.test(out)) {
    out = out.replace(
      /<svg\b/,
      `<svg fill="${WHITE}" stroke="${WHITE}"`
    );
  }

  return out;
}

// rasterize the svg big then find the opaque bounding box and crop the PNG to just the glyph
// lets a small glyph in a padded viewBox fill the final icon at a size that matches its
// siblings instead of shrinking with the empty viewBox padding
async function trimToGlyph(svgText) {
  const HI = 240;
  const png = await sharp(Buffer.from(svgText))
    .resize(HI, HI, { fit: 'contain', background: TRANSPARENT })
    .png()
    .toBuffer();

  const { data, info } = await sharp(png).raw().toBuffer({ resolveWithObject: true });
  const channels = info.channels;
  let minX = HI, minY = HI, maxX = -1, maxY = -1;
  for (let y = 0; y < HI; y++) {
    for (let x = 0; x < HI; x++) {
      if (data[(y * HI + x) * channels + 3] > 16) {
        if (x < minX) {
          minX = x;
        }
        if (x > maxX) {
          maxX = x;
        }
        if (y < minY) {
          minY = y;
        }
        if (y > maxY) {
          maxY = y;
        }
      }
    }
  }

  if (maxX < 0) {
    return png; // fully transparent so nothing to trim
  }

  return sharp(png)
    .extract({ left: minX, top: minY, width: maxX - minX + 1, height: maxY - minY + 1 })
    .png()
    .toBuffer();
}

async function render(svgText, [width, height], outputFile, opts = {}) {
  // sharp requires a buffer here instead of a file path because we modified the raw svg string in memory above
  const input = opts.trim ? await trimToGlyph(svgText) : Buffer.from(svgText);
  await sharp(input)
    .resize(width, height, {
      // fit contain ensures svgs with weird aspect ratios dont stretch padding the rest of the bounding box with the transparent background
      fit: 'contain',
      background: TRANSPARENT
    })
    .png()
    .toFile(outputFile);
}

// "wi/wi-day-sunny" -> vendor/weather-icons/svg/wi-day-sunny.svg
function svgPath(ref) {
  const slash = ref.indexOf('/');
  const key = slash === -1 ? '' : ref.slice(0, slash);
  const dir = VENDOR_DIRS[key];
  if (!dir) {
    throw new Error(`Unknown vendor key in svg ref "${ref}" (expected one of: ${Object.keys(VENDOR_DIRS).join(', ')})`);
  }

  return path.join(VENDOR, dir, `${ref.slice(slash + 1)}.svg`);
}

// "wi-clear" -> "ICON_WI_CLEAR"
function resourceName(basename) {
  return 'ICON_' + basename.toUpperCase().replace(/-/g, '_');
}

// a media entry is a face icon when it is a bitmap under an icons/ dir. media file paths
// are relative to the face's resources/ dir so this matches both the new local
// "icons/foo.png" and the old shared "../../../lib/resources/icons/foo.png"
function isIconEntry(entry) {
  return entry.type === 'bitmap' && typeof entry.file === 'string' && /(^|\/)icons\//.test(entry.file);
}

/**
 * Merge a manifest's icons into an existing media array. Non-icon entries (fonts,
 * background images) keep their place and order; the icon block is replaced in
 * full and lands where the first old icon sat (or just before the fonts on a face
 * that had none). Pure so it can be tested without touching disk.
 */
function buildMedia(media, manifest) {
  const icons = Object.keys(manifest).map((name) => ({
    type: 'bitmap',
    name: resourceName(name),
    file: `icons/${name}.png`
  }));

  let insertAt = media.findIndex(isIconEntry);
  if (insertAt === -1) {
    const firstFont = media.findIndex((entry) => entry.type === 'font');
    insertAt = firstFont === -1 ? media.length : firstFont;
  }

  const keptBefore = media.slice(0, insertAt).filter((entry) => !isIconEntry(entry));
  const keptAfter = media.slice(insertAt).filter((entry) => !isIconEntry(entry));

  return [...keptBefore, ...icons, ...keptAfter];
}

// serialize one media entry. bitmaps ride on a single line for a scannable list
// fonts keep their multi-line block so their extra fields (like characterRegex) stay put
function formatEntry(entry, indent) {
  if (entry.type === 'bitmap') {
    return `${indent}{ "type": "bitmap", "name": "${entry.name}", "file": "${entry.file}" }`;
  }

  return JSON.stringify(entry, null, 2)
    .split('\n')
    .map((line) => indent + line)
    .join('\n');
}

// splice the rebuilt media array back into the raw package.json text and leave every
// other byte of the file untouched. bracket-matching steps over array-valued fields
// inside an entry (like a font's targetPlatforms) and quoted brackets in strings
function replaceMediaArray(raw, newMedia) {
  const keyAt = raw.indexOf('"media"');
  if (keyAt === -1) {
    throw new Error('no "media" array in package.json');
  }

  const open = raw.indexOf('[', keyAt);
  let depth = 0, close = -1, inString = false, escaped = false;
  for (let i = open; i < raw.length; i++) {
    const char = raw[i];
    if (inString) {
      if (escaped) {
        escaped = false;
      } else if (char === '\\') {
        escaped = true;
      } else if (char === '"') {
        inString = false;
      }
      continue;
    }
    if (char === '"') {
      inString = true;
    } else if (char === '[') {
      depth++;
    } else if (char === ']') {
      depth--;
      if (depth === 0) {
        close = i;
        break;
      }
    }
  }
  if (close === -1) {
    throw new Error('unterminated "media" array in package.json');
  }

  const lineStart = raw.lastIndexOf('\n', keyAt) + 1;
  const mediaIndent = raw.slice(lineStart, keyAt).match(/^\s*/)[0];
  const entryIndent = mediaIndent + '  ';

  const body = newMedia.map((entry) => formatEntry(entry, entryIndent)).join(',\n');
  return raw.slice(0, open + 1) + '\n' + body + '\n' + mediaIndent + raw.slice(close);
}

// rewrite a face's package.json so its media icon block matches the manifest
function syncMedia(pkgPath, manifest) {
  const raw = fs.readFileSync(pkgPath, 'utf8');
  const pkg = JSON.parse(raw);
  const media = pkg.pebble && pkg.pebble.resources && pkg.pebble.resources.media;
  if (!Array.isArray(media)) {
    throw new Error(`no pebble.resources.media array in ${pkgPath}`);
  }

  fs.writeFileSync(pkgPath, replaceMediaArray(raw, buildMedia(media, manifest)));
}

// render every icon a manifest asks for into the face's resources/icons dir
async function renderFace(faceDir, manifest) {
  const outDir = path.join(faceDir, 'resources', 'icons');
  await fs.promises.mkdir(outDir, { recursive: true });

  for (const [name, spec] of Object.entries(manifest)) {
    const src = svgPath(spec.svg);
    try {
      await fs.promises.access(src);
    } catch {
      throw new Error(`Missing source: ${src} (for ${name})`);
    }

    const svg = await fs.promises.readFile(src, 'utf8');
    await render(whiten(svg), spec.size, path.join(outDir, `${name}.png`), { trim: !!spec.trim });
  }

  return Object.keys(manifest).length;
}

async function main() {
  const only = process.argv[2];

  let faces = (await fs.promises.readdir(FACES_DIR, { withFileTypes: true }))
    .filter((entry) => entry.isDirectory())
    .map((entry) => entry.name);

  if (only) {
    if (!faces.includes(only)) {
      throw new Error(`No such face "${only}" under watchfaces/ (have: ${faces.join(', ')})`);
    }
    faces = [only];
  }

  let faceCount = 0;
  let iconCount = 0;

  for (const face of faces) {
    const faceDir = path.join(FACES_DIR, face);
    const manifestPath = path.join(faceDir, 'resources', 'icons.json');
    if (!fs.existsSync(manifestPath)) {
      if (only) {
        throw new Error(`Face "${only}" has no resources/icons.json to generate from`);
      }
      continue; // a face with no icons.json simply opts out
    }

    const manifest = JSON.parse(await fs.promises.readFile(manifestPath, 'utf8'));
    const rendered = await renderFace(faceDir, manifest);
    syncMedia(path.join(faceDir, 'package.json'), manifest);

    console.log(`  ${face}: ${rendered} icons`);
    faceCount++;
    iconCount += rendered;
  }

  console.log(`Rendered ${iconCount} icons across ${faceCount} faces.`);
}

if (require.main === module) {
  main().catch((err) => {
    console.error(err);
    process.exit(1);
  });
}

module.exports = { whiten, resourceName, buildMedia, replaceMediaArray };
