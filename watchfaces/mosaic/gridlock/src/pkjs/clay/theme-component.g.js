// generated from core/pkjs/clay/builder/theme.manifest.ts by the family core's tools/clay-components/generate-components.ts
// do not edit by hand: run `npm run gen:clay` after changing the sources
/**
 * Clay custom component for per-module appearance: colours plus header and
 * border on/off toggles.
 *
 * Shows an "Edit Module Appearance" button. Tapping it opens a full screen
 * editor that lists every module with four colour swatches (accent, value,
 * icon, sub) and per size Header/Border toggles. Tapping a swatch opens the
 * 64 colour Pebble palette. A channel left alone stays mono (a striped
 * swatch).
 *
 * The value it round trips is the APPEARANCE_CUSTOM_COLORS wire string the
 * watch parses. The current sparse "~3" format is a colour section then "|"
 * then a flag section. A colour record is 5 chars (id + four channels), a
 * colour char is one url-safe base64 symbol holding the palette index 0..63
 * or "." to leave that channel mono. A flag record is 3 chars (id + two
 * packed flag chars). "0" means nothing set. The colours apply under the
 * Custom theme, the header and border flags in every theme. The codec also
 * reads and migrates the two older formats, see ts/theme/codec.ts.
 *
 * IMPORTANT: initialize and the manipulator run inside the Clay config
 * webview, a separate JS context. They must be self contained, which is why
 * the generator inlines every piece below into one initialize.
 */
