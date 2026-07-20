/**
 * Stitches the clay builder pieces into the self contained component files
 * Clay ships into its config webview.
 *
 * Clay serializes a component's initialize and re-runs it in an isolated
 * webview where require does not exist, so the shipped file has to carry
 * everything inline. The pieces under src/pkjs/clay/builder/ stay small,
 * importable and testable (the waf pkjs glob skips that subtree, only the
 * generated files ship), and esbuild bundles them into one ES2015 IIFE that
 * becomes the component's initialize.
 *
 * Per component the generator builds a tiny entry that pulls in every piece and
 * re-exports the piece that declares init, bundles it, and drops the bundle into
 * initialize followed by `init.call(this)` so the manipulator's this still binds
 * the Clay component.
 *
 * Run with `npm run gen:clay`. Outputs are committed, and the spec fails when
 * they drift from the pieces.
 */

import fs from 'node:fs';
import path from 'node:path';
import esbuild from 'esbuild';
import { createRequire } from 'node:module';

// the manifests are loaded by path at runtime, which an import specifier cannot do:
// require(esm) hands back the namespace, so the manifest lands on .default
const requireManifest = createRequire(import.meta.url);

const ROOT = path.resolve(import.meta.dirname, '..', '..', '..');
const BUILDER_DIR = path.join(ROOT, 'src', 'pkjs', 'clay', 'builder');

/** One component's build recipe: which pieces to bundle and what to emit. */
export type Manifest = {
  name: string;
  output: string;
  template: string;
  styles: string[];
  pieces: string[];
  hookPrefix: string;
  doc: string[];
};

/** Reads a file with line endings normalized so Windows checkouts diff clean. */
function readText(filePath: string): string {
  return fs.readFileSync(filePath, 'utf8').replace(/\r\n/g, '\n');
}

/** Every *.manifest.ts recipe at the builder root. */
function findManifests(): string[] {
  return fs
    .readdirSync(BUILDER_DIR)
    .filter((name) => name.endsWith('.manifest.ts'))
    .map((name) => path.join(BUILDER_DIR, name))
    .sort();
}

/**
 * The manifest piece that declares init(), the entry the bundle re-exports.
 * Matched by filename so it works before esbuild runs and whatever extension the
 * piece carries (Node cannot require a .ts to look at its exports).
 */
function findInitPiece(manifest: Pick<Manifest, 'name' | 'pieces'>): string {
  const initPiece = manifest.pieces.find((piece) => path.basename(piece).replace(/\.[jt]s$/, '') === 'init');
  if (!initPiece) {
    throw new Error(`${manifest.name}: no piece named init`);
  }
  return initPiece;
}

/**
 * The entry source esbuild bundles: a require for every piece so none is tree
 * shaken away, then the init piece re-exported as the bundle's value.
 */
function buildEntrySource(manifest: Pick<Manifest, 'pieces'>, initPiece: string): string {
  const lines = manifest.pieces.map((piece) => `require(${JSON.stringify('./' + piece)});`);
  lines.push(`module.exports = require(${JSON.stringify('./' + initPiece)});`);
  return lines.join('\n');
}

/** Bundles a manifest's pieces into the ES2015 IIFE that becomes initialize. */
function bundleInitialize(manifest: Manifest, manifestDir: string): string {
  const initPiece = findInitPiece(manifest);
  const result = esbuild.buildSync({
    stdin: {
      contents: buildEntrySource(manifest, initPiece),
      resolveDir: manifestDir,
      sourcefile: 'component-entry.js',
      loader: 'js',
    },
    bundle: true,
    format: 'iife',
    globalName: '__clayComponent',
    target: 'es2015',
    write: false,
    legalComments: 'none',
    logLevel: 'silent',
  });
  return result.outputFiles[0].text.replace(/\r\n/g, '\n').replace(/\n+$/, '');
}

/** Trims blank edges and indents every non empty line for the initialize body. */
function indentBlock(text: string, indent: string): string {
  return text
    .replace(/^\n+/, '')
    .replace(/\n+$/, '')
    .split('\n')
    .map((line) => (line.length > 0 ? indent + line : line))
    .join('\n');
}

/** The template file flattened to the one line string Clay expects. */
function buildTemplate(templatePath: string): string {
  return readText(templatePath)
    .split('\n')
    .filter((line) => line.trim().length > 0)
    .join('');
}

/**
 * Squeezes a stylesheet down for the shipped string: comments out, whitespace
 * collapsed, punctuation tightened. The source files stay pretty, only the
 * inlined copy shrinks.
 */
function minifyCss(css: string): string {
  return css
    .replace(/\/\*[^]*?\*\//g, '')
    .replace(/\s+/g, ' ')
    .replace(/\s*([{}:;,>])\s*/g, '$1')
    .replace(/;\}/g, '}')
    .trim();
}

/** The css files minified and joined into the one style string Clay injects. */
function buildStyle(stylePaths: string[]): string {
  return stylePaths.map((stylePath) => minifyCss(readText(stylePath))).join('');
}

/** Builds the finished component source for one manifest file, plus where it lands. */
function buildComponentSource(manifestPath: string): { output: string; source: string } {
  const manifest: Manifest = requireManifest(manifestPath).default;
  const manifestDir = path.dirname(manifestPath);
  const relManifest = path.relative(ROOT, manifestPath).replace(/\\/g, '/');

  const bundle = indentBlock(bundleInitialize(manifest, manifestDir), '    ');
  const template = buildTemplate(path.join(manifestDir, manifest.template));
  const style = buildStyle(manifest.styles.map((name) => path.join(manifestDir, name)));
  const doc = manifest.doc.map((line) => (line ? ` * ${line}` : ' *')).join('\n');

  const source = `// generated by src/tools/clay-components/generate-components.ts from ${relManifest}
// do not edit by hand: run \`npm run gen:clay\` after changing the sources
/**
${doc}
 */
module.exports = {
  name: ${JSON.stringify(manifest.name)},

  template: ${JSON.stringify(template)},

  style: ${JSON.stringify(style)},

  manipulator: {
    set: function (value) {
      this.$element[0].${manifest.hookPrefix}Set(value || '');
    },
    get: function () {
      return this.$element[0].${manifest.hookPrefix}Get();
    }
  },

  initialize: function () {
${bundle}

    __clayComponent.init.call(this);
  }
};
`;

  // the finished file has to at least parse before it ships into the webview
  new Function('module', 'exports', 'require', source);

  return { output: path.join(ROOT, manifest.output), source };
}

/** Builds and writes every component, the whole `npm run gen:clay`. */
function generateAll(): void {
  for (const manifestPath of findManifests()) {
    const built = buildComponentSource(manifestPath);
    fs.writeFileSync(built.output, built.source);
    console.log(`wrote ${path.relative(ROOT, built.output)} (${built.source.length} bytes)`);
  }
}

if (import.meta.main) {
  generateAll();
}

export {
  ROOT,
  BUILDER_DIR,
  findManifests,
  findInitPiece,
  buildEntrySource,
  bundleInitialize,
  generateAll,
  indentBlock,
  buildTemplate,
  minifyCss,
  buildStyle,
  buildComponentSource,
};
