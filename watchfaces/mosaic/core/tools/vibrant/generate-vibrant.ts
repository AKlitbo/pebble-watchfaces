#!/usr/bin/env node
/**
 * Generate the single source of truth for each module's VIBRANT-theme colour.
 *
 * Every module paints its own accent/value/icon colour over the mono base under the
 * Vibrant theme. That one colour-per-module fact is authored once in
 * src/data/module-vibrant.json and expanded here into the two forms the
 * runtimes actually read:
 *
 *   C  -> src/c/engine/vibrant_table.g.h   the MODULE_VIBRANT[] table the grid engine reads
 *   JS -> src/pkjs/clay/vibrant.g.js       the ordinal -> palette name map the config UI reads
 *
 * Source entry shapes (keyed by ModuleType enum name):
 *   {}                                              no colour, stays mono (MOD_EMPTY, retired ids)
 *   { "color": "Vivid Cerulean" }                   accent = value = icon, subtitle left mono
 *   { "accent": "...", "value": "...", "icon": "..." }  per-channel, any omitted channel stays mono
 *   { "alias": "MOD_OTHER" }                         same family: inherit MOD_OTHER's resolved colour
 *
 * Palette names are validated against the official Pebble-64 list in
 * src/pkjs/clay/builder/ts/theme/palette.ts, so a typo fails the build
 * instead of rendering a box.
 *
 * Re-run after editing the manifest:
 *   npm run gen:vibrant
 */
import fs from 'node:fs';
import path from 'node:path';
import { PEBBLE_COLORS_CSV, buildPalette } from '../../pkjs/clay/builder/ts/theme/palette.ts';
import { faceDir } from '../../../../../tools/faces.ts';

// this generator lives in the family core, so its own root is the core and the face it writes
// the JS half into arrives as an argument
const CORE = path.resolve(import.meta.dirname, '..', '..');
const CATALOG_H = path.join(CORE, 'c', 'engine', 'catalog.h');
const SOURCE_JSON = path.join(CORE, 'data', 'module-vibrant.json');
const OUT_C = path.join(CORE, 'c', 'engine', 'vibrant_table.g.h');

/** Where this face's generated JS table lands. The C half is shared, so it only has one home. */
function outJs(face: string): string {
  return path.join(faceDir(face), 'src', 'pkjs', 'clay', 'vibrant.g.js');
}

/** A module's four channel colours. null means the channel keeps the mono base. */
type Channels = {
  accent: string | null;
  value: string | null;
  icon: string | null;
  subtitle: string | null;
};

/** One module's entry in module-vibrant.json. */
type SourceEntry = Partial<Record<keyof Channels, string>> & { color?: string; alias?: string };

/** module-vibrant.json, keyed by ModuleType enum name. */
type VibrantSource = Record<string, SourceEntry>;

const CHANNELS: Array<keyof Channels> = ['accent', 'value', 'icon', 'subtitle'];

/**
 * Pull the ModuleType members in declared order, so an index is a module's stored id.
 * MOD_TYPE_COUNT is the sentinel, not a module, so it is dropped.
 */
function parseModuleOrder(catalogText: string): string[] {
  const block = catalogText.match(/typedef enum\s*\{([\s\S]*?)\}\s*ModuleType\s*;/);
  if (!block) {
    throw new Error('could not find the ModuleType enum in catalog.h');
  }

  const names = [];
  for (const rawLine of block[1].split('\n')) {
    const line = rawLine.replace(/\/\/.*$/, '');
    const match = line.match(/\bMOD_[A-Z0-9_]+\b/);
    if (match && match[0] !== 'MOD_TYPE_COUNT') {
      names.push(match[0]);
    }
  }
  return names;
}

/**
 * The official Pebble-64 names, read from the theme builder's palette piece so
 * there is one palette source instead of a second hand-kept copy here.
 */
function paletteNameSet(): Set<string> {
  const names = new Set<string>();
  for (const entry of buildPalette(PEBBLE_COLORS_CSV)) {
    names.add(entry.name);
  }
  return names;
}

/** A palette name maps to its GColor constant by dropping the spaces. */
function gcolorConst(name: string): string {
  return 'GColor' + name.replace(/ /g, '');
}

/**
 * Resolve one module to its four channel names (null = stays mono), following an alias to
 * whichever module owns the family colour.
 */