module.exports = {
  name: "themeBuilder",

  template: "<div class=\"component tb\">  <button type=\"button\" class=\"tb-edit-btn\">Tap to Edit Module Appearance</button>  <p class=\"tb-hint\">Per-module colours (used by the Custom theme) and show/hide module headers.</p>  <input type=\"hidden\" class=\"tb-value\"></div>",

  style: "body{background-color:#101216 !important;background-image:linear-gradient(rgba(255,255,255,0.05) 1px,transparent 1px),linear-gradient(90deg,rgba(255,255,255,0.05) 1px,transparent 1px);background-size:22px 22px;padding-bottom:40px !important}.section{background:#171a1f !important;border:2px solid #3d434d !important;border-radius:0 !important;box-shadow:none !important}.component-heading{color:#ff6a1f !important;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace !important;letter-spacing:2px !important}.component-heading:first-child{background:#1d2128 !important;border-radius:0 !important}.section>.component:first-child{margin-top:0 !important}select{background:#14161a !important;color:#eceef2 !important;border:1px solid #3d434d !important;border-radius:0 !important;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace !important}input[type='text'],input[type='email'],input[type='url'],input[type='search'],input[type='number'],input[type='password']{background:#14161a !important;color:#eceef2 !important;border:1px solid #3d434d !important;border-radius:0 !important;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace !important}.component-submit button,button[type='submit']{display:block !important;width:100% !important;min-width:0 !important;box-sizing:border-box !important;margin:0 !important;padding:13px !important;background:#ff6a1f !important;color:#1a0c03 !important;border:2px solid #ff6a1f !important;border-radius:0 !important;font-weight:800 !important;letter-spacing:2px !important;cursor:pointer !important;transition:background 0.12s !important}.component-submit{padding-bottom:.7rem !important}.component-submit button:hover,button[type='submit']:hover{background:#ff7d38 !important;border-color:#ff7d38 !important}.component-submit button:active,button[type='submit']:active{background:#e85e15 !important;border-color:#e85e15 !important}.component-button button{display:block !important;width:100% !important;min-width:0 !important;box-sizing:border-box !important;margin:0 !important;padding:10px 14px !important;background:transparent !important;color:#eceef2 !important;border:2px solid #3d434d !important;border-radius:0 !important;font-size:12px !important;font-weight:800 !important;letter-spacing:1.5px !important;text-transform:uppercase !important;cursor:pointer !important;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace !important;transition:background 0.12s !important}.component-button button:hover{background:#1d2128 !important}.component-button button:active{background:#23262c !important}.loc-search .loc-list{background:#171a1f;color:#eceef2;border:2px solid #3d434d;border-radius:0}.loc-search .loc-item{border-bottom:1px solid #2c3038}.loc-search .loc-item:hover,.loc-search .loc-item:active{background:#ff6a1f;color:#1a0c03}.tb{padding:12px 0;user-select:none;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-edit-btn{display:block;width:100%;box-sizing:border-box;min-width:0 !important;margin:0 !important;padding:10px 14px !important;border:2px solid #ff6a1f;border-radius:0;background:#ff6a1f;color:#1a0c03;font-size:12px;font-weight:800;letter-spacing:1.5px;text-transform:uppercase;cursor:pointer;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-edit-btn:active{background:#e85e15}.tb-hint{font-size:11px;color:#6a707a;line-height:1.4;margin:8px 2px 0}.tb-overlay{position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(8,9,11,0.8);z-index:50000}.tb-sheet{position:fixed;top:0;left:0;right:0;bottom:0;z-index:50001;background-color:#131519;background-image:linear-gradient(rgba(255,255,255,0.045) 1px,transparent 1px),linear-gradient(90deg,rgba(255,255,255,0.045) 1px,transparent 1px);background-size:22px 22px;display:flex;flex-direction:column}.tb-bar{display:flex;gap:10px;padding:12px;flex-shrink:0;background:#191c22;border-bottom:2px solid #3d434d}.tb-bar-bottom{border-bottom:none;border-top:2px solid #3d434d}.tb-bar-btn{flex:1;min-width:0 !important;margin:0 !important;padding:10px 6px !important;border:2px solid #ff6a1f;border-radius:0;background:#ff6a1f;color:#1a0c03;font-size:11px;font-weight:800;letter-spacing:1px;text-transform:uppercase;cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-bar-btn:active{background:#e85e15}.tb-bar-btn.ghost{background:transparent;color:#eceef2;border:2px solid #3d434d}.tb-bar-btn.ghost:active{background:#23262c}.tb-bar-btn.io{background:#23262c;color:#eceef2;border:2px solid #3d434d}.tb-bar-btn.io:active{background:#2c3038}.tb-io-textarea{width:100%;height:90px;padding:8px;font-family:ui-monospace,Menlo,Consolas,monospace;font-size:11px;border:2px solid #3d434d;border-radius:0;margin-bottom:12px;resize:none;box-sizing:border-box;background:#14161a;color:#eceef2}.tb-io-btns{display:flex;gap:8px}.tb-io-btn{flex:1;min-width:0 !important;margin:0 !important;padding:10px 12px !important;border:2px solid #3d434d;border-radius:0;font-size:12px;font-weight:700;letter-spacing:1px;text-transform:uppercase;cursor:pointer;background:#23262c;color:#eceef2;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-io-btn.primary{background:#ff6a1f;color:#1a0c03;border-color:#ff6a1f}.tb-io-btn.primary:active{background:#e85e15}.tb-legend{flex-shrink:0;padding:12px;border-bottom:2px solid #3d434d;background:#1d2128}.tb-legend-ttl{font-size:10px;font-weight:800;text-transform:uppercase;letter-spacing:2px;color:#ff6a1f;margin-bottom:8px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-legend-grid{display:grid;grid-template-columns:74px 1fr;column-gap:4px;row-gap:6px;align-items:start}.tb-lg-term{font-size:11px;color:#eceef2;font-weight:700;white-space:nowrap;padding-right:8px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-lg-dot{display:inline-block;width:8px;height:8px;margin-right:8px;vertical-align:middle;background:#ff6a1f}.tb-lg-dot.empty{background:transparent}.tb-lg-desc{font-size:11px;color:#a3a9b3;line-height:1.45;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-scroll{flex:1;overflow-y:auto;display:flex;flex-direction:column}.tb-list{padding:12px;display:flex;flex-direction:column;gap:10px}.tb-row{display:flex;flex-direction:column;padding:0 0 2px;border-radius:0;background:#171a1f;border:2px solid #3d434d}.tb-row-head{display:flex;align-items:center;gap:11px;padding:10px 12px 8px;border-bottom:1px solid #2c3038}.tb-row-ic{font-size:16px;width:28px;height:28px;flex-shrink:0;display:flex;align-items:center;justify-content:center;border-radius:0;background:#14161a;border:1px solid #3d434d}.tb-row-nm{font-size:12px;font-weight:700;color:#eceef2;flex:1;min-width:0;letter-spacing:1px;text-transform:uppercase;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-chips{display:flex;gap:6px;flex-shrink:0}.tb-chip{display:flex;flex-direction:column;align-items:center;gap:3px}.tb-chip-lbl{font-size:8px;font-weight:700;text-transform:uppercase;letter-spacing:0.4px;color:#6a707a;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-swatch{width:26px;height:26px;border-radius:0;border:2px solid #3d434d;cursor:pointer;box-sizing:border-box}.tb-swatch:active{transform:scale(0.94)}.tb-toggle{width:30px;height:28px;border-radius:0;border:1px solid #3d434d;display:flex;align-items:center;justify-content:center;font-size:11px;font-weight:800;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace;color:#fff;cursor:pointer;box-sizing:border-box}.tb-toggle:active{transform:scale(0.92)}.sz-list{margin:0;padding:8px 12px 10px;display:flex;flex-direction:column;gap:8px}.sz{display:flex;align-items:center;gap:11px;padding:0}.sz-shot{background:#000;border-radius:0;flex-shrink:0;display:flex;align-items:center;justify-content:center;overflow:hidden;border:2px solid #3d434d}.sz-shot img{max-width:100%;max-height:100%;width:auto;height:auto;display:block;image-rendering:pixelated;border-radius:0}.sz-noshot{color:#6a707a;font-size:9px}.sz-lbl{font-size:11px;font-weight:700;letter-spacing:1px;color:#a3a9b3;min-width:30px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.sz-tog{display:flex;gap:6px;margin-left:auto}.row-note{font-size:10px;color:#6a707a;padding:0 12px 4px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-pick-overlay{position:fixed;top:0;left:0;right:0;bottom:0;background:rgba(8,9,11,0.7);z-index:60000}.tb-pick{position:fixed;left:50%;top:50%;transform:translate(-50%,-50%);z-index:60001;background:#171a1f;border:2px solid #3d434d;border-radius:0;padding:14px;box-shadow:0 20px 60px -20px rgba(0,0,0,0.9);width:280px;max-width:90vw}.tb-pick-title{font-size:12px;font-weight:800;text-transform:uppercase;letter-spacing:1px;color:#ff6a1f;margin-bottom:10px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-grid{display:grid;grid-template-columns:repeat(8,1fr);gap:4px}.tb-cell{width:100%;padding-bottom:100%;border-radius:0;border:1px solid rgba(255,255,255,0.18);cursor:pointer;box-sizing:border-box;position:relative}.tb-cell.sel{outline:2px solid #ff6a1f;outline-offset:1px}.tb-cell-mono{background:repeating-linear-gradient(45deg,#fff,#fff 3px,#cfcfcf 3px,#cfcfcf 6px)}.tb-mv{display:flex;align-items:stretch;margin-bottom:10px;border-radius:0;overflow:hidden;border:2px solid #3d434d}.tb-mv-opt{flex:1;display:flex;align-items:center;justify-content:center;gap:6px;padding:9px;cursor:pointer;background:#23262c;color:#eceef2;font-size:12px;font-weight:700;text-transform:uppercase;letter-spacing:1px;font-family:ui-monospace,'Cascadia Mono',Menlo,Consolas,monospace}.tb-mv-opt:active{background:#2c3038}.tb-mv-opt + .tb-mv-opt{border-left:2px solid #3d434d}.tb-mv-opt.sel{background:#ff6a1f;color:#1a0c03}.tb-mv-ind{width:18px;height:18px;border-radius:0;flex-shrink:0;box-sizing:border-box;border:1px solid rgba(255,255,255,0.25)}.tb-ex{width:130px;margin:0 auto 12px;background:#000;border:1px solid #fff;border-radius:0;overflow:hidden;font-family:system-ui,-apple-system,sans-serif}.tb-ex-hdr{font-size:9px;font-weight:700;letter-spacing:0.5px;padding:2px 5px;background:#fff;color:#000;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}.tb-ex-val{font-size:20px;font-weight:700;text-align:center;color:#fff;padding:3px 4px 0}.tb-ex-sub{font-size:8px;font-weight:700;letter-spacing:0.4px;text-align:center;color:#AAA;padding:1px 4px 0}.tb-ex-icon{font-size:13px;line-height:1;text-align:center;color:#fff;padding:3px 4px 5px}.tb-pick-bar{display:flex;gap:8px;margin-top:12px}.tb-pick-nav{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:12px}.tb-pick-nav .tb-pick-title{margin-bottom:0;flex:1;text-align:center}.tb-nav-btn{min-width:0 !important;margin:0 !important;padding:2px 12px !important;border:1px solid #3d434d;border-radius:0;background:#23262c;color:#eceef2;font-size:16px;font-weight:700;line-height:1.3;cursor:pointer}.tb-nav-btn:active{background:#2c3038}",

  manipulator: {
    set: function (value) {
      this.$element[0]._tbSet(value || '');
    },
    get: function () {
      return this.$element[0]._tbGet();
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

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/thumbs.ts
      var thumbs_exports = {};
      __export(thumbs_exports, {
        thumbByLabel: () => thumbByLabel
      });
      function thumbByLabel(thumbs, label, size) {
        const byModule = thumbs[label];
        return byModule && byModule[size] || null;
      }
      var init_thumbs = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/thumbs.ts"() {
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/overlay.ts
      var overlay_exports = {};
      __export(overlay_exports, {
        createOverlayHost: () => createOverlayHost
      });
      function createOverlayHost(overlayClass, panelClass, dismissOnTap) {
        let backdrop = null;
        let panel = null;
        function close() {
          if (backdrop && backdrop.parentNode) {
            backdrop.parentNode.removeChild(backdrop);
          }
          if (panel && panel.parentNode) {
            panel.parentNode.removeChild(panel);
          }
          backdrop = null;
          panel = null;
        }
        function open() {
          close();
          backdrop = document.createElement("div");
          backdrop.className = overlayClass;
          if (dismissOnTap) {
            backdrop.addEventListener("click", close);
          }
          panel = document.createElement("div");
          panel.className = panelClass;
          document.body.appendChild(backdrop);
          document.body.appendChild(panel);
          return panel;
        }
        return { open, close };
      }
      var init_overlay = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/overlay.ts"() {
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/io-panel.ts
      var io_panel_exports = {};
      __export(io_panel_exports, {
        buildIoPanel: () => buildIoPanel
      });
      function buildIoPanel(panel, opts) {
        const title = document.createElement("div");
        title.className = opts.css.title;
        title.textContent = opts.title;
        panel.appendChild(title);
        const textarea = document.createElement("textarea");
        textarea.className = opts.css.textarea;
        textarea.value = opts.value;
        panel.appendChild(textarea);
        const buttons = document.createElement("div");
        buttons.className = opts.css.buttons;
        const copyButton = document.createElement("button");
        copyButton.type = "button";
        copyButton.className = opts.css.button;
        copyButton.textContent = "Copy";
        copyButton.addEventListener("click", function() {
          textarea.select();
          document.execCommand("copy");
          copyButton.textContent = "Copied!";
          setTimeout(function() {
            copyButton.textContent = "Copy";
          }, opts.copyResetMs);
        });
        buttons.appendChild(copyButton);
        const applyButton = document.createElement("button");
        applyButton.type = "button";
        applyButton.className = opts.css.button + " primary";
        applyButton.textContent = "Apply";
        applyButton.addEventListener("click", function() {
          opts.onApply(textarea.value);
        });
        buttons.appendChild(applyButton);
        panel.appendChild(buttons);
      }
      var init_io_panel = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/shared/io-panel.ts"() {
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/palette.ts
      var palette_exports = {};
      __export(palette_exports, {
        PEBBLE_COLORS_CSV: () => PEBBLE_COLORS_CSV,
        argbToCss: () => argbToCss,
        buildArgbByName: () => buildArgbByName,
        buildPalette: () => buildPalette,
        rgbToCss: () => rgbToCss,
        toHexByte: () => toHexByte
      });
      function buildPalette(csv) {
        const entries = csv.split(",");
        const palette = [];
        for (let i = 0; i < entries.length; i++) {
          const entry = entries[i];
          const hex = entry.substr(0, 6);
          const red = parseInt(hex.substr(0, 2), 16);
          const green = parseInt(hex.substr(2, 2), 16);
          const blue = parseInt(hex.substr(4, 2), 16);
          const argb = 192 + Math.round(red / 85) * 16 + Math.round(green / 85) * 4 + Math.round(blue / 85);
          palette.push({ argb, css: "#" + hex, name: entry.substr(7) });
        }
        return palette;
      }
      function buildArgbByName(palette) {
        const argbByName = {};
        for (let i = 0; i < palette.length; i++) {
          argbByName[palette[i].name] = palette[i].argb;
        }
        return argbByName;
      }
      function toHexByte(number) {
        const text = number.toString(16).toUpperCase();
        return text.length < 2 ? "0" + text : text;
      }
      function rgbToCss(red, green, blue) {
        return "#" + toHexByte(red) + toHexByte(green) + toHexByte(blue);
      }
      function argbToCss(argb) {
        const red = (argb >> 4 & 3) * 85;
        const green = (argb >> 2 & 3) * 85;
        const blue = (argb & 3) * 85;
        return rgbToCss(red, green, blue);
      }
      var PEBBLE_COLORS_CSV;
      var init_palette = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/palette.ts"() {
          PEBBLE_COLORS_CSV = "000000 Black,000055 Oxford Blue,0000AA Duke Blue,0000FF Blue,005500 Dark Green,005555 Midnight Green,0055AA Cobalt Blue,0055FF Blue Moon,00AA00 Islamic Green,00AA55 Jaeger Green,00AAAA Tiffany Blue,00AAFF Vivid Cerulean,00FF00 Green,00FF55 Malachite,00FFAA Medium Spring Green,00FFFF Cyan,550000 Bulgarian Rose,550055 Imperial Purple,5500AA Indigo,5500FF Electric Ultramarine,555500 Army Green,555555 Dark Gray,5555AA Liberty,5555FF Very Light Blue,55AA00 Kelly Green,55AA55 May Green,55AAAA Cadet Blue,55AAFF Picton Blue,55FF00 Bright Green,55FF55 Screamin Green,55FFAA Medium Aquamarine,55FFFF Electric Blue,AA0000 Dark Candy Apple Red,AA0055 Jazzberry Jam,AA00AA Purple,AA00FF Vivid Violet,AA5500 Windsor Tan,AA5555 Rose Vale,AA55AA Purpureus,AA55FF Lavender Indigo,AAAA00 Limerick,AAAA55 Brass,AAAAAA Light Gray,AAAAFF Baby Blue Eyes,AAFF00 Spring Bud,AAFF55 Inchworm,AAFFAA Mint Green,AAFFFF Celeste,FF0000 Red,FF0055 Folly,FF00AA Fashion Magenta,FF00FF Magenta,FF5500 Orange,FF5555 Sunset Orange,FF55AA Brilliant Rose,FF55FF Shocking Pink,FFAA00 Chrome Yellow,FFAA55 Rajah,FFAAAA Melon,FFAAFF Rich Brilliant Lavender,FFFF00 Yellow,FFFF55 Icterine,FFFFAA Pastel Yellow,FFFFFF White";
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/codec.ts
      var codec_exports = {};
      __export(codec_exports, {
        COLOR_ALPHABET: () => COLOR_ALPHABET,
        FORMAT_MARKER: () => FORMAT_MARKER,
        SIZE_ORDER: () => SIZE_ORDER,
        channelToken: () => channelToken,
        decodeChannel: () => decodeChannel,
        flagOn: () => flagOn,
        flagToken: () => flagToken,
        flagValue: () => flagValue,
        idToken: () => idToken,
        packFlags: () => packFlags,
        parseAppearance: () => parseAppearance,
        readColours: () => readColours,
        serializeAppearance: () => serializeAppearance,
        setFlag: () => setFlag,
        unpackFlags: () => unpackFlags
      });
      function channelToken(byte) {
        return byte == null ? "." : COLOR_ALPHABET.charAt(byte & 63);
      }
      function flagToken(sixBits) {
        return sixBits === 0 ? "." : COLOR_ALPHABET.charAt(sixBits & 63);
      }
      function flagValue(ch) {
        const idx = COLOR_ALPHABET.indexOf(ch);
        return idx < 0 ? 0 : idx;
      }
      function idToken(id) {
        return COLOR_ALPHABET.charAt(id & 63);
      }
      function decodeChannel(ch) {
        const idx = COLOR_ALPHABET.indexOf(ch);
        return idx < 0 ? null : 192 + idx;
      }
      function flagOn(map, moduleValue, size) {
        return Boolean(map[moduleValue] && map[moduleValue][size]);
      }
      function setFlag(map, moduleValue, size, on) {
        if (!map[moduleValue]) {
          map[moduleValue] = {};
        }
        map[moduleValue][size] = on;
      }
      function packFlags(headerless, borderless, id) {
        let byte = 0;
        for (let s = 0; s < SIZE_ORDER.length; s++) {
          const size = SIZE_ORDER[s];
          if (flagOn(headerless, id, size)) {
            byte |= 1 << s;
          }
          if (flagOn(borderless, id, size)) {
            byte |= 1 << 4 + s;
          }
        }
        return byte;
      }
      function unpackFlags(headerless, borderless, id, byte) {
        for (let s = 0; s < SIZE_ORDER.length; s++) {
          const size = SIZE_ORDER[s];
          if (byte & 1 << s) {
            setFlag(headerless, id, size, true);
          }
          if (byte & 1 << 4 + s) {
            setFlag(borderless, id, size, true);
          }
        }
      }
      function readColours(colors, text, id, offset) {
        const accent = decodeChannel(text.charAt(offset));
        const value = decodeChannel(text.charAt(offset + 1));
        const icon = decodeChannel(text.charAt(offset + 2));
        const subtitle = decodeChannel(text.charAt(offset + 3));
        if (accent != null || value != null || icon != null || subtitle != null) {
          colors[id] = { accent, value, icon, subtitle };
        }
      }
      function serializeAppearance(colors, headerless, borderless) {
        const colorIds = Object.keys(colors).sort(function(a, b) {
          return parseInt(a, 10) - parseInt(b, 10);
        });
        const colorRecs = [];
        for (let i = 0; i < colorIds.length; i++) {
          const id = parseInt(colorIds[i], 10);
          const c = colors[id];
          if (!c || c.accent == null && c.value == null && c.icon == null && c.subtitle == null) {
            continue;
          }
          colorRecs.push(idToken(id) + channelToken(c.accent) + channelToken(c.value) + channelToken(c.icon) + channelToken(c.subtitle));
        }
        const flagSeen = {};
        let key;
        for (key in headerless) {
          flagSeen[key] = true;
        }
        for (key in borderless) {
          flagSeen[key] = true;
        }
        const flagIds = Object.keys(flagSeen).sort(function(a, b) {
          return parseInt(a, 10) - parseInt(b, 10);
        });
        const flagRecs = [];
        for (let j = 0; j < flagIds.length; j++) {
          const fid = parseInt(flagIds[j], 10);
          const byte = packFlags(headerless, borderless, fid);
          if (byte === 0) {
            continue;
          }
          flagRecs.push(idToken(fid) + flagToken(byte & 63) + flagToken(byte >> 6 & 63));
        }
        if (colorRecs.length === 0 && flagRecs.length === 0) {
          return "0";
        }
        return FORMAT_MARKER + "3" + colorRecs.join("") + "|" + flagRecs.join("");
      }
      function parseAppearance(text) {
        const colors = {};
        const headerless = {};
        const borderless = {};
        const result = { colors, headerless, borderless };
        if (!text || text === "0") {
          return result;
        }
        if (text.charAt(0) === FORMAT_MARKER && text.charAt(1) === "3") {
          const sparse = text.substring(2);
          const bar = sparse.indexOf("|");
          const colorPart = bar < 0 ? sparse : sparse.substring(0, bar);
          const flagPart = bar < 0 ? "" : sparse.substring(bar + 1);
          let o;
          for (o = 0; o + 5 <= colorPart.length; o += 5) {
            const cid = COLOR_ALPHABET.indexOf(colorPart.charAt(o));
            if (cid > 0) {
              readColours(colors, colorPart, cid, o + 1);
            }
          }
          for (o = 0; o + 3 <= flagPart.length; o += 3) {
            const fid = COLOR_ALPHABET.indexOf(flagPart.charAt(o));
            if (fid > 0) {
              const fbyte = flagValue(flagPart.charAt(o + 1)) | flagValue(flagPart.charAt(o + 2)) << 6;
              unpackFlags(headerless, borderless, fid, fbyte);
            }
          }
          return result;
        }
        if (text.charAt(0) === FORMAT_MARKER) {
          const body = text.substring(1);
          for (let offset = 0; offset + 6 <= body.length; offset += 6) {
            const id = offset / 6;
            if (id === 0) {
              continue;
            }
            readColours(colors, body, id, offset);
            const byte = flagValue(body.charAt(offset + 4)) | flagValue(body.charAt(offset + 5)) << 6;
            unpackFlags(headerless, borderless, id, byte);
          }
          return result;
        }
        for (let oldOffset = 0; oldOffset + 5 <= text.length; oldOffset += 5) {
          const oldId = oldOffset / 5;
          if (oldId === 0) {
            continue;
          }
          readColours(colors, text, oldId, oldOffset);
          const flag = text.charAt(oldOffset + 4);
          const hideHeader = flag === "H" || flag === "X";
          const hideBorder = flag === "B" || flag === "X";
          for (let s = 0; s < SIZE_ORDER.length; s++) {
            if (hideHeader) {
              setFlag(headerless, oldId, SIZE_ORDER[s], true);
            }
            if (hideBorder) {
              setFlag(borderless, oldId, SIZE_ORDER[s], true);
            }
          }
        }
        return result;
      }
      var COLOR_ALPHABET, FORMAT_MARKER, SIZE_ORDER;
      var init_codec = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/codec.ts"() {
          COLOR_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
          FORMAT_MARKER = "~";
          SIZE_ORDER = ["1x2", "2x2", "1x4", "2x4"];
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/model.ts
      var model_exports = {};
      __export(model_exports, {
        allSizesHidden: () => allSizesHidden,
        buildOptionByLabel: () => buildOptionByLabel,
        buildThemeModules: () => buildThemeModules,
        sizeRowsFor: () => sizeRowsFor
      });
      function buildOptionByLabel(rawModules) {
        const optionByLabel = {};
        for (let i = 0; i < rawModules.length; i++) {
          if (rawModules[i]) {
            optionByLabel[rawModules[i].label] = rawModules[i];
          }
        }
        return optionByLabel;
      }
      function sizeRowsFor(option, optionByLabel) {
        if (option.themeRows) {
          return option.themeRows.map(function(row) {
            const src = optionByLabel[row.thumb || ""] || option;
            const panelHeaderless = row.alwaysHeaderless != null ? row.alwaysHeaderless : src.alwaysHeaderless || option.alwaysHeaderless;
            return {
              size: row.size,
              thumbLabel: row.thumb || option.label,
              value: src.value,
              alwaysHeaderless: Boolean(panelHeaderless)
            };
          });
        }
        return (option.sizes || []).map(function(size) {
          return {
            size,
            thumbLabel: option.label,
            value: option.value,
            alwaysHeaderless: Boolean(option.alwaysHeaderless)
          };
        });
      }
      function buildThemeModules(rawModules) {
        const optionByLabel = buildOptionByLabel(rawModules);
        const modules = [];
        for (let i = 0; i < rawModules.length; i++) {
          const option = rawModules[i];
          if (!option || option.value === 0 || option.themeHidden) {
            continue;
          }
          const sizeRows = sizeRowsFor(option, optionByLabel);
          modules.push({
            value: option.value,
            label: option.themeLabel || option.label,
            // themeLabel renames the row but thumbnails are keyed by the real label so keep it
            // for the lookup. the forecast row survives on the hourly panel (4-day is themeHidden)
            // so it shows the hourly shot
            thumbLabel: option.label,
            icon: option.icon || "\xB7",
            vibrant: option.vibrant,
            sizeRows,
            alwaysHeaderless: sizeRows.length > 0 && sizeRows.every(function(row) {
              return row.alwaysHeaderless;
            })
          });
        }
        return modules;
      }
      function allSizesHidden(module, map) {
        if (!module.sizeRows || module.sizeRows.length === 0) {
          return false;
        }
        for (let s = 0; s < module.sizeRows.length; s++) {
          const row = module.sizeRows[s];
          if (!flagOn(map, row.value, row.size)) {
            return false;
          }
        }
        return true;
      }
      var init_model = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/model.ts"() {
          init_codec();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/preview.ts
      var preview_exports = {};
      __export(preview_exports, {
        buildExampleBox: () => buildExampleBox,
        channelCss: () => channelCss,
        contrastText: () => contrastText,
        paintSwatch: () => paintSwatch
      });
      function paintSwatch(swatch, byte) {
        if (byte == null) {
          swatch.style.background = "repeating-linear-gradient(45deg, #fff, #fff 4px, #ddd 4px, #ddd 8px)";
        } else {
          swatch.style.background = argbToCss(byte);
        }
      }
      function channelCss(channelKey, byte) {
        if (byte != null) {
          return argbToCss(byte);
        }
        return channelKey === "subtitle" ? "#AAAAAA" : "#FFFFFF";
      }
      function contrastText(byte) {
        if (byte == null) {
          return "#000000";
        }
        const red = (byte >> 4 & 3) * 85;
        const green = (byte >> 2 & 3) * 85;
        const blue = (byte & 3) * 85;
        const luma = (red * 299 + green * 587 + blue * 114) / 1e3;
        return luma > 140 ? "#000000" : "#FFFFFF";
      }
      function buildExampleBox(module, stagedColors, hideHeader, hideBorder) {
        const box = document.createElement("div");
        box.className = "tb-ex";
        const header = document.createElement("div");
        header.className = "tb-ex-hdr";
        header.textContent = (module.label || "Module").toUpperCase();
        const value = document.createElement("div");
        value.className = "tb-ex-val";
        value.textContent = "8,423";
        const subtitle = document.createElement("div");
        subtitle.className = "tb-ex-sub";
        subtitle.textContent = "OF 10,000";
        const icon = document.createElement("div");
        icon.className = "tb-ex-icon";
        icon.textContent = "\u25CF";
        if (!hideHeader) {
          box.appendChild(header);
        }
        box.appendChild(value);
        box.appendChild(subtitle);
        box.appendChild(icon);
        function paintChannel(key, byte) {
          if (key === "accent") {
            const accentCss = channelCss("accent", byte);
            if (!hideHeader) {
              header.style.background = accentCss;
              header.style.color = contrastText(byte);
            }
            box.style.border = "1px solid " + (hideBorder ? "transparent" : accentCss);
          } else if (key === "value") {
            value.style.color = channelCss("value", byte);
          } else if (key === "icon") {
            icon.style.color = channelCss("icon", byte);
          } else if (key === "subtitle") {
            subtitle.style.color = channelCss("subtitle", byte);
          }
        }
        ["accent", "value", "icon", "subtitle"].forEach(function(key) {
          paintChannel(key, stagedColors[key]);
        });
        return { box, paint: paintChannel };
      }
      var init_preview = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/preview.ts"() {
          init_palette();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/picker.ts
      var picker_exports = {};
      __export(picker_exports, {
        createPicker: () => createPicker
      });
      function createPicker(env) {
        const host = createOverlayHost("tb-pick-overlay", "tb-pick", true);
        function open(module, channelKey, swatchEls) {
          const panel = host.open();
          const moduleValue = module.value;
          const committed = env.getColors()[moduleValue] || {};
          const stagedColors = {
            accent: committed.accent != null ? committed.accent : null,
            value: committed.value != null ? committed.value : null,
            icon: committed.icon != null ? committed.icon : null,
            subtitle: committed.subtitle != null ? committed.subtitle : null
          };
          let active = channelKey;
          function labelFor(key) {
            for (let i = 0; i < env.channels.length; i++) {
              if (env.channels[i].key === key) {
                return env.channels[i].label;
              }
            }
            return key;
          }
          const nav = document.createElement("div");
          nav.className = "tb-pick-nav";
          const prevButton = document.createElement("button");
          prevButton.type = "button";
          prevButton.className = "tb-nav-btn";
          prevButton.textContent = "\u2039";
          const titleLabel = document.createElement("div");
          titleLabel.className = "tb-pick-title";
          const nextButton = document.createElement("button");
          nextButton.type = "button";
          nextButton.className = "tb-nav-btn";
          nextButton.textContent = "\u203A";
          nav.appendChild(prevButton);
          nav.appendChild(titleLabel);
          nav.appendChild(nextButton);
          panel.appendChild(nav);
          const cells = [];
          let monoOption = null;
          let vibrantOption = null;
          const vibrantByte = module.vibrant != null ? env.argbByName[module.vibrant] : null;
          const hideHeader = module.alwaysHeaderless || allSizesHidden(module, env.getHeaderless());
          const hideBorder = allSizesHidden(module, env.getBorderless());
          const example = buildExampleBox(module, stagedColors, hideHeader, hideBorder);
          panel.appendChild(example.box);
          function refreshHighlight() {
            const byte = stagedColors[active];
            if (monoOption) {
              monoOption.classList.toggle("sel", byte == null);
            }
            if (vibrantOption) {
              vibrantOption.classList.toggle("sel", vibrantByte != null && byte === vibrantByte);
            }
            for (let k = 0; k < cells.length; k++) {
              cells[k].el.classList.toggle("sel", byte != null && cells[k].argb === byte);
            }
          }
          function setActive(key) {
            active = key;
            titleLabel.textContent = "Editing: " + labelFor(key);
            refreshHighlight();
          }
          function step(delta) {
            const order = env.channels.map(function(channel) {
              return channel.key;
            });
            const at = order.indexOf(active);
            setActive(order[(at + delta + order.length) % order.length]);
          }
          prevButton.addEventListener("click", function() {
            step(-1);
          });
          nextButton.addEventListener("click", function() {
            step(1);
          });
          function stage(byte) {
            stagedColors[active] = byte;
            example.paint(active, byte);
            refreshHighlight();
          }
          const mvRow = document.createElement("div");
          mvRow.className = "tb-mv";
          monoOption = document.createElement("div");
          monoOption.className = "tb-mv-opt";
          monoOption.innerHTML = '<span class="tb-mv-ind tb-cell-mono"></span>Mono';
          monoOption.addEventListener("click", function() {
            stage(null);
          });
          mvRow.appendChild(monoOption);
          if (vibrantByte != null) {
            vibrantOption = document.createElement("div");
            vibrantOption.className = "tb-mv-opt";
            const vibrantDot = document.createElement("span");
            vibrantDot.className = "tb-mv-ind";
            vibrantDot.style.background = argbToCss(vibrantByte);
            vibrantOption.appendChild(vibrantDot);
            vibrantOption.appendChild(document.createTextNode("Vibrant"));
            vibrantOption.addEventListener("click", function() {
              stage(vibrantByte);
            });
            mvRow.appendChild(vibrantOption);
          }
          panel.appendChild(mvRow);
          const grid = document.createElement("div");
          grid.className = "tb-grid";
          for (let index = 0; index < env.palette.length; index++) {
            (function(entry) {
              const cell = document.createElement("div");
              cell.className = "tb-cell";
              cell.style.background = entry.css;
              cell.title = entry.name;
              cell.addEventListener("click", function() {
                stage(entry.argb);
              });
              grid.appendChild(cell);
              cells.push({ el: cell, argb: entry.argb });
            })(env.palette[index]);
          }
          panel.appendChild(grid);
          const bar = document.createElement("div");
          bar.className = "tb-pick-bar";
          const cancelButton = document.createElement("button");
          cancelButton.type = "button";
          cancelButton.className = "tb-io-btn";
          cancelButton.textContent = "Cancel";
          cancelButton.addEventListener("click", host.close);
          bar.appendChild(cancelButton);
          const applyButton = document.createElement("button");
          applyButton.type = "button";
          applyButton.className = "tb-io-btn primary";
          applyButton.textContent = "Apply";
          applyButton.addEventListener("click", function() {
            env.channels.forEach(function(channel) {
              env.setChannel(moduleValue, channel.key, stagedColors[channel.key]);
              paintSwatch(swatchEls[channel.key], stagedColors[channel.key]);
            });
            host.close();
          });
          bar.appendChild(applyButton);
          panel.appendChild(bar);
          setActive(active);
        }
        return { open, close: host.close };
      }
      var init_picker = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/picker.ts"() {
          init_overlay();
          init_palette();
          init_model();
          init_preview();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/rows.ts
      var rows_exports = {};
      __export(rows_exports, {
        createRowBuilder: () => createRowBuilder
      });
      function createRowBuilder(env) {
        function buildFlagToggle(getMap, flagId, size, letter) {
          const toggle = document.createElement("div");
          toggle.className = "tb-toggle";
          toggle.title = (letter === "H" ? "Header" : "Border") + " " + size;
          const paint = function() {
            const shown = !flagOn(getMap(), flagId, size);
            toggle.textContent = letter;
            toggle.style.background = shown ? "#7bd88f" : "#23262c";
            toggle.style.color = shown ? "#08210f" : "#666c76";
          };
          paint();
          toggle.addEventListener("click", function() {
            setFlag(getMap(), flagId, size, !flagOn(getMap(), flagId, size));
            paint();
            env.persist();
          });
          return toggle;
        }
        function buildSizeRow(row) {
          const size = row.size;
          const sizeRow = document.createElement("div");
          sizeRow.className = "sz";
          const shot = document.createElement("div");
          shot.className = "sz-shot";
          const src = thumbByLabel(env.thumbs, row.thumbLabel, size);
          if (src) {
            const shotImg = document.createElement("img");
            shotImg.src = src;
            shotImg.alt = row.thumbLabel + " " + size;
            shot.appendChild(shotImg);
          } else {
            const noshot = document.createElement("div");
            noshot.className = "sz-noshot";
            noshot.textContent = "no shot";
            shot.appendChild(noshot);
          }
          sizeRow.appendChild(shot);
          const label = document.createElement("div");
          label.className = "sz-lbl";
          label.textContent = size;
          sizeRow.appendChild(label);
          const toggles = document.createElement("div");
          toggles.className = "sz-tog";
          if (!row.alwaysHeaderless) {
            toggles.appendChild(buildFlagToggle(env.getHeaderless, row.value, size, "H"));
          }
          toggles.appendChild(buildFlagToggle(env.getBorderless, row.value, size, "B"));
          sizeRow.appendChild(toggles);
          return sizeRow;
        }
        function buildRow(module) {
          const row = document.createElement("div");
          row.className = "tb-row";
          const head = document.createElement("div");
          head.className = "tb-row-head";
          const icon = document.createElement("div");
          icon.className = "tb-row-ic";
          icon.textContent = module.icon;
          head.appendChild(icon);
          const name = document.createElement("div");
          name.className = "tb-row-nm";
          name.textContent = module.label;
          head.appendChild(name);
          const cols = document.createElement("div");
          cols.className = "tb-chips";
          const swatchEls = {};
          for (let index = 0; index < env.channels.length; index++) {
            (function(channel) {
              const chip = document.createElement("div");
              chip.className = "tb-chip";
              const label = document.createElement("div");
              label.className = "tb-chip-lbl";
              label.textContent = channel.label;
              chip.appendChild(label);
              const swatch = document.createElement("div");
              swatch.className = "tb-swatch";
              const stored = env.getColors()[module.value] ? env.getColors()[module.value][channel.key] : null;
              paintSwatch(swatch, stored);
              swatch.addEventListener("click", function() {
                env.openPicker(module, channel.key, swatchEls);
              });
              chip.appendChild(swatch);
              swatchEls[channel.key] = swatch;
              cols.appendChild(chip);
            })(env.channels[index]);
          }
          head.appendChild(cols);
          row.appendChild(head);
          const sizeList = document.createElement("div");
          sizeList.className = "sz-list";
          for (let sizeIndex = 0; sizeIndex < module.sizeRows.length; sizeIndex++) {
            sizeList.appendChild(buildSizeRow(module.sizeRows[sizeIndex]));
          }
          row.appendChild(sizeList);
          return row;
        }
        return { buildRow };
      }
      var init_rows = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/rows.ts"() {
          init_thumbs();
          init_codec();
          init_preview();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/sheet.ts
      var sheet_exports = {};
      __export(sheet_exports, {
        createSheet: () => createSheet
      });
      function createSheet(env) {
        const sheetHost = createOverlayHost("tb-overlay", "tb-sheet", false);
        const ioHost = createOverlayHost("tb-pick-overlay", "tb-pick", true);
        let editorList = null;
        function renderList() {
          if (!editorList) {
            return;
          }
          while (editorList.firstChild) {
            editorList.removeChild(editorList.firstChild);
          }
          for (let index = 0; index < env.modules.length; index++) {
            editorList.appendChild(env.buildRow(env.modules[index]));
          }
        }
        function openIO() {
          const panel = ioHost.open();
          buildIoPanel(panel, {
            title: "Import / Export Colours",
            css: { title: "tb-pick-title", textarea: "tb-io-textarea", buttons: "tb-io-btns", button: "tb-io-btn" },
            value: env.serialize(),
            copyResetMs: 1500,
            onApply: function(text) {
              env.applyImport(text);
              renderList();
              ioHost.close();
            }
          });
        }
        function closeSheet() {
          env.picker.close();
          ioHost.close();
          sheetHost.close();
          editorList = null;
          document.documentElement.style.overflow = "";
          env.onDone();
        }
        function buildBulkBar() {
          const bar = document.createElement("div");
          bar.className = "tb-bar";
          function mapFor(which) {
            return which === "header" ? env.getHeaderless() : env.getBorderless();
          }
          function anyStillOn(which) {
            const map = mapFor(which);
            for (let index = 0; index < env.modules.length; index++) {
              const rows = env.modules[index].sizeRows;
              for (let sizeIndex = 0; sizeIndex < rows.length; sizeIndex++) {
                const subRow = rows[sizeIndex];
                if (which === "header" && subRow.alwaysHeaderless) {
                  continue;
                }
                if (!flagOn(map, subRow.value, subRow.size)) {
                  return true;
                }
              }
            }
            return false;
          }
          function setAll(which, on) {
            const map = mapFor(which);
            for (let index = 0; index < env.modules.length; index++) {
              const rows = env.modules[index].sizeRows;
              for (let sizeIndex = 0; sizeIndex < rows.length; sizeIndex++) {
                const subRow = rows[sizeIndex];
                if (which === "header" && subRow.alwaysHeaderless) {
                  continue;
                }
                setFlag(map, subRow.value, subRow.size, on);
              }
            }
          }
          function makeBulkButton(label, which) {
            const button = document.createElement("button");
            button.type = "button";
            button.className = "tb-bar-btn ghost";
            const paint = function() {
              button.textContent = anyStillOn(which) ? "All " + label + " Off" : "All " + label + " On";
            };
            paint();
            button.addEventListener("click", function() {
              setAll(which, anyStillOn(which));
              env.persist();
              paint();
              renderList();
            });
            return button;
          }
          bar.appendChild(makeBulkButton("borders", "border"));
          bar.appendChild(makeBulkButton("headers", "header"));
          return bar;
        }
        function buildBar(isBottom) {
          const bar = document.createElement("div");
          bar.className = "tb-bar" + (isBottom ? " tb-bar-bottom" : "");
          const resetButton = document.createElement("button");
          resetButton.type = "button";
          resetButton.className = "tb-bar-btn ghost";
          resetButton.textContent = "Reset";
          resetButton.addEventListener("click", function() {
            env.resetAll();
            env.persist();
            renderList();
          });
          bar.appendChild(resetButton);
          const ioButton = document.createElement("button");
          ioButton.type = "button";
          ioButton.className = "tb-bar-btn io";
          ioButton.textContent = "Import/Export";
          ioButton.addEventListener("click", openIO);
          bar.appendChild(ioButton);
          const doneButton = document.createElement("button");
          doneButton.type = "button";
          doneButton.className = "tb-bar-btn";
          doneButton.textContent = "Done";
          doneButton.addEventListener("click", closeSheet);
          bar.appendChild(doneButton);
          return bar;
        }
        function open() {
          const panel = sheetHost.open();
          const legend = document.createElement("div");
          legend.className = "tb-legend";
          legend.innerHTML = '<div class="tb-legend-ttl">What each control changes</div><div class="tb-legend-grid"><div class="tb-lg-term"><span class="tb-lg-dot empty"></span>H / B</div><div class="tb-lg-desc">show or hide the header strip / the panel outline, per size (green shown, grey hidden)</div><div class="tb-lg-term"><span class="tb-lg-dot"></span>Accent</div><div class="tb-lg-desc">the header bar, border &amp; progress (shared across every size)</div><div class="tb-lg-term"><span class="tb-lg-dot"></span>Value</div><div class="tb-lg-desc">the big number or readout</div><div class="tb-lg-term"><span class="tb-lg-dot"></span>Icon</div><div class="tb-lg-desc">the icon</div><div class="tb-lg-term"><span class="tb-lg-dot"></span>Sub</div><div class="tb-lg-desc">the small caption line (e.g. OF 10000 STEPS)</div></div>';
          editorList = document.createElement("div");
          editorList.className = "tb-list";
          renderList();
          const scroller = document.createElement("div");
          scroller.className = "tb-scroll";
          scroller.appendChild(legend);
          scroller.appendChild(buildBulkBar());
          scroller.appendChild(editorList);
          panel.appendChild(scroller);
          panel.appendChild(buildBar(true));
          document.documentElement.style.overflow = "hidden";
        }
        return { open };
      }
      var init_sheet = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/sheet.ts"() {
          init_overlay();
          init_io_panel();
          init_codec();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/init.ts
      var init_exports = {};
      __export(init_exports, {
        init: () => init
      });
      function init() {
        const self = this;
        const root = self.$element[0];
        const config = self.config || {};
        const rawModules = config.moduleOptions || [];
        const THUMBS = config.moduleThumbnails || {};
        const hidden = root.querySelector(".tb-value");
        const editButton = root.querySelector(".tb-edit-btn");
        const modules = buildThemeModules(rawModules);
        const palette = buildPalette(PEBBLE_COLORS_CSV);
        const argbByName = buildArgbByName(palette);
        const channels = [
          { key: "accent", label: "Accent" },
          { key: "value", label: "Value" },
          { key: "icon", label: "Icon" },
          { key: "subtitle", label: "Sub" }
        ];
        let colors = {};
        let headerless = {};
        let borderless = {};
        function persist() {
          hidden.value = serializeAppearance(colors, headerless, borderless);
        }
        function setChannel(moduleValue, channelKey, byte) {
          if (!colors[moduleValue]) {
            colors[moduleValue] = { accent: null, value: null, icon: null, subtitle: null };
          }
          colors[moduleValue][channelKey] = byte;
          persist();
        }
        const picker = createPicker({
          channels,
          palette,
          argbByName,
          getColors: function() {
            return colors;
          },
          getHeaderless: function() {
            return headerless;
          },
          getBorderless: function() {
            return borderless;
          },
          setChannel
        });
        const rowBuilder = createRowBuilder({
          thumbs: THUMBS,
          channels,
          getColors: function() {
            return colors;
          },
          getHeaderless: function() {
            return headerless;
          },
          getBorderless: function() {
            return borderless;
          },
          persist,
          openPicker: picker.open
        });
        const sheet = createSheet({
          modules,
          buildRow: rowBuilder.buildRow,
          getHeaderless: function() {
            return headerless;
          },
          getBorderless: function() {
            return borderless;
          },
          serialize: function() {
            return serializeAppearance(colors, headerless, borderless);
          },
          applyImport: function(text) {
            const parsed = parseAppearance(text);
            colors = parsed.colors;
            headerless = parsed.headerless;
            borderless = parsed.borderless;
            persist();
          },
          resetAll: function() {
            colors = {};
            headerless = {};
            borderless = {};
          },
          persist,
          picker,
          onDone: function() {
            persist();
            self.trigger("change");
          }
        });
        editButton.addEventListener("click", sheet.open);
        root._tbSet = function(value) {
          const parsed = parseAppearance(value);
          colors = parsed.colors;
          headerless = parsed.headerless;
          borderless = parsed.borderless;
          persist();
        };
        root._tbGet = function() {
          return serializeAppearance(colors, headerless, borderless);
        };
        root._tbSet(hidden.value || "");
      }
      var init_init = __esm({
        "watchfaces/mosaic/core/pkjs/clay/builder/ts/theme/init.ts"() {
          init_palette();
          init_codec();
          init_model();
          init_picker();
          init_rows();
          init_sheet();
        }
      });

      // watchfaces/mosaic/core/pkjs/clay/builder/component-entry.js
      var require_component_entry = __commonJS({
        "watchfaces/mosaic/core/pkjs/clay/builder/component-entry.js"(exports, module) {
          init_thumbs();
          init_overlay();
          init_io_panel();
          init_palette();
          init_codec();
          init_model();
          init_preview();
          init_picker();
          init_rows();
          init_sheet();
          init_init();
          module.exports = (init_init(), __toCommonJS(init_exports));
        }
      });
      return require_component_entry();
    })();

    __clayComponent.init.call(this);
  }
};
