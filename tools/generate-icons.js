#!/usr/bin/env node
/*
 * generate-icons.js - rasterize watchface SVG glyphs into Pebble PNG resources.
 *
 * Sources are vendored under vendor/:
 *   weather-icons - Erik Flowers
 *   uxwing - heart, feet, thermometer
 *
 * Glyphs are forced to white because upstream SVGs do not define a consistent
 * fill or stroke color.
 *
 * Each icon is rasterized at its final display size. Pebble clips bitmaps
 * rather than scaling them at draw time.
 *
 * Output:
 *   shared/resources/icons/<name>.png
 *
 * Re-run after editing:
 *   npm run gen:icons
 */
'use strict';

const fs = require('fs');
const path = require('path');
const sharp = require('sharp');

const OUT = path.resolve(__dirname, '..', 'shared', 'resources', 'icons');
const VENDOR = path.resolve(__dirname, '..', 'vendor');

const WI = (file) => path.join(VENDOR, 'weather-icons', 'svg', file);
const UX = (file) => path.join(VENDOR, 'uxwing', file);

const WHITE = '#ffffff';

const TRANSPARENT = {
  r: 0,
  g: 0,
  b: 0,
  alpha: 0
};

// name -> [vendored svg path, [width, height]]
const FILE_ICONS = {
  // weather (Erik Flowers), rendered into the 24x24 WX_ICON slot
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
  // glyphs (UXWing)
  'heart':       [UX('heart-pulse-icon.svg'), [14, 14]],
  'feet':        [UX('steps-icon.svg'),       [14, 14]],
  'thermometer': [UX('thermometer-icon.svg'), [13, 17]]
};

function whiten(svgText) {
  // injecting fill and stroke directly into the root svg tag using regex avoids needing a heavy dom parser in node just to change colors
  return svgText.replace(
    /<svg\b/,
    `<svg fill="${WHITE}" stroke="${WHITE}"`
  );
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