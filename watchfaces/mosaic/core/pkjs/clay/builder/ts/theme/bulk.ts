/**
 * The every panel colour sweep: writes the channels picked once into every
 * module the appearance editor lists.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { vibrantByteFor } from './model';
import type { ChannelKey, ChannelPicks, ColorMap, ThemeModule } from '../types';

/**
 * Writes each picked channel into every listed module's colour record. A channel left out of
 * picks keeps each module's own colour, so a sweep can set the value colour everywhere and leave
 * the accents alone. 'vibrant' gives each module its own signature colour, and mono for a module
 * that has none.
 *
 * Only the listed modules are written. A themeHidden module draws with the colour of the row it
 * shares, so it follows along.
 *
 * @param colors The live colour map, changed in place.
 * @param modules The modules the editor lists.
 * @param picks The channels to set and what to set them to.
 * @param argbByName Palette name to argb byte, for the vibrant colours.
 * @return Nothing. colors carries the result.
 */
export function colourAll(
  colors: ColorMap,
  modules: ThemeModule[],
  picks: ChannelPicks,
  argbByName: Record<string, number>
): void {
  const keys = Object.keys(picks) as ChannelKey[];

  for (let index = 0; index < modules.length; index++) {
    const module = modules[index];

    if (!colors[module.value]) {
      colors[module.value] = { accent: null, value: null, icon: null, subtitle: null };
    }

    for (let k = 0; k < keys.length; k++) {
      const pick = picks[keys[k]];

      colors[module.value][keys[k]] = pick === 'vibrant' ? vibrantByteFor(module, argbByName) : pick != null ? pick : null;
    }
  }
}
