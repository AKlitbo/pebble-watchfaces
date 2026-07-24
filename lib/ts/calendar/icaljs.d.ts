/**
 * The seam between our code and ical.js.
 *
 * ical.js is MPL 2.0 and ships as its own file rather than being pulled in by name (see
 * NOTICES.md). The name matters: the package is ESM with an `exports` map, and the Pebble SDK
 * bundles with webpack 1, which predates both. Asking for it by name would resolve `main` to the
 * ESM build and break, so the build copies the package's prebuilt ES5 CommonJS file to
 * emit/lib/ts/calendar/icaljs.js and everything requires it from there.
 *
 * There is no icaljs.js in the source tree, only this declaration. tsc reads the types through it
 * and emits a plain require, which webpack resolves against the copied file. See the vendor step
 * in tools/pkjs/build-pkjs.ts.
 */
import ICAL from 'ical.js';

export default ICAL;
