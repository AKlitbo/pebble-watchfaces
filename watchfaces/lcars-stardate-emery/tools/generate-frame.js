#!/usr/bin/env node
/*
 * generate-frame.js - bake this face's LCARS HTML chrome into a background bitmap.
 *
 * Thin wrapper over tools/generate-frame-common.js; see that for the pipeline.
 * This face has no themes, and its primary frame (classic) bakes to the bare
 * background.png while the others land at background-<base>.png.
 *
 * The build bakes every frame automatically; run this by hand to re-bake a single
 * frame during design:
 *   npm run gen:frame -- lower-decks
 */
'use strict';

const { run } = require('../../../tools/generate-frame-common');

run({
  toolsDir: __dirname,
  defaultFrame: 'classic-standard',
  defaultScale: 4,
  supportsTheme: false,
  bareBackgroundBase: 'classic',
  clearTextSelectors: ['.time', '.banner', '.data'],
  hideSelectors: ['.lcars-text-bar h3']
});
