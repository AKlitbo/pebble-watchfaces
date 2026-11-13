/**
 * Tells the layout builder which Data Sources the wearer has on the Pebble app plugin, so it can warn
 * about a weather, stock, or calendar panel the phone has no plugin for.
 *
 * Clay hands a builder component nothing but its own row, so the builder cannot read the source
 * selects itself. A face passes this as its customClay, which Clay runs with the page as `this`. Once
 * the page is built it reads each select and hands the answer to the builder through the
 * `_lbPluginSources` hook on its element, and again whenever one of them changes. A select missing
 * from the page counts as not picked.
 *
 * Clay serializes this and re-runs it inside the config webview, so it reaches nothing outside
 * itself: no imports and no module-scope constants.
 */

/** The bits of a Clay row this reaches. */
interface ClayRow {
  $element: Array<HTMLElement & { _lbPluginSources?: (picked: Record<string, boolean>) => void }>;
  get(): unknown;
  on(event: string, handler: () => void): void;
}

/** The bits of the Clay config page this reaches. */
interface ClayPage {
  EVENTS: { AFTER_BUILD: string };
  on(event: string, handler: () => void): void;
  getItemByMessageKey(messageKey: string): ClayRow | undefined;
}

export default function layoutPluginSources(this: ClayPage): void {
  this.on(this.EVENTS.AFTER_BUILD, () => {
    const builder = this.getItemByMessageKey('LAYOUT');
    const root = builder ? builder.$element[0] : undefined;

    if (!root || typeof root._lbPluginSources !== 'function') {
      return;
    }

    // each kind's source select, by the kind's name in the providers list
    const selects: Record<string, string> = { weather: 'WEATHER_PROVIDER', stocks: 'STOCK_PROVIDER', calendar: 'CALENDAR_SOURCE' };
    const kinds = Object.keys(selects);

    const push = (): void => {
      const picked: Record<string, boolean> = {};

      kinds.forEach((kind) => {
        const select = this.getItemByMessageKey(selects[kind]);

        picked[kind] = Boolean(select) && String((select as ClayRow).get()) === 'plugin';
      });

      if (root._lbPluginSources) {
        root._lbPluginSources(picked);
      }
    };

    kinds.forEach((kind) => {
      const select = this.getItemByMessageKey(selects[kind]);

      if (select) {
        select.on('change', push);
      }
    });

    push();
  });
}
