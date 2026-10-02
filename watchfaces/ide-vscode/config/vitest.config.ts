/**
 * Vitest config for the unit's own specs.
 *
 * Runs every *.spec.ts the unit holds in node. A spec that needs a DOM opts in to jsdom with a pragma at
 * the top of its file. The framework's specs stay with the framework, so paf/ is never searched.
 *
 * It sits in config/ with the unit's other configs, and sets the unit's root as the one its paths are
 * read from. Run through paf test, or npm run test:watch and test:coverage.
 */
import path from 'node:path';
import { defineConfig } from 'vitest/config';

export default defineConfig({
  root: path.resolve(import.meta.dirname, '..'),
  test: {
    include: ['src/**/*.spec.ts'],
    exclude: ['**/node_modules/**', 'paf/**', 'targets/**'],
    coverage: {
      provider: 'v8',
      reporter: ['text', 'html'],
      reportsDirectory: 'coverage',
      include: ['src/pkjs/**'],
      exclude: ['**/*.spec.ts', '**/*.d.ts', '**/*.g.js'],
    },
  },
});
