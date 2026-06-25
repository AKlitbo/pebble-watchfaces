#!/usr/bin/env node
/*
 * generate-frame.js - bake this face's radar-array HTML chrome into background bitmaps.
 *
 * Thin wrapper over tools/generate-frame-common.js; see that for the pipeline.
 * One frame/radar-array.html holds the layout and each theme is a palette-only
 * stylesheet at frame/css/theme_<name>.css.
 *
 * The build bakes every theme automatically; run gen:themes by hand to re-bake
 * during design:
 *   npm run gen:themes     (every theme)
 */
'use strict';

const { run } = require('../../../tools/generate-frame-common');

run({
  toolsDir: __dirname,
  defaultFrame: 'radar-array',
  defaultScale: 4,
  supportsTheme: true,
  bareBackgroundBase: null,
  clearTextSelectors: ['.time', '.meridiem', '.banner', '.data', '.lbl', '.pwr-label', '.tlabel'],
  hideSelectors: ['.batt']
});
