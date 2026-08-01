/**
 * The live colour preview: swatch painting, the mono fallbacks, the header
 * contrast flip and the little example panel the colour picker shows.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import { argbToCss } from './palette';
import type { ChannelKey, ColorRecord, ThemeModule } from '../types';

/** A painted example panel and the callback that repaints one channel. */
export interface ExampleBox {
  box: HTMLElement;
  paint: (key: ChannelKey, byte: number | null) => void;
}

/**
 * Paints a row swatch: the channel's colour, or the striped white that reads
 * as "left on mono".
 */
export function paintSwatch(swatch: HTMLElement, byte: number | null): void {
  if (byte == null) {
    swatch.style.background =
      'repeating-linear-gradient(45deg, #fff, #fff 4px, #ddd 4px, #ddd 8px)';
  } else {
    swatch.style.background = argbToCss(byte);
  }
}

/**
 * A channel's argb turned into css, falling back to the Custom theme's mono
 * look (white on black, a light gray caption) so an unset channel previews
 * the way the default renders.
 */
export function channelCss(channelKey: string, byte: number | null): string {
  if (byte != null) {
    return argbToCss(byte);
  }

  return channelKey === 'subtitle' ? '#AAAAAA' : '#FFFFFF';
}

/**
 * Black text on a light accent, white on a dark one, so the header title
 * stays readable (the same flip the firmware does). Mono accent is white, so
 * its title goes black.
 */
export function contrastText(byte: number | null): string {
  if (byte == null) {
    return '#000000';
  }

  const red = ((byte >> 4) & 3) * 85;
  const green = ((byte >> 2) & 3) * 85;
  const blue = (byte & 3) * 85;
  const luma = (red * 299 + green * 587 + blue * 114) / 1000; // rec. 601

  return luma > 140 ? '#000000' : '#FFFFFF';
}

/**
 * A generic "kinda looks like this" panel: header (accent) plus border
 * (accent), a value number, an icon glyph, and a caption. One shape for every
 * module, so it shows the colour mapping live rather than the real layout.
 * paint(key, byte) repaints one channel.
 */
export function buildExampleBox(
  module: Pick<ThemeModule, 'label'>,
  stagedColors: ColorRecord,
  hideHeader: boolean,
  hideBorder: boolean
): ExampleBox {
  const box = document.createElement('div');
  box.className = 'tb-ex';

  const header = document.createElement('div');
  header.className = 'tb-ex-hdr';
  header.textContent = (module.label || 'Module').toUpperCase();

  const value = document.createElement('div');
  value.className = 'tb-ex-val';
  value.textContent = '8,423';

  const subtitle = document.createElement('div');
  subtitle.className = 'tb-ex-sub';
  subtitle.textContent = 'OF 10,000';

  const icon = document.createElement('div');
  icon.className = 'tb-ex-icon';
  icon.textContent = '●'; // a plain dot stands in for the panel icon (tintable, unlike an emoji)

  // value up top under the header / caption below it / icon down at the bottom
  if (!hideHeader) {
    box.appendChild(header);
  }
  box.appendChild(value);
  box.appendChild(subtitle);
  box.appendChild(icon);

  function paintChannel(key: ChannelKey, byte: number | null): void {
    if (key === 'accent') {
      const accentCss = channelCss('accent', byte);
      if (!hideHeader) {
        header.style.background = accentCss;
        header.style.color = contrastText(byte);
      }
      box.style.border = '1px solid ' + (hideBorder ? 'transparent' : accentCss);
    } else if (key === 'value') {
      value.style.color = channelCss('value', byte);
    } else if (key === 'icon') {
      icon.style.color = channelCss('icon', byte);
    } else if (key === 'subtitle') {
      subtitle.style.color = channelCss('subtitle', byte);
    }
  }

  // paint every channel from the staged map so the whole panel previews at once
  (['accent', 'value', 'icon', 'subtitle'] as ChannelKey[]).forEach(function (key) {
    paintChannel(key, stagedColors[key]);
  });

  return { box: box, paint: paintChannel };
}
