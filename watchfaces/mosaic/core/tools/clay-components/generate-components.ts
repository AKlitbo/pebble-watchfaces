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
import { faceDir, familyCoreDir } from '../../../../../tools/faces.ts';

// the manifests are loaded by path at runtime, which an import specifier cannot do:
// require(esm) hands back the namespace, so the manifest lands on .default
const requireManifest = createRequire(import.meta.url);

/**
 * The two roots a builder piece can come from, face first.
 *
 * The family core mirrors a face's own src/, so the same relative path names a file under
 * either root and a lookup falls back by swapping one prefix for the other. That is the same
 * rule the C build follows, where a face-local header wins over the family's.
 */
type Roots = { face: string; core: string | null; faceRoot: string };

function rootsFor(face: string): Roots {
  return {
    face: path.join(faceDir(face), 'src'),
    core: familyCoreDir(face),
    faceRoot: faceDir(face),
  };
}

/** The first of the two roots that actually holds this relative path, or null. */
function resolveIn(roots: Roots, rel: string): string | null {
  for (const root of [roots.face, roots.core]) {
    if (root) {
      const full = path.join(root, rel);
      if (fs.existsSync(full)) {
        return full;
      }
    }
  }

  return null;
}

/** Re-roots an absolute path onto the other root, for the esbuild fallback. */
function swapRoot(roots: Roots, full: string): string | null {
  for (const [from, to] of [[roots.face, roots.core], [roots.core, roots.face]] as const) {
    if (from && to && !path.relative(from, full).startsWith('..')) {
      return path.join(to, path.relative(from, full));
    }
  }

  return null;
}

const BUILDER_REL = path.join('pkjs', 'clay', 'builder');

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
 * Every *.manifest.ts recipe across both builder roots, face first.
 *
 * A face that ships its own copy of a manifest shadows the core one, so the pair is keyed by
 * filename rather than concatenated.
 */
function findManifests(roots: Roots): string[] {
  const byName = new Map<string, string>();

  for (const root of [roots.core, roots.face]) {
    const dir = root && path.join(root, BUILDER_REL);
    if (!dir || !fs.existsSync(dir)) {
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
 * Lets a piece under one root import a file that only exists under the other.
 *
 * A shared piece in core can name a face-specific neighbour (the layout builder's geometry, say,
 * or that face's presets) and a face piece can name a shared one. esbuild is handed the miss and
 * retries it with the roots swapped, so neither side has to spell out where the other lives.
 */
function overlayPlugin(roots: Roots): esbuild.Plugin {
  return {
    name: 'family-core-overlay',
    setup(build) {
      build.onResolve({ filter: /^\.\.?\// }, (args) => {
        if (!args.importer || !roots.core) {
          return null;
        }

        const wanted = path.resolve(args.resolveDir, args.path);
        const candidates = [wanted, swapRoot(roots, wanted)].filter(Boolean) as string[];

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
  const relManifest = path.relative(path.dirname(roots.faceRoot), manifestPath).replace(/\\/g, '/');

  // the template and the stylesheets follow the same face-then-core lookup as the pieces, so a
  // face can restyle just its own builder and take the rest from the family
  const asset = (name: string): string =>
    resolveIn(roots, path.join(BUILDER_REL, name)) || path.join(manifestDir, name);

  const bundle = indentBlock(await bundleInitialize(manifest, manifestDir, roots), '    ');
  const template = buildTemplate(asset(manifest.template));
  const style = buildStyle(manifest.styles.map(asset));
  const doc = manifest.doc.map((line) => (line ? ` * ${line}` : ' *')).join('\n');

  const source = `// generated from ${relManifest} by the family core's tools/clay-components/generate-components.ts
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

  // the component always lands in the face being built, never in the shared core
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

  // esbuild only takes a resolve plugin through its async API, so the run ends on a promise.
  // rethrowing off-tick makes a failure a non-zero exit rather than a silent unhandled rejection
  generateAll(face).catch((error) => {
    setTimeout(() => {
      throw error;
    });
  });
}

export {
  BUILDER_REL,
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
