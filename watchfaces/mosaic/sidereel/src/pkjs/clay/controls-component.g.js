// generated from core/pkjs/clay/builder/controls.manifest.ts by the Clay generator
// do not edit by hand: run `paf gen <face> clay` after changing the sources
/**
 * Clay custom component that carries the look every mosaic settings page
 * control shares: the colour tokens and one rule per role, from controls.css.
 *
 * A face registers it beside its builders and never places it on the page.
 * Clay adds a registered component's style to the page whether or not the
 * page uses it, so the layout builder and the appearance editor both pick it
 * up, and it appears once however many builders the page holds.
 */
module.exports = {
  name: "mosaicControls",

  template: "<div class=\"component\"></div>",

  style: ":root{--mo-page:#101216;--mo-card:#171a1f;--mo-panel:#1d2128;--mo-inset:#14161a;--mo-raise:#23262c;--mo-raise-press:#2c3038;--mo-line:#3d434d;--mo-line-control:#717986;--mo-ink:#eceef2;--mo-ink-2:#a3a9b3;--mo-ink-muted:#8b929c;--mo-accent:#ff6a1f;--mo-accent-press:#e85e15;--mo-accent-tint:rgba(255,106,31,0.2);--mo-on-accent:#1a0c03;--mo-danger:#c53a2c;--mo-danger-press:#a82f24;--mo-edge:2px;--mo-mono:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace;--pf-accent:var(--mo-accent);--pf-on-accent:var(--mo-on-accent)}:root .lb-btn,:root .lb-io-btn,:root .tb-io-btn,:root .lb-filter-all,:root .lb-ltab,:root .lb-tab,:root .lb-group{font-family:var(--mo-mono);font-weight:700}:root .tb-edit-btn,:root .tb-bar-btn,:root .component-button button,:root .component-submit button{font-family:var(--mo-mono);font-weight:800}:root .lb-btn,:root .lb-filter-all{letter-spacing:normal}:root .lb-ltab,:root .lb-tab,:root .lb-group{letter-spacing:0.5px}:root .lb-io-btn,:root .tb-io-btn,:root .tb-bar-btn{letter-spacing:1px}:root .tb-edit-btn,:root .component-button button{letter-spacing:1.5px}:root .component-submit button{letter-spacing:2px}:root .lb-btn,:root .lb-io-btn,:root .tb-io-btn,:root .tb-bar-btn,:root .tb-nav-btn,:root .tb-edit-btn,:root .lb-filter-all,:root .component-button button,:root .component-submit button{cursor:pointer;-webkit-tap-highlight-color:transparent;transition:background-color 0.12s,border-color 0.12s}:root .lb-btn,:root .lb-io-btn,:root .tb-io-btn,:root .tb-bar-btn.ghost,:root .tb-bar-btn.io,:root .tb-nav-btn,:root .lb-filter-all,:root .component-button button{background:var(--mo-raise);border-color:var(--mo-line-control);color:var(--mo-ink)}:root .lb-btn:active,:root .lb-io-btn:active,:root .tb-io-btn:active,:root .tb-bar-btn.ghost:active,:root .tb-bar-btn.io:active,:root .tb-nav-btn:active,:root .lb-filter-all:active,:root .component-button button:active{background:var(--mo-raise-press);border-color:var(--mo-ink-2)}:root .tb-edit-btn,:root .tb-bar-btn,:root .lb-io-btn.primary,:root .tb-io-btn.primary,:root .component-submit button{background:var(--mo-accent);border-color:var(--mo-accent);color:var(--mo-on-accent)}:root .tb-edit-btn:active,:root .tb-bar-btn:active,:root .lb-io-btn.primary:active,:root .tb-io-btn.primary:active,:root .component-submit button:active{background:var(--mo-accent-press);border-color:var(--mo-accent-press)}:root .lb-btn-clear{background:var(--mo-danger);border-color:var(--mo-danger);color:#fff}:root .lb-btn-clear:active{background:var(--mo-danger-press);border-color:var(--mo-danger-press)}:root .lb-ltab,:root .lb-tab,:root .tb-mv-opt{background:var(--mo-inset);color:var(--mo-ink-2);cursor:pointer;-webkit-tap-highlight-color:transparent}:root .lb-ltab{border-color:var(--mo-line-control)}:root .lb-tab{box-shadow:inset 0 0 0 var(--mo-edge) var(--mo-line-control)}:root .lb-ltab:active,:root .lb-tab:active,:root .tb-mv-opt:active{background:var(--mo-raise-press)}:root .lb-ltab.active,:root .lb-tab.active,:root .tb-mv-opt.sel{background:var(--mo-accent);border-color:var(--mo-accent);box-shadow:none;color:var(--mo-on-accent)}:root .tb-mv,:root .tb-mv-opt + .tb-mv-opt{border-color:var(--mo-line-control)}:root .lb-group{background:var(--mo-inset);border-bottom-color:var(--mo-line-control);box-shadow:inset 0 1px 0 var(--mo-line-control),inset 1px 0 0 var(--mo-line-control),inset -1px 0 0 var(--mo-line-control);color:var(--mo-ink-2);-webkit-tap-highlight-color:transparent}:root .lb-group:active{background:var(--mo-raise-press)}:root .lb-group[aria-pressed='true']{background:var(--mo-accent-tint);border-bottom-color:var(--mo-accent);box-shadow:inset 0 1px 0 var(--mo-accent),inset 1px 0 0 var(--mo-accent),inset -1px 0 0 var(--mo-accent);color:#ff8a4c}:root .lb-filter{color:var(--mo-ink-2)}:root .tb-swatch,:root .tb-toggle{border-color:var(--mo-line-control)}:root .lb-desc,:root .tb-hint,:root .lb-assign-lbl,:root .tb-chip-lbl,:root .row-note,:root .sz-noshot{color:var(--mo-ink-muted)}@media (hover:hover) and (pointer:fine){:root .lb-btn:not(.lb-btn-clear):hover,:root .lb-io-btn:not(.primary):hover,:root .tb-io-btn:not(.primary):hover,:root .tb-bar-btn.ghost:hover,:root .tb-bar-btn.io:hover,:root .tb-nav-btn:hover,:root .lb-filter-all:hover,:root .lb-ltab:hover,:root .component-button button:hover{border-color:var(--mo-accent)}:root .lb-tab:not(.active):hover{box-shadow:inset 0 0 0 var(--mo-edge) var(--mo-accent)}:root .lb-group:hover{border-bottom-color:var(--mo-accent);box-shadow:inset 0 1px 0 var(--mo-accent),inset 1px 0 0 var(--mo-accent),inset -1px 0 0 var(--mo-accent)}}:root button:focus-visible,:root summary:focus-visible,:root select:focus-visible,:root input:focus-visible,:root textarea:focus-visible,:root a:focus-visible{outline:2px solid var(--mo-accent);outline-offset:2px}:root .component-input input:not([type]){background:var(--mo-inset);border-radius:0;box-shadow:inset 0 0 0 1px var(--mo-line-control);color:var(--mo-ink);font-family:var(--mo-mono)}",

  manipulator: {
    set: function (value) {
      this.$element[0]._mcSet(value || '');
    },
    get: function () {
      return this.$element[0]._mcGet();
    }
  },

  initialize: function () {
    var __clayComponent = (() => {
      var __defProp = Object.defineProperty;
      var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
      var __getOwnPropNames = Object.getOwnPropertyNames;
      var __hasOwnProp = Object.prototype.hasOwnProperty;
      var __esm = (fn, res, err) => function __init() {
        if (err) throw err[0];
        try {
          return fn && (res = (0, fn[__getOwnPropNames(fn)[0]])(fn = 0)), res;
        } catch (e) {
          throw err = [e], e;
        }
      };
      var __commonJS = (cb, mod) => function __require() {
        try {
          return mod || (0, cb[__getOwnPropNames(cb)[0]])((mod = { exports: {} }).exports, mod), mod.exports;
        } catch (e) {
          throw mod = 0, e;
        }
      };
      var __export = (target, all) => {
        for (var name in all)
          __defProp(target, name, { get: all[name], enumerable: true });
      };
      var __copyProps = (to, from, except, desc) => {
        if (from && typeof from === "object" || typeof from === "function") {
          for (let key of __getOwnPropNames(from))
            if (!__hasOwnProp.call(to, key) && key !== except)
              __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
        }
        return to;
      };
      var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

      // core/pkjs/clay/builder/ts/controls/init.ts
      var init_exports = {};
      __export(init_exports, {
        init: () => init
      });
      function init() {
        const root = this.$element[0];
        root._mcSet = function() {
        };
        root._mcGet = function() {
          return "";
        };
      }
      var init_init = __esm({
        "core/pkjs/clay/builder/ts/controls/init.ts"() {
        }
      });

      // core/pkjs/clay/builder/component-entry.js
      var require_component_entry = __commonJS({
        "core/pkjs/clay/builder/component-entry.js"(exports, module) {
          init_init();
          module.exports = (init_init(), __toCommonJS(init_exports));
        }
      });
      return require_component_entry();
    })();

    __clayComponent.init.call(this);
  }
};
