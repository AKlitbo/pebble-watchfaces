/**
 * A button that puts a list of rows back to their starting values, used for Reset Face Colours.
 *
 * Clay's colour rows have no "unset" the way the panel editor's swatches do, so the four face
 * pickers need a control that puts them back. It is a component rather than the page's custom
 * function, because a face that passes its own custom function stops the framework wiring the
 * visibleWhen rows. The markup is Clay's own button, so it picks up the same look.
 *
 * Clay serializes initialize with toString and re-runs it inside the config webview, so it reaches
 * nothing outside itself: no imports, no module-scope constants, and the defaults come off the rows
 * themselves rather than being written twice.
 */

/** The bits of a Clay row this reaches. */
interface ClayRow {
  config: { defaultValue?: unknown };
  set(value: unknown): void;
}

/** The bits of the Clay config page initialize is handed. */
interface ClayPage {
  getItemByMessageKey(messageKey: string): ClayRow | undefined;
}

/** What Clay binds initialize to: the button's own row and its config. */
interface ResetButtonContext {
  config: { resetKeys?: string[] };
  on(event: string, handler: () => void): void;
}

export default {
  name: 'resetButton',

  template: '<div class="component component-button">'
    + '<button type="button" data-manipulator-target class="{{primary ? \'primary\' : \'\'}}"'
    + ' {{each key: attributes}}{{key}}="{{this}}"{{/each}}></button>'
    + '{{if description}}<div class="description">{{{description}}}</div>{{/if}}'
    + '</div>',

  manipulator: 'button',

  defaults: {
    primary: false,
    attributes: {},
    description: '',
  },

  /**
   * Puts each row named in resetKeys back to its default on a click.
   *
   * A `function` expression, never shorthand method syntax. Clay writes a registered component out
   * as `key:` followed by the function's own toString, and a shorthand method stringifies with its
   * name attached, which stops the whole settings page parsing.
   *
   * @param minified Clay's little DOM helper, which this does not use.
   * @param clay The config page, used to find the rows to put back.
   * @return Nothing.
   */
  initialize: function (this: ResetButtonContext, minified: unknown, clay: ClayPage): void {
    const keys = this.config.resetKeys || [];

    this.on('click', () => {
      // this only restages the pickers. the user still saves to push them to the watch
      keys.forEach((key) => {
        const row = clay.getItemByMessageKey(key);

        // a row missing from the page is skipped so the rest still reset
        if (row) {
          row.set(row.config.defaultValue);
        }
      });
    });
  },
};
