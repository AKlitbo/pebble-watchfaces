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
import { faceDir, familyCoreDir } from '../faces.ts';

// the manifests are loaded by path at runtime which an import specifier cannot do
// require(esm) hands back the namespace so the manifest lands on .default
const requireManifest = createRequire(import.meta.url);

const ROOT = path.resolve(import.meta.dirname, '..', '..');

/** Where a root keeps its builder pieces, relative to the root itself. */
const FACE_BUILDER_REL = path.join('pkjs', 'clay', 'builder');
const LIB_BUILDER_REL = path.join('clay', 'builder');

/** One place builder pieces can live: the directory holding them. */
type Root = { base: string; builder: string };

/**
 * The three roots a builder piece can come from, face first.
 *
 * Each mirrors the others: the same path below a root's builder dir names the same piece
 * wherever it lives, so a lookup falls back by swapping one builder dir for another. That is the
 * same rule the C build follows, where a face-local header wins over the family's, and the
 * family's over lib's.
 *
 * lib is never null. A face in no family has no core, and reaches lib directly.
 */
type Roots = { face: Root; core: Root | null; lib: Root; faceRoot: string };

function rootsFor(face: string): Roots {
  const core = familyCoreDir(face);
  return {
    face: { base: path.join(faceDir(face), 'src'), builder: FACE_BUILDER_REL },
    core: core ? { base: core, builder: FACE_BUILDER_REL } : null,
    lib: { base: path.join(ROOT, 'lib', 'ts'), builder: LIB_BUILDER_REL },
    faceRoot: faceDir(face),
  };
}

/** The roots in precedence order: a face shadows its family, and both shadow lib. */
function rootList(roots: Roots): Root[] {
  return [roots.face, roots.core, roots.lib].filter(Boolean) as Root[];
}

/** A root's builder directory. */
function builderDir(root: Root): string {
  return path.join(root.base, root.builder);
}

/** The first root whose builder dir actually holds this relative path, or null. */
function resolveIn(roots: Roots, rel: string): string | null {
  for (const root of rootList(roots)) {
    const full = path.join(builderDir(root), rel);
    if (fs.existsSync(full)) {
      return full;
    }
  }

  return null;
}

/**
 * The same piece as it would sit under every other root, for the esbuild fallback.
 *
 * Returns candidates rather than one path, because with three roots a miss under the face can be
 * satisfied by either the family or lib.
 */
function swapRoot(roots: Roots, full: string): string[] {
  const all = rootList(roots);

  for (const from of all) {
    const rel = path.relative(builderDir(from), full);
    if (rel.startsWith('..')) {
      continue;
    }

    return all.filter((to) => to !== from).map((to) => path.join(builderDir(to), rel));
  }

  return [];
}

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

/**
 * Every *.manifest.ts recipe across all three builder roots.
 *
 * A face that ships its own copy of a manifest shadows the family's, and the family's shadows
 * lib's, so they are keyed by filename rather than concatenated.
 */
function findManifests(roots: Roots): string[] {
  const byName = new Map<string, string>();

  // least specific first so a family shadows lib and a face shadows both
  for (const root of rootList(roots).slice().reverse()) {
    const dir = builderDir(root);
    if (!fs.existsSync(dir)) {
      continue;
    }

    for (const name of fs.readdirSync(dir)) {
      if (name.endsWith('.manifest.ts')) {
        byName.set(name, path.join(dir, name));
      }
    }
  }

  return [...byName.values()].sort();
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

/**
 * Lets a piece under one root import a file that only exists under another.
 *
 * A shared piece can name a face-specific neighbour (the layout builder's geometry, say, or that
 * face's presets) and a face piece can name a shared one. esbuild is handed the miss and retries
 * it under the other roots, so no side has to spell out where the others live.
 *
 * There is no bail on a missing core: a face in no family still reaches lib through here.
 */
function overlayPlugin(roots: Roots): esbuild.Plugin {
  return {
    name: 'builder-root-overlay',
    setup(build) {
      build.onResolve({ filter: /^\.\.?\// }, (args) => {
        if (!args.importer) {
          return null;
        }

        const wanted = path.resolve(args.resolveDir, args.path);
        const candidates = [wanted, ...swapRoot(roots, wanted)];

        for (const candidate of candidates) {
          for (const suffix of ['', '.ts', '.js', '.json', '/index.ts', '/index.js']) {
            const full = candidate + suffix;
            if (fs.existsSync(full) && fs.statSync(full).isFile()) {
              return { path: full };
            }
          }
        }

        return null;
      });
    },
  };
}

/** Bundles a manifest's pieces into the ES2015 IIFE that becomes initialize. */
async function bundleInitialize(manifest: Manifest, manifestDir: string, roots: Roots): Promise<string> {
  const initPiece = findInitPiece(manifest);
  const result = await esbuild.build({
    stdin: {
      contents: buildEntrySource(manifest, initPiece),
      resolveDir: manifestDir,
      sourcefile: 'component-entry.js',
      loader: 'js',
    },
    plugins: [overlayPlugin(roots)],
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
async function buildComponentSource(manifestPath: string, roots: Roots): Promise<{ output: string; source: string }> {
  const manifest: Manifest = requireManifest(manifestPath).default;
  const manifestDir = path.dirname(manifestPath);
  // repo-root relative so the line reads the same whichever root the manifest came
  // from and whether or not the face sits inside a family folder
  const relManifest = path.relative(ROOT, manifestPath).replace(/\\/g, '/');

  // the template and the stylesheets follow the same face-then-core-then-lib lookup
  // as the pieces so a face can restyle just its own builder and take the rest from
  // the family
  const asset = (name: string): string => resolveIn(roots, name) || path.join(manifestDir, name);

  const bundle = indentBlock(await bundleInitialize(manifest, manifestDir, roots), '    ');
  const template = buildTemplate(asset(manifest.template));
  const style = buildStyle(manifest.styles.map(asset));
  const doc = manifest.doc.map((line) => (line ? ` * ${line}` : ' *')).join('\n');

  const source = `// generated from ${relManifest} by tools/clay-components/generate-components.ts
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

  // the component always lands in the face being built and never in the shared core
  return { output: path.join(roots.faceRoot, manifest.output), source };
}

/** Builds and writes every component for one face. */
async function generateAll(face: string): Promise<void> {
  const roots = rootsFor(face);

  for (const manifestPath of findManifests(roots)) {
    const built = await buildComponentSource(manifestPath, roots);
    fs.writeFileSync(built.output, built.source);
    console.log(`wrote ${path.relative(roots.faceRoot, built.output)} (${built.source.length} bytes)`);
  }
}

if (import.meta.main) {
  const face = process.argv[2];
  if (!face) {
    throw new Error('usage: generate-components.ts <face>');
  }

  // esbuild only takes a resolve plugin through its async API so the run ends on a
  // promise. rethrowing off-tick makes a failure a non-zero exit rather than a
  // silent unhandled rejection
  generateAll(face).catch((error) => {
    setTimeout(() => {
      throw error;
    });
  });
}

export {
  builderDir,
  rootsFor,
  resolveIn,
  swapRoot,
  overlayPlugin,
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
