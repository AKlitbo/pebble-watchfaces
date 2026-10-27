/**
 * The layout builder's palette: a tab per block size, a row of group tabs under it, the
 * draggable panel tiles, and a status line under them saying which group is showing.
 *
 * The group row keeps each list short, so a panel is never a long drag away from the face above
 * it. A tab opens on its first group rather than on everything for the same reason.
 *
 * This is a Clay builder piece. esbuild bundles it into the component's
 * initialize, which runs in the config webview, so it sticks to browser APIs.
 */

import type { ModuleGroup, ModuleInfo } from '../types';

/** One group tab: its name, which the status line shows too, and its icon. */
export interface PaletteGroup {
  name: ModuleGroup | 'Other';
  icon: string;
}

// keyed by every ModuleGroup, so a group added to the type without a tab here fails to compile
// rather than quietly dropping its panels into Other. the key order is the tab order
const GROUP_ICONS: Record<ModuleGroup, string> = {
  'Clock': '⏰',
  'Date': '📅',
  'Progress': '📊',
  'Weather': '🌤',
  'Sky': '🌗',
  'Health': '❤️',
  'Goals': '🎯',
  'Calendar & Stocks': '📈',
  'Status': '🔋',
};

/** The groups in the order their tabs show. */
export const PALETTE_GROUPS: PaletteGroup[] = (Object.keys(GROUP_ICONS) as ModuleGroup[]).map(function (name) {
  return { name: name, icon: GROUP_ICONS[name] };
});

// a panel with no group, or one this list does not know, still shows under here
const OTHER_GROUP: PaletteGroup = { name: 'Other', icon: '·' };

// the group tab that shows every panel in the size
const ALL = 'All';

/** One size tab. A wide size spans the grid, so its shots are twice as wide. */
export interface PaletteSize {
  key: string;
  wide: boolean;
}

/** One group as a size tab shows it, with its panels in list order. */
export interface PaletteSection {
  group: PaletteGroup;
  modules: ModuleInfo[];
}

/**
 * The groups a size tab shows, in PALETTE_GROUPS order, each holding its panels in the order the
 * module list has them. A group with no panel at this size is left out. A panel with no known group
 * lands in Other at the end, so a new panel nobody grouped still shows.
 *
 * @param modules The placeable modules.
 * @param size The size key, such as "1x2".
 * @return The groups with at least one panel at this size.
 */
export function groupsForSize(modules: ModuleInfo[], size: string): PaletteSection[] {
  const known: Record<string, boolean> = {};
  const sections: PaletteSection[] = [];
  const atSize = modules.filter(function (module) {
    return module.sizes.indexOf(size) !== -1;
  });

  for (let index = 0; index < PALETTE_GROUPS.length; index++) {
    const group = PALETTE_GROUPS[index];
    const members = atSize.filter(function (module) {
      return module.group === group.name;
    });

    known[group.name] = true;

    if (members.length > 0) {
      sections.push({ group: group, modules: members });
    }
  }

  const others = atSize.filter(function (module) {
    return !module.group || !known[module.group];
  });

  if (others.length > 0) {
    sections.push({ group: OTHER_GROUP, modules: others });
  }

  return sections;
}

/** What the palette cannot own itself, handed in by the layout builder. */
export interface PaletteEnv {
  container: HTMLElement;
  modules: ModuleInfo[];
  sizes: PaletteSize[];
  thumbFor(value: number, size: string): string | null;
  onPick(module: ModuleInfo, size: string, event: PointerEvent): void;
}

/** The palette's handle for the builder. */
export interface Palette {
  markPlaced(values: number[]): void;
}

/**
 * Builds the palette into the container.
 *
 * The group picked on each size tab is kept while the page is open, so switching size and back
 * lands on the same group. A size with a single group shows its panels with no group row, since
 * there is nothing to pick between.
 *
 * @param env The container, the modules, the size tabs, the thumbnail lookup, and what a drag start calls.
 * @return markPlaced, which dims the tiles already on the face.
 */
