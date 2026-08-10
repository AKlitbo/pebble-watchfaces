/**
 * Ambient types for the generated `*.g` modules the gen: tools emit
 * (the layout builder component and the thumbnail table). They carry no
 * hand-written types, so a .ts source that imports one gets it typed loosely.
 * At runtime the real `.g.js` resolves. Here it is just a shape to import.
 */
declare module '*.g' {
  // generated glue, not worth hand-typing, so callers get it loosely typed
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  const value: any;
  export = value;
}
