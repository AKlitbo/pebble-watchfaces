/**
 * ESLint flat config.
 *
 * Lints all hand-authored TypeScript in the project, including the shared lib/,
 * src/pkjs/, Clay builder sources, and tools/.
 *
 * The project uses a consistent house style: 2-space indentation, single quotes,
 * semicolons, K&R braces, mandatory braces for control statements, and trailing
 * commas on multiline literals.
 *
 * All JavaScript files are generated, so only .ts files are linted.
 *
 * The ignore patterns live in here as well.
 */
import js from '@eslint/js';
import globals from 'globals';
import stylistic from '@stylistic/eslint-plugin';
import tseslint from 'typescript-eslint';
import { defineConfig, globalIgnores } from 'eslint/config';
import type { Linter } from 'eslint';

// house formatting and style rules
const houseStyleRules = {
  '@stylistic/semi': ['error', 'always'],
  '@stylistic/quotes': ['error', 'single', { avoidEscape: true, allowTemplateLiterals: 'always' }],
  '@stylistic/indent': ['error', 2, { SwitchCase: 1 }],

  // K&R braces. compact single-line lookup tables are also allowed
  '@stylistic/brace-style': ['error', '1tbs', { allowSingleLine: true }],

  '@stylistic/eol-last': ['error', 'always'],
  '@stylistic/no-trailing-spaces': 'error',

  // trailing commas on multiline literals
  // never in function parameters or arguments
  '@stylistic/comma-dangle': ['error', {
    arrays: 'always-multiline',
    objects: 'always-multiline',
    imports: 'always-multiline',
    exports: 'always-multiline',
    functions: 'never',
  }],

  '@stylistic/object-curly-spacing': ['error', 'always'],
  '@stylistic/keyword-spacing': 'error',
  '@stylistic/space-infix-ops': 'error',
  '@stylistic/comma-spacing': 'error',
  '@stylistic/space-before-blocks': 'error',
  '@stylistic/arrow-parens': ['error', 'always'],

  // always require braces around control statements
  'curly': ['error', 'all'],

  // allow the == null / != null idiom
  'eqeqeq': ['error', 'always', { null: 'ignore' }],

  'no-throw-literal': 'error',
  'no-implicit-coercion': 'error',
  'dot-notation': 'error',

  // empty catch blocks are permitted
  'no-empty': ['error', { allowEmptyCatch: true }],
} satisfies Linter.RulesRecord;

export default defineConfig([
  // ignore generated output and staged build artifacts
  globalIgnores([
    'node_modules/',
    '**/build/',
    'vendor/',
    '**/*.js',
    'targets/',
  ]),
  // lint all TypeScript sources except declaration files
  {
    files: ['**/*.ts'],
    ignores: ['**/*.d.ts'],
    // typescript-eslint disables the core rules it replaces
    extends: [js.configs.recommended, tseslint.configs.recommended],
    plugins: {
      '@stylistic': stylistic,
    },
    languageOptions: {
      ecmaVersion: 'latest',
      sourceType: 'module',
      globals: {
        ...globals.node,
        ...globals.browser,
        Pebble: 'readonly',
      },
    },
    rules: {
      ...houseStyleRules,
      // prefer the TypeScript-aware implementations
      'no-shadow': 'off',
      '@typescript-eslint/no-shadow': 'error',
      '@typescript-eslint/no-unused-vars': [
        'error',
        {
          caughtErrors: 'none',
          argsIgnorePattern: '^_',
        },
      ],
    },
  },
]);
