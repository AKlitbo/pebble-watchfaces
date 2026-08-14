// generated from watchfaces/lcars-stardate/src/pkjs/clay/builder/slots.manifest.ts by tools/clay-components/generate-components.ts
// do not edit by hand: run `npm run gen:clay` after changing the sources
/**
 * Clay custom component for the drag and drop ops slot builder.
 *
 * Shows a sketch of the watch face with its four panels as drop targets, and a
 * palette of every readout below it. Drag a readout onto a panel, or drag one
 * panel onto another to swap them.
 *
 * The value it round trips is the upper-left panel only, which is this
 * component's own message key. The other three ride hidden stores on the same
 * page, because a Clay component owns exactly one key.
 *
 * IMPORTANT: initialize and the manipulator run inside the Clay config webview,
 * a separate JS context. They must be self contained, which is why the generator
 * inlines every piece below into one initialize.
 */
module.exports = {
  name: "slotBuilder",

  template: "<div class=\"component sb\">  <div class=\"sb-header\">    <p class=\"sb-desc\">Drag a readout onto a panel.<br>Drag one panel onto another to swap them.</p>  </div>  <div class=\"sb-face-wrap\">    <div class=\"sb-face\">      <div class=\"sb-rail\">        <div class=\"sb-rail-block sb-rail-1\"></div>        <div class=\"sb-rail-block sb-rail-2\"></div>      </div>      <div class=\"sb-slots\">        <div class=\"sb-slot\" data-slot=\"0\"></div>        <div class=\"sb-slot\" data-slot=\"2\"></div>        <div class=\"sb-slot\" data-slot=\"1\"></div>        <div class=\"sb-slot\" data-slot=\"3\"></div>      </div>    </div>    <div class=\"sb-actions\">      <button type=\"button\" class=\"sb-btn sb-preset\" data-preset=\"default\">Default</button>      <button type=\"button\" class=\"sb-btn sb-btn-clear\">Clear</button>    </div>  </div>  <div class=\"sb-palette\"></div>  <input type=\"hidden\" class=\"sb-value\"></div>",

  style: "@font-face{font-family:'LcarsAntonio';font-style:normal;font-weight:700;font-display:swap;src:url(data:font/woff2;base64,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) format('woff2')}.sb{--sb-slot-w:100px;--sb-slot-h:44px;--sb-gap:4px;--sb-label:#ffaa00}.sb-header{margin:0 0 10px}.sb-desc{margin:0;font-size:13px;line-height:1.4;opacity:0.75}.sb-face-wrap{display:flex;justify-content:center;gap:8px;margin-bottom:12px}.sb-face{display:flex;gap:var(--sb-gap);padding:6px;background:#000;border-radius:6px;user-select:none;-webkit-user-select:none;touch-action:none;flex:0 1 auto;min-width:0;max-width:calc(var(--sb-slot-w) * 2 + var(--sb-gap) * 2 + 32px)}.sb-rail{display:flex;flex-direction:column;gap:2px;width:20px}.sb-rail-block{flex:1;border-radius:2px}.sb-rail-1{background:#ff5500}.sb-rail-2{background:#ffaa55}.sb-slots{display:grid;grid-template-columns:minmax(0,var(--sb-slot-w)) minmax(0,var(--sb-slot-w));grid-template-rows:var(--sb-slot-h) var(--sb-slot-h);gap:var(--sb-gap);min-width:0;flex:0 1 auto}.sb-slot{position:relative;background:#111;border:1px dashed #444;border-radius:3px;overflow:hidden;cursor:grab;display:flex;align-items:center;justify-content:center}.sb-slot[data-slot=\"0\"]{grid-area:1 / 1}.sb-slot[data-slot=\"1\"]{grid-area:2 / 1}.sb-slot[data-slot=\"2\"]{grid-area:1 / 2}.sb-slot[data-slot=\"3\"]{grid-area:2 / 2}.sb-slots.tall .sb-slot[data-slot=\"0\"]{grid-area:1 / 1 / span 2 / span 1}.sb-slots.tall .sb-slot[data-slot=\"1\"]{display:none}.sb-slot.filled{border-style:solid;border-color:#222}.sb-slot.over{border-color:var(--sb-label);border-style:solid;box-shadow:0 0 0 2px rgba(255,170,0,0.35)}.sb-slot.blocked{opacity:0.35}.sb-slot-img{width:100%;height:100%;object-fit:contain;pointer-events:none;image-rendering:pixelated}.sb-slot-empty{color:#555;font-size:11px;letter-spacing:0.5px;pointer-events:none}.sb-actions{display:flex;flex-direction:column;justify-content:flex-start;gap:5px;width:84px;box-sizing:border-box;padding-left:8px;border-left:1px solid #444}.sb-btn{width:100%;min-width:0;box-sizing:border-box;margin:0;padding:5px 4px;font-size:12px;line-height:1.2;border:1px solid #666;border-radius:9px;background:transparent;color:inherit;cursor:pointer}.sb-preset{border-color:#aa55ff;color:#ccaaff}.sb-btn-clear{border-color:#aa5555;color:#ffaaaa}.sb-palette{display:grid;grid-template-columns:repeat(4,1fr);grid-auto-rows:34px;gap:4px}.sb-pal.tall{grid-row:span 2;height:72px}.sb-pal{position:relative;height:34px;border-radius:3px;overflow:hidden;background:#000;cursor:grab;display:flex;align-items:center;justify-content:center;touch-action:none}.sb-pal-img{max-width:100%;max-height:100%;pointer-events:none;image-rendering:pixelated}.sb-pal-fallback{flex-direction:column;gap:2px;color:#fff;font-size:10px;text-align:center;line-height:1.1;padding:2px}.sb-pal-icon{font-size:16px}.sb-ghost{position:fixed;z-index:9999;width:var(--sb-slot-w);height:var(--sb-slot-h);pointer-events:none;opacity:0.85;border-radius:3px;overflow:hidden;background:#000;box-shadow:0 2px 8px rgba(0,0,0,0.5)}.sb-ghost.tall{height:calc(var(--sb-slot-h) * 2 + var(--sb-gap))}.sb-ghost img{width:100%;height:100%;object-fit:contain;image-rendering:pixelated}body{background:#000;color:#fff}body .section{position:relative;background:#0a0a0a;border:none;border-left:10px solid #aa55ff;border-radius:0 10px 10px 0;margin:0 6px 14px;padding:10px 12px 12px}body .section:nth-of-type(2){border-left-color:#ffaa55}body .section:nth-of-type(3){border-left-color:#55aaff}body .section:nth-of-type(4){border-left-color:#ff5500}body .section:nth-of-type(5){border-left-color:#aaaaff}body .section:nth-of-type(6){border-left-color:#aa5555}body .component.component-heading,body .section>.component.component-heading{position:relative;background:transparent;border:none;padding:0;margin:0 0 10px;overflow:visible}body .component.component-heading::after,body .section>.component.component-heading::after{content:'';display:block;position:absolute;left:0;right:0;top:7px;bottom:auto;height:11px;background:#aaaaff;border-radius:6px;z-index:0}body .inputs>.component.component-heading{margin-top:14px}body .inputs>.component.component-text{margin-bottom:16px}body .component.component-heading h4{position:relative;display:inline-block;margin:0;padding:0 8px 0 0;background:#0a0a0a;color:#ffaa00;font-family:'LcarsAntonio','Arial Narrow',sans-serif;font-size:21px;font-weight:700;letter-spacing:1.5px;line-height:25px;text-transform:uppercase;z-index:1}body .component.component-text p{color:#c8c8d8;font-size:13px;line-height:1.45}body .component .label{color:#ffaa55;font-family:'LcarsAntonio','Arial Narrow',sans-serif;font-size:16px;letter-spacing:0.8px;text-transform:uppercase}body .component .description{color:#8a8a9a;font-size:12px;line-height:1.4;text-transform:none}body .component select,body .component input[type=\"text\"],body .component input[type=\"number\"]{background:#16161c;color:#ffcc88;border:1px solid #3a3a44;border-radius:4px;font-family:'LcarsAntonio','Arial Narrow',sans-serif;font-size:16px;letter-spacing:0.5px}body .component-submit button,body button[type=\"submit\"]{background:#ff5500;color:#000;border:none;border-radius:18px;padding:11px 24px;font-family:'LcarsAntonio','Arial Narrow',sans-serif;font-size:19px;font-weight:700;letter-spacing:1.4px;text-transform:uppercase}body .sb-btn{font-family:'LcarsAntonio','Arial Narrow',sans-serif;font-size:13px;letter-spacing:1px;text-transform:uppercase}",

  manipulator: {
    set: function (value) {
      this.$element[0]._sbSet(value || '');
    },
    get: function () {
      return this.$element[0]._sbGet();
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

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/geometry.ts
      var geometry_exports = {};
      __export(geometry_exports, {
        DEFAULT_SLOTS: () => DEFAULT_SLOTS,
        ID_EMPTY: () => ID_EMPTY,
        ID_HEART: () => ID_HEART,
        ID_SENSORS: () => ID_SENSORS,
        ID_STEPS: () => ID_STEPS,
        SLOT_COUNT: () => SLOT_COUNT,
        SLOT_KEYS: () => SLOT_KEYS,
        SLOT_LB: () => SLOT_LB,
        SLOT_LT: () => SLOT_LT,
        SLOT_RB: () => SLOT_RB,
        SLOT_RT: () => SLOT_RT,
        SLOT_STORE_CLASS: () => SLOT_STORE_CLASS,
        canPlace: () => canPlace,
        isBottom: () => isBottom,
        isTall: () => isTall,
        swallowedBy: () => swallowedBy
      });
      function isBottom(slot) {
        return slot === SLOT_LB || slot === SLOT_RB;
      }
      function isTall(id) {
        return id === ID_SENSORS;
      }
      function canPlace(id, slot) {
        return !isTall(id) || slot === SLOT_LT;
      }
      function swallowedBy(id, slot) {
        return isTall(id) && slot === SLOT_LT ? SLOT_LB : -1;
      }
      var SLOT_LT, SLOT_LB, SLOT_RT, SLOT_RB, SLOT_COUNT, ID_HEART, ID_STEPS, ID_EMPTY, ID_SENSORS, DEFAULT_SLOTS, SLOT_KEYS, SLOT_STORE_CLASS;
      var init_geometry = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/geometry.ts"() {
          SLOT_LT = 0;
          SLOT_LB = 1;
          SLOT_RT = 2;
          SLOT_RB = 3;
          SLOT_COUNT = 4;
          ID_HEART = 0;
          ID_STEPS = 1;
          ID_EMPTY = 20;
          ID_SENSORS = 23;
          DEFAULT_SLOTS = [ID_SENSORS, ID_EMPTY, ID_HEART, ID_STEPS];
          SLOT_KEYS = [
            "APPEARANCE_SLOT_LEFT_TOP",
            "APPEARANCE_SLOT_LEFT_BOTTOM",
            "APPEARANCE_SLOT_RIGHT_TOP",
            "APPEARANCE_SLOT_RIGHT_BOTTOM"
          ];
          SLOT_STORE_CLASS = [null, "sb-lb", "sb-rt", "sb-rb"];
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/codec.ts
      var codec_exports = {};
      __export(codec_exports, {
        defaults: () => defaults,
        formatId: () => formatId,
        parseId: () => parseId,
        readSlots: () => readSlots,
        sanitize: () => sanitize,
        storeFor: () => storeFor,
        writeStores: () => writeStores
      });
      function parseId(text) {
        const n = parseInt(text || "", 10);
        return isNaN(n) || n < 0 ? ID_EMPTY : n;
      }
      function formatId(id) {
        return String(id);
      }
      function storeFor(root, slot) {
        const cls = SLOT_STORE_CLASS[slot];
        if (!cls) {
          return null;
        }
        const doc = root.ownerDocument || document;
        return doc.querySelector("." + cls);
      }
      function readSlots(root, ownValue) {
        const slots = [];
        for (let slot = 0; slot < SLOT_COUNT; slot++) {
          const store = storeFor(root, slot);
          slots.push(parseId(store ? store.value : ownValue));
        }
        return sanitize(slots);
      }
      function writeStores(root, slots) {
        for (let slot = 1; slot < SLOT_COUNT; slot++) {
          const store = storeFor(root, slot);
          const next = formatId(slots[slot]);
          if (store && store.value !== next) {
            store.value = next;
            store.dispatchEvent(new Event("change"));
          }
        }
      }
      function sanitize(slots) {
        const out = slots.slice(0, SLOT_COUNT);
        while (out.length < SLOT_COUNT) {
          out.push(ID_EMPTY);
        }
        for (let slot = 0; slot < SLOT_COUNT; slot++) {
          if (!canPlace(out[slot], slot)) {
            out[slot] = ID_EMPTY;
          }
        }
        return out;
      }
      function defaults() {
        return DEFAULT_SLOTS.slice();
      }
      var init_codec = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/codec.ts"() {
          init_geometry();
        }
      });

      // watchfaces/lcars-stardate/src/data/slot-presets.json
      var slot_presets_default;
      var init_slot_presets = __esm({
        "watchfaces/lcars-stardate/src/data/slot-presets.json"() {
          slot_presets_default = {
            default: [23, 20, 0, 1]
          };
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/presets.ts
      var presets_exports = {};
      __export(presets_exports, {
        SLOT_PRESETS: () => SLOT_PRESETS
      });
      var SLOT_PRESETS;
      var init_presets = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/presets.ts"() {
          init_slot_presets();
          SLOT_PRESETS = slot_presets_default;
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/visuals.ts
      var visuals_exports = {};
      __export(visuals_exports, {
        buildReadoutList: () => buildReadoutList,
        fillVisual: () => fillVisual,
        readoutById: () => readoutById,
        sizeFor: () => sizeFor,
        thumbFor: () => thumbFor
      });
      function buildReadoutList(options) {
        return (options || []).map(function(option) {
          return {
            value: typeof option.value === "string" ? parseInt(option.value, 10) : option.value,
            label: option.label,
            icon: option.icon || "\xB7",
            color: option.blockColor || "#999"
          };
        }).filter(function(readout) {
          return readout.value !== ID_EMPTY;
        });
      }
      function sizeFor(id) {
        return isTall(id) ? "tall" : "slot";
      }
      function thumbFor(thumbs, readout) {
        if (!readout) {
          return null;
        }
        const byLabel = thumbs[readout.label];
        return byLabel && byLabel[sizeFor(readout.value)] || null;
      }
      function fillVisual(el, readout, thumbs, withName) {
        el.innerHTML = "";
        el.classList.remove("sb-pal-fallback");
        el.style.background = "";
        if (!readout) {
          const empty = el.ownerDocument.createElement("span");
          empty.className = "sb-slot-empty";
          empty.textContent = "Empty";
          el.appendChild(empty);
          return;
        }
        const thumb = thumbFor(thumbs, readout);
        if (thumb) {
          const img = el.ownerDocument.createElement("img");
          img.className = el.classList.contains("sb-pal") ? "sb-pal-img" : "sb-slot-img";
          img.src = thumb;
          img.alt = readout.label;
          el.appendChild(img);
          return;
        }
        el.classList.add("sb-pal-fallback");
        el.style.background = readout.color;
        const icon = el.ownerDocument.createElement("span");
        icon.className = "sb-pal-icon";
        icon.textContent = readout.icon;
        el.appendChild(icon);
        if (withName) {
          const name = el.ownerDocument.createElement("span");
          name.textContent = readout.label;
          el.appendChild(name);
        }
      }
      function readoutById(readouts, id) {
        for (let i = 0; i < readouts.length; i++) {
          if (readouts[i].value === id) {
            return readouts[i];
          }
        }
        return null;
      }
      var init_visuals = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/visuals.ts"() {
          init_geometry();
        }
      });

      // lib/ts/clay/builder/ts/drag.ts
      function createDrag(spec, doc = document) {
        const threshold = spec.threshold === void 0 ? DEFAULT_THRESHOLD : spec.threshold;
        let armed = null;
        let active = null;
        function place(ghost, x, y) {
          if (spec.anchor === "pointer") {
            ghost.style.left = x + "px";
            ghost.style.top = y + "px";
            return;
          }
          const box = ghost.getBoundingClientRect();
          ghost.style.left = x - box.width / 2 + "px";
          ghost.style.top = y - box.height / 2 + "px";
        }
        function begin(payload, x, y) {
          const ghost = spec.ghost(payload);
          doc.body.appendChild(ghost);
          active = { payload, ghost };
          place(ghost, x, y);
          track(x, y);
        }
        function track(x, y) {
          if (!active) {
            return;
          }
          place(active.ghost, x, y);
          const target = spec.hitTest(x, y);
          const allowed = target !== null && spec.allows(active.payload, target);
          spec.highlight(active.payload, target, allowed);
        }
        function end() {
          if (active) {
            active.ghost.remove();
            active = null;
          }
          armed = null;
          spec.highlight(null, null, false);
        }
        doc.addEventListener("pointermove", (event) => {
          if (armed) {
            const moved = Math.abs(event.clientX - armed.x) > threshold || Math.abs(event.clientY - armed.y) > threshold;
            if (moved) {
              const payload = armed.payload;
              armed = null;
              if (spec.lift) {
                spec.lift(payload);
              }
              begin(payload, event.clientX, event.clientY);
            }
          }
          if (active) {
            track(event.clientX, event.clientY);
            event.preventDefault();
          }
        });
        doc.addEventListener("pointerup", (event) => {
          if (!active) {
            armed = null;
            return;
          }
          const payload = active.payload;
          const target = spec.hitTest(event.clientX, event.clientY);
          end();
          if (target !== null && spec.allows(payload, target)) {
            spec.drop(payload, target);
            return;
          }
          spec.dropOutside(payload);
        });
        doc.addEventListener("pointercancel", () => {
          const payload = active && active.payload;
          end();
          if (payload !== null && payload !== void 0) {
            spec.dropOutside(payload);
          }
        });
        return {
          start(payload, event) {
            begin(payload, event.clientX, event.clientY);
            event.preventDefault();
          },
          arm(payload, event) {
            armed = { payload, x: event.clientX, y: event.clientY };
            event.preventDefault();
          }
        };
      }
      var DEFAULT_THRESHOLD;
      var init_drag = __esm({
        "lib/ts/clay/builder/ts/drag.ts"() {
          DEFAULT_THRESHOLD = 10;
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/drag.ts
      var drag_exports = {};
      __export(drag_exports, {
        installDrag: () => installDrag,
        tryPlace: () => tryPlace
      });
      function tryPlace(slots, id, from, to) {
        const next = slots.slice();
        if (from >= 0) {
          next[from] = slots[to];
        }
        next[to] = id;
        const swallowed = swallowedBy(id, to);
        if (swallowed >= 0) {
          next[swallowed] = ID_EMPTY;
        }
        for (let slot = 0; slot < SLOT_COUNT; slot++) {
          if (!canPlace(next[slot], slot)) {
            return null;
          }
        }
        return next;
      }
      function installDrag(env) {
        const doc = env.root.ownerDocument || document;
        function slotAt(x, y) {
          for (let slot = 0; slot < SLOT_COUNT; slot++) {
            const box = env.slotEls[slot].getBoundingClientRect();
            if (box.width && x >= box.left && x <= box.right && y >= box.top && y <= box.bottom) {
              return slot;
            }
          }
          return null;
        }
        const drag = createDrag({
          anchor: "center",
          ghost(dragged) {
            const ghost = doc.createElement("div");
            ghost.className = "sb-ghost" + (isTall(dragged.id) ? " tall" : "");
            fillVisual(ghost, readoutById(env.readouts, dragged.id), env.thumbs, false);
            return ghost;
          },
          hitTest: slotAt,
          allows(dragged, slot) {
            return tryPlace(env.getSlots(), dragged.id, dragged.from, slot) !== null;
          },
          highlight(dragged, slot, allowed) {
            for (let i = 0; i < SLOT_COUNT; i++) {
              env.slotEls[i].classList.remove("over", "blocked");
            }
            if (slot !== null) {
              env.slotEls[slot].classList.add(allowed ? "over" : "blocked");
            }
          },
          drop(dragged, slot) {
            const next = tryPlace(env.getSlots(), dragged.id, dragged.from, slot);
            if (next) {
              env.setSlots(next);
            }
          },
          dropOutside(dragged) {
            if (dragged.from < 0) {
              return;
            }
            const cleared = env.getSlots().slice();
            cleared[dragged.from] = ID_EMPTY;
            env.setSlots(cleared);
          }
        }, doc);
        return {
          fromPalette(id, event) {
            if (readoutById(env.readouts, id)) {
              drag.start({ id, from: -1 }, event);
            }
          },
          // armed rather than started, so a tap on a panel is not mistaken for picking it up
          fromSlot(slot, event) {
            const id = env.getSlots()[slot];
            if (id !== ID_EMPTY) {
              drag.arm({ id, from: slot }, event);
            }
          }
        };
      }
      var init_drag2 = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/drag.ts"() {
          init_geometry();
          init_visuals();
          init_drag();
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/init.ts
      var init_exports = {};
      __export(init_exports, {
        init: () => init
      });
      function init() {
        const root = this.$element[0];
        const cfg = this.config || {};
        const fireChange = this.trigger.bind(this);
        const READOUTS = buildReadoutList(cfg.moduleOptions || []);
        const THUMBS = cfg.moduleThumbnails || {};
        const hidden = root.querySelector(".sb-value");
        const slotsWrap = root.querySelector(".sb-slots");
        const palette = root.querySelector(".sb-palette");
        const slotEls = [];
        for (let slot = 0; slot < SLOT_COUNT; slot++) {
          slotEls.push(root.querySelector('.sb-slot[data-slot="' + slot + '"]'));
        }
        let slots = readSlots(root, hidden.value);
        function render() {
          slotsWrap.classList.toggle("tall", isTall(slots[0]));
          for (let slot = 0; slot < SLOT_COUNT; slot++) {
            const readout = slots[slot] === ID_EMPTY ? null : readoutById(READOUTS, slots[slot]);
            fillVisual(slotEls[slot], readout, THUMBS, false);
            slotEls[slot].classList.toggle("filled", Boolean(readout));
          }
          publish();
        }
        function publish() {
          const own = formatId(slots[0]);
          if (hidden.value !== own) {
            hidden.value = own;
            fireChange("change");
          }
          writeStores(root, slots);
        }
        function setSlots(next) {
          slots = sanitize(next);
          render();
        }
        const drag = installDrag({
          root,
          readouts: READOUTS,
          thumbs: THUMBS,
          slotEls,
          getSlots: function() {
            return slots;
          },
          setSlots
        });
        for (let slot = 0; slot < SLOT_COUNT; slot++) {
          (function(index) {
            slotEls[index].addEventListener("pointerdown", function(event) {
              drag.fromSlot(index, event);
            });
          })(slot);
        }
        READOUTS.forEach(function(readout) {
          const cell = root.ownerDocument.createElement("div");
          cell.className = "sb-pal" + (isTall(readout.value) ? " tall" : "");
          cell.title = readout.label;
          fillVisual(cell, readout, THUMBS, true);
          cell.addEventListener("pointerdown", function(event) {
            drag.fromPalette(readout.value, event);
          });
          palette.appendChild(cell);
        });
        const presetBtns = root.querySelectorAll(".sb-preset");
        for (let i = 0; i < presetBtns.length; i++) {
          presetBtns[i].addEventListener("click", function(event) {
            const id = event.currentTarget.getAttribute("data-preset") || "";
            if (SLOT_PRESETS[id]) {
              setSlots(SLOT_PRESETS[id]);
            }
          });
        }
        const clear = root.querySelector(".sb-btn-clear");
        if (clear) {
          clear.addEventListener("click", function() {
            setSlots([ID_EMPTY, ID_EMPTY, ID_EMPTY, ID_EMPTY]);
          });
        }
        const hooked = root;
        hooked._sbSet = function(value) {
          slots = readSlots(root, value);
          render();
        };
        hooked._sbGet = function() {
          return formatId(slots[0]);
        };
        render();
        setTimeout(function() {
          const late = readSlots(root, hidden.value);
          const moved = late.some(function(id, slot) {
            return id !== slots[slot];
          });
          if (moved) {
            slots = late;
            render();
          }
        }, 0);
      }
      var init_init = __esm({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/ts/slots/init.ts"() {
          init_geometry();
          init_codec();
          init_presets();
          init_visuals();
          init_drag2();
        }
      });

      // watchfaces/lcars-stardate/src/pkjs/clay/builder/component-entry.js
      var require_component_entry = __commonJS({
        "watchfaces/lcars-stardate/src/pkjs/clay/builder/component-entry.js"(exports, module) {
          init_geometry();
          init_codec();
          init_presets();
          init_visuals();
          init_drag2();
          init_init();
          module.exports = (init_init(), __toCommonJS(init_exports));
        }
      });
      return require_component_entry();
    })();

    __clayComponent.init.call(this);
  }
};