export function createPalette(env: PaletteEnv): Palette {
  const chosen: Record<string, string> = {};
  let placed: number[] = [];
  let activeSize = '';
  let activeWide = false;

  const wrap = document.createElement('div');

  wrap.className = 'lb-drawer-wrap';

  const tabBar = document.createElement('div');

  tabBar.className = 'lb-tabs';

  const groupBar = document.createElement('div');

  groupBar.className = 'lb-groups';

  const panes = document.createElement('div');

  panes.className = 'lb-drawer-panes';

  const pane = document.createElement('div');

  pane.className = 'lb-drawer-pane';
  // the group row sits inside the size tab's box, since its groups belong to that size
  panes.appendChild(groupBar);
  panes.appendChild(pane);

  function buildTile(module: ModuleInfo, size: string): HTMLElement {
    const tile = document.createElement('div');
    const shot = env.thumbFor(module.value, size);

    // a full width block shows a wide shot, so its tile gets twice the room
    tile.className = 'lb-pal-item ' + (activeWide ? 'wide' : 'narrow');
    tile.dataset.value = String(module.value);
    tile.title = module.label;

    if (shot) {
      const img = document.createElement('img');

      tile.style.background = '#000';
      img.className = 'lb-pal-img';
      img.src = shot;
      img.alt = module.label;
      tile.appendChild(img);
    } else {
      const icon = document.createElement('div');

      tile.style.background = module.color;
      icon.className = 'lb-pal-icon';
      icon.textContent = module.icon;
      tile.appendChild(icon);
    }

    tile.addEventListener('pointerdown', function (event) {
      env.onPick(module, size, event as PointerEvent);
    });

    return tile;
  }

  function buildGroupTab(group: PaletteGroup | null, pressed: boolean): HTMLElement {
    const tab = document.createElement('button');
    const icon = document.createElement('span');
    const name = document.createElement('span');

    tab.type = 'button';
    tab.className = 'lb-group';
    tab.title = group ? group.name : 'Every panel';
    tab.setAttribute('aria-pressed', String(pressed));
    icon.className = 'lb-group-ic';
    icon.textContent = group ? group.icon : '▦';
    name.className = 'lb-group-nm';
    name.textContent = group ? group.name : ALL;
    tab.appendChild(icon);
    tab.appendChild(name);
    tab.addEventListener('click', function () {
      chosen[activeSize] = group ? group.name : ALL;
      draw();
    });

    return tab;
  }

  function applyPlaced(): void {
    const tiles = pane.querySelectorAll<HTMLElement>('.lb-pal-item');

    for (let index = 0; index < tiles.length; index++) {
      const value = parseInt(tiles[index].dataset.value || '', 10);

      tiles[index].classList.toggle('placed', placed.indexOf(value) !== -1);
    }
  }

  // redraws the group row and the list for the active size
  function draw(): void {
    const sections = groupsForSize(env.modules, activeSize);
    const single = sections.length < 2;
    let pick = chosen[activeSize];
    let total = 0;

    for (let index = 0; index < sections.length; index++) {
      total += sections[index].modules.length;
    }

    if (single) {
      pick = ALL;
    } else if (pick !== ALL && !sections.some(function (section) { return section.group.name === pick; })) {
      pick = sections[0].group.name;
    }

    while (groupBar.firstChild) {
      groupBar.removeChild(groupBar.firstChild);
    }

    groupBar.hidden = single;

    for (let index = 0; index < sections.length && !single; index++) {
      groupBar.appendChild(buildGroupTab(sections[index].group, sections[index].group.name === pick));
    }

    if (!single) {
      groupBar.appendChild(buildGroupTab(null, pick === ALL));
    }

    while (pane.firstChild) {
      pane.removeChild(pane.firstChild);
    }

    const showing = pick === ALL ? null : sections.filter(function (section) {
      return section.group.name === pick;
    })[0];

    const drawer = document.createElement('div');

    drawer.className = 'lb-drawer';

    const listed = showing ? [showing] : sections;

    for (let index = 0; index < listed.length; index++) {
      for (let moduleIndex = 0; moduleIndex < listed[index].modules.length; moduleIndex++) {
        drawer.appendChild(buildTile(listed[index].modules[moduleIndex], activeSize));
      }
    }

    pane.appendChild(drawer);

    // the status line under the tiles is what makes the filter impossible to miss, so it names
    // the group and what is left out, with a way back to every panel
    if (showing) {
      const status = document.createElement('div');
      const what = document.createElement('span');
      const all = document.createElement('button');

      status.className = 'lb-filter';
      what.textContent = showing.group.name + ' · ' + showing.modules.length + ' of ' + total;
      all.type = 'button';
      all.className = 'lb-filter-all';
      all.textContent = 'Show all';
      all.addEventListener('click', function () {
        chosen[activeSize] = ALL;
        draw();
      });
      status.appendChild(what);
      status.appendChild(all);
      pane.appendChild(status);
    }

    applyPlaced();
  }

  function selectSize(size: PaletteSize): void {
    const tabs = tabBar.querySelectorAll<HTMLElement>('.lb-tab');

    activeSize = size.key;
    activeWide = size.wide;

    for (let index = 0; index < tabs.length; index++) {
      tabs[index].classList.toggle('active', tabs[index].dataset.size === size.key);
    }

    draw();
  }

  let first: PaletteSize | null = null;

  for (let index = 0; index < env.sizes.length; index++) {
    const size = env.sizes[index];

    // a size no module comes in gets no tab
    if (groupsForSize(env.modules, size.key).length === 0) {
      continue;
    }

    const tab = document.createElement('button');

    tab.type = 'button';
    tab.className = 'lb-tab';
    tab.dataset.size = size.key;
    tab.textContent = size.key;
    tab.addEventListener('click', function () {
      selectSize(size);
    });
    tabBar.appendChild(tab);

    if (!first) {
      first = size;
    }
  }

  wrap.appendChild(tabBar);
  wrap.appendChild(panes);
  env.container.appendChild(wrap);

  if (first) {
    selectSize(first);
  }

  return {
    markPlaced: function (values) {
      placed = values.slice();
      applyPlaced();
    },
  };
}
