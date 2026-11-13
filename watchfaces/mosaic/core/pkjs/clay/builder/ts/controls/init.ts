/**
 * The controls component's wiring, which is only the manipulator handles.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize and calls init there with the component as this. The component
 * carries a stylesheet and holds no value, so a face never places it on the
 * page. The handles are here so Clay's manipulator still has something to call
 * if one ever is.
 */

import type { ClayComponentInstance } from '../types';

/** The set/get handles init hangs on the root element for the manipulator. */
interface McHooks {
  _mcSet(value: string): void;
  _mcGet(): string;
}

/**
 * Hangs set and get handles that keep nothing, since the component has no
 * value of its own.
 */
export function init(this: ClayComponentInstance): void {
  const root = this.$element[0] as HTMLElement & McHooks;

  root._mcSet = function () {};

  root._mcGet = function () {
    return '';
  };
}
