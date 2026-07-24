/**
 * Vitest config.
 *
 * Runs the host spec suite (every *.spec.ts) under node by default. Specs that need a
 * browser environment opt in per file with a jsdom pragma. Live API blocks stay off
 * unless RUN_LIVE_WEATHER=1 or RUN_LIVE_STOCK=1 and the provider key are set, so the
 * default run is offline and deterministic.
 *
 * Run via npm test, or npm run test:watch and test:coverage.
 */
import path from 'node:path';
import { defineConfig } from 'vitest/config';

// where the watch's ical.js actually lives. lib/ts/calendar/icaljs.d.ts describes it but the
// source tree has no such file. the build copies ical.js's prebuilt ES5 CommonJS bundle into
// emit/ beside the compiled calendar code. aim the specs at that same bundle so they run what
// the watch runs
const ICALJS = path.resolve(
  import.meta.dirname, '..', 'node_modules', 'ical.js', 'dist', 'ical.es5.min.cjs'
);

export default defineConfig({
  resolve: {
    alias: [{ find: /^\.\/icaljs$/, replacement: ICALJS }],
  },
  test: {
    include: ['**/*.spec.ts'],
    // targets/ holds build-time copies of the shared sources including the specs
    // the real specs live at the root so skip the staged duplicates
    exclude: ['**/node_modules/**', '**/build/**', 'targets/**'],
    // dotenv/config reads .env for the live-API vars
    setupFiles: ['dotenv/config'],
    // generous enough to cover a live API round-trip when those blocks are on
    testTimeout: 15000,
    // text for the console. html for the pages site. json-summary for the landing-page percent
    coverage: {
      provider: 'v8',
      reporter: ['text', 'html', 'json-summary'],
      reportsDirectory: 'coverage',
      // every tree that has specs so the number covers the whole suite
      // not just the runtime
      include: ['lib/ts/**', 'lib/tools/**', 'watchfaces/*/src/pkjs/**', 'tools/**'],
      exclude: [
        '**/*.spec.ts',
        '**/*.d.ts',
        // generated from the clay-components pieces which carry their own coverage
        // counting the assembled copies would double-book them
        '**/*.g.js',
        'targets/**',
        '**/build/**',
      ],
    },
  },
});
