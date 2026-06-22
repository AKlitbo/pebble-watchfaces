#!/usr/bin/env node
/**
 * Rasterize watchface SVG glyphs into Pebble PNG resources.
 *
 * Sources are vendored under vendor/:
 *   weather-icons - Erik Flowers
 *   uxwing - heart, feet, thermometer
 *   svgrepo - bluetooth on/slash
 *
 * Glyphs are forced to white because upstream SVGs do not define a consistent
 * fill or stroke color. Faces that need another color recolor the loaded bitmap's
 * palette at runtime, so one white master serves every tint instead of
 * shipping a per-color asset.
 *
 * Each icon is rasterized at its final display size. Pebble clips bitmaps
 * rather than scaling them at draw time.
 *
 * Output:
 *   lib/resources/icons/<name>.png
 *
 * Re-run after editing:
 *   npm run gen:icons
 */
'use strict';

const fs = require('fs');
const path = require('path');
const sharp = require('sharp');

const OUT = path.resolve(__dirname, '..', 'lib', 'resources', 'icons');
const VENDOR = path.resolve(__dirname, '..', 'vendor');

const WI = (file) => path.join(VENDOR, 'weather-icons', 'svg', file);
const UX = (file) => path.join(VENDOR, 'uxwing', file);
const SR = (file) => path.join(VENDOR, 'svgrepo', file);

const WHITE = '#ffffff';

const TRANSPARENT = {
  r: 0,
  g: 0,
  b: 0,
  alpha: 0
};

// name -> [vendored svg path, [width, height]]
const FILE_ICONS = {
  // Erik Flowers, rendered into the 24x24 WX_ICON slot
  'wi-clear':        [WI('wi-day-sunny.svg'),    [24, 24]],
  'wi-night-clear':  [WI('wi-night-clear.svg'),  [24, 24]],
  'wi-cloudy':       [WI('wi-cloudy.svg'),       [24, 24]],
  'wi-fog':          [WI('wi-fog.svg'),          [24, 24]],
  'wi-drizzle':      [WI('wi-sprinkle.svg'),     [24, 24]],
  'wi-rain':         [WI('wi-rain.svg'),         [24, 24]],
  'wi-sleet':        [WI('wi-sleet.svg'),        [24, 24]],
  'wi-snow':         [WI('wi-snow.svg'),         [24, 24]],
  'wi-showers':      [WI('wi-showers.svg'),      [24, 24]],
  'wi-snow-wind':    [WI('wi-snow-wind.svg'),    [24, 24]],
  'wi-thunderstorm': [WI('wi-thunderstorm.svg'), [24, 24]],
  'wi-na':           [WI('wi-na.svg'),           [24, 24]],
  // UXWing
  'heart':       [UX('heart-pulse-icon.svg'), [14, 14]],
  'feet':        [UX('steps-icon.svg'),       [14, 14]],
  'thermometer': [UX('thermometer-icon.svg'), [13, 17]],
  // SVG Repo
  'bluetooth-sm':       [SR('bluetooth-on.svg'),    [10, 10]],
  'bluetooth-slash-sm': [SR('bluetooth-slash.svg'), [10, 10]],
  'bluetooth-md':       [SR('bluetooth-on.svg'),    [14, 14]],
  'bluetooth-slash-md': [SR('bluetooth-slash.svg'), [14, 14]]
};

function whiten(svgText) {
  // recolor any hard-coded black so stroke-style glyphs (which set their own color on the path) come through white
  let out = svgText.replace(
    /(fill|stroke)="(#000000|#000|black)"/gi,
    `$1="${WHITE}"`
  );

  // glyphs that never declare a root fill default to black, so force white there. glyphs that already declare a root fill (e.g. fill="none" on stroke icons) are left untouched so their outlines stay open instead of being flooded white. injecting into the root svg tag with a regex avoids pulling in a heavy dom parser just to change colors
  if (!/<svg\b[^>]*\bfill=/i.test(out)) {
    out = out.replace(
      /<svg\b/,
      `<svg fill="${WHITE}" stroke="${WHITE}"`
    );
  }

  return out;
}

async function render(svgText, [width, height], outputFile) {
  // sharp requires a buffer here instead of a file path because we modified the raw svg string in memory above
  await sharp(Buffer.from(svgText))
    .resize(width, height, {
      // fit contain ensures svgs with weird aspect ratios dont stretch padding the rest of the bounding box with the transparent background
      fit: 'contain',
      background: TRANSPARENT
    })
    .png()
    .toFile(outputFile);
}

(async () => {
  await fs.promises.mkdir(OUT, { recursive: true });

  let count = 0;

  for (const [name, [src, size]] of Object.entries(FILE_ICONS)) {
    try {
      await fs.promises.access(src);
    } catch {
      throw new Error(`Missing Source: ${src}`);
    }

    const svg = await fs.promises.readFile(src, 'utf8');
    await render(
      whiten(svg),
      size,
      path.join(OUT, `${name}.png`)
    );

    count++;
  }

  console.log(
    `Rendered ${count} Icons -> \n` +
    `    ${path.relative(path.resolve(__dirname, '..'), OUT)}`
  );
})().catch((err) => {
  console.error(err);
  process.exit(1);
});