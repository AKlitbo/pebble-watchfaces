/**
 * ESLint flat config for the unit's own TypeScript.
 *
 * The house style the framework uses: 2-space indentation, single quotes, semicolons, K&R braces,
 * braces on every control statement, and trailing commas on multiline literals. The framework's own
 * code in paf/ is linted where it is written, so it is ignored here, along with generated and built
 * output.
 *
 * It sits in config/ with the unit's other configs, so the lint script names it with --config, and
 * ESLint then reads every pattern below from where it runs, the unit's root.
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
  // the framework copy, its swap folders, and generated or built output
  globalIgnores([
    'node_modules/',
    'paf/',
    'paf.paf-*/',
    'targets/',
    '.tmp/',
    '**/*.js',
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
