import { defineConfig } from 'vitest/config';

// deterministic util/provider specs plus opt-in live API blocks (RUN_LIVE=1 and
// the provider key), so the default suite is offline
// dotenv/config reads .env for those vars and the timeout covers the round-trips
export default defineConfig({
  test: {
    include: ['lib/**/*.spec.js'],
    exclude: ['**/node_modules/**'],
    setupFiles: ['dotenv/config'],
    testTimeout: 15000,
  },
});