function resolveColors(source: VibrantSource, name: string, seen?: Set<string>): Channels {
  seen = seen || new Set<string>();
  if (seen.has(name)) {
    throw new Error('alias cycle through ' + name);
  }
  seen.add(name);

  const entry = source[name];
  if (!entry) {
    throw new Error('no source entry for ' + name);
  }
  if (entry.alias) {
    return resolveColors(source, entry.alias, seen);
  }

  const colors: Channels = { accent: null, value: null, icon: null, subtitle: null };
  if (entry.color) {
    colors.accent = colors.value = colors.icon = entry.color;
  } else {
    for (const channel of CHANNELS) {
      const value = entry[channel];
      if (value) {
        colors[channel] = value;
      }
    }
  }
  return colors;
}

/** Fail loudly on a typo or a stray key before either output is written. */
function validate(order: string[], source: VibrantSource, paletteNames: Set<string>): void {
  const known = new Set(order);
  for (const name of Object.keys(source)) {
    if (!known.has(name)) {
      throw new Error('unknown module ' + name + ' in module-vibrant.json');
    }
  }
  for (const name of order) {
    if (!source[name]) {
      throw new Error('module ' + name + ' is missing from module-vibrant.json');
    }
    const colors = resolveColors(source, name);
    for (const channel of CHANNELS) {
      if (colors[channel] && !paletteNames.has(colors[channel])) {
        throw new Error(name + '.' + channel + ' uses an unknown palette colour "' + colors[channel] + '"');
      }
    }
  }
}

/** The C table: one designated-initialiser row per coloured module, mono ones left out. */
function buildHeader(order: string[], source: VibrantSource): string {
  const rows: string[] = [];
  for (const name of order) {
    const colors = resolveColors(source, name);
    const set = CHANNELS.filter((channel) => colors[channel]);
    if (set.length === 0) {
      continue;
    }
    const fields = set.map((channel) => '.' + channel + ' = ' + gcolorConst(colors[channel] as string)).join(', ');
    rows.push('    [' + name + '] = { ' + fields + ' },');
  }

  return [
    '// generated by src/tools/vibrant/generate-vibrant.ts from src/data/module-vibrant.json',
    '// do not edit by hand: run `npm run gen:vibrant` after changing the source',
    '#pragma once',
    '',
    '// each module\'s VIBRANT colours, indexed by ModuleType. an unlisted or GColorClear',
    '// channel keeps the mono base. include after catalog.h so the ids and ModuleColors exist',
    'static const ModuleColors MODULE_VIBRANT[MOD_TYPE_COUNT] = {',
    ...rows,
    '};',
    '',
  ].join('\n');
}

/** The JS map: ordinal -> the module's accent palette name, for the config Use Vibrant button. */
function buildJsMap(order: string[], source: VibrantSource): string {
  const rows: string[] = [];
  order.forEach((name, ordinal) => {
    const colors = resolveColors(source, name);
    const vibrant = colors.accent || colors.value || colors.icon;
    if (vibrant) {
      rows.push('  ' + ordinal + ': "' + vibrant + '",');
    }
  });

  return [
    '// generated by src/tools/vibrant/generate-vibrant.ts from src/data/module-vibrant.json',
    '// do not edit by hand: run `npm run gen:vibrant` after changing the source',
    '// the VIBRANT colour per ModuleType id, merged onto MODULE_OPTIONS in config.ts',
    'module.exports = {',
    ...rows,
    '};',
    '',
  ].join('\n');
}

/** Reads the source manifest, checks it, and writes both the C table and the JS map. */
function main(face: string): void {
  const order = parseModuleOrder(fs.readFileSync(CATALOG_H, 'utf8'));
  const source: VibrantSource = JSON.parse(fs.readFileSync(SOURCE_JSON, 'utf8'));
  const paletteNames = paletteNameSet();

  validate(order, source, paletteNames);

  const outJsPath = outJs(face);
  fs.writeFileSync(OUT_C, buildHeader(order, source));
  fs.writeFileSync(outJsPath, buildJsMap(order, source));
  console.log('wrote the core C table and ' + path.relative(faceDir(face), outJsPath));
}

if (import.meta.main) {
  const face = process.argv[2];
  if (!face) {
    throw new Error('usage: generate-vibrant.ts <face>');
  }
  main(face);
}

export {
  CATALOG_H,
  SOURCE_JSON,
  OUT_C,
  outJs,
  parseModuleOrder,
  paletteNameSet,
  gcolorConst,
  resolveColors,
  validate,
  buildHeader,
  buildJsMap,
};
