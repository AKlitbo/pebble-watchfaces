/**
 * Clay custom component for live location autocomplete.
 *
 * As the user types, it queries Open-Meteo's free geocoding API and shows
 * matching places. Selecting one stores the resolved coordinates so the watch
 * never has to geocode.
 *
 * Persisted value:
 *   JSON string {lat, lon, label}
 *
 * IMPORTANT: `initialize` and the `manipulator` methods are serialised
 * (via .toString()) and run inside the Clay config webview, a separate JS
 * context. They must be self-contained and must NOT reference anything from
 * this module's scope (module-level constants, imports, etc.). Those are
 * undefined in the webview and will throw. Keep such values inline.
 */

/** The Clay component context the manipulator and initialize bind to. */
export interface ClayComponentContext {
  $element: HTMLElement[];
  config: { label?: string; description?: string; messageKey?: string; attributes?: { placeholder?: string } };
}

/** One Open-Meteo geocoding result the autocomplete list shows. */
interface GeoPlace {
  name?: string;
  admin1?: string;
  country?: string;
  latitude?: number;
  longitude?: number;
}

export default {
  name: 'locationsearch',

  template: [
    '<div class="component component-input loc-search">',
    '  <label class="tap-highlight">',
    '    <span class="label loc-label"></span>',
    '    <span class="input">',
    '      <input type="text" class="loc-query" autocomplete="off" autocorrect="off" autocapitalize="words">',
    '      <ul class="loc-list"></ul>',
    '    </span>',
    '  </label>',
    '  <div class="description" style="display:none;"></div>',
    '  <input type="hidden" class="loc-value">',
    '</div>',
  ].join(''),

  style: [
    '.loc-search .input { position: relative; }',
    '.loc-search .loc-list { position: absolute; left: 0; right: 0; top: 100%; z-index: 10; list-style: none; margin: 0; padding: 0; background: #fff; color: #000; border: 1px solid #ccc; border-radius: 0 0 4px 4px; max-height: 180px; overflow-y: auto; }',
    '.loc-search .loc-list:not(.show) { display: none; }',
    '.loc-search .loc-item { padding: 10px 12px; cursor: pointer; border-bottom: 1px solid #eee; }',
    '.loc-search .loc-item:last-child { border-bottom: none; }',
    '.loc-search .loc-item:hover, .loc-search .loc-item:active { background: #ff4700; color: #fff; }',
  ].join(''),

  manipulator: {
    /** Restores a saved selection: shows the place label and keeps the raw stored value. */
    set: function(this: ClayComponentContext, value: string) {
      const root = this.$element[0];
      let label = '';

      (root.querySelector('.loc-value') as HTMLInputElement).value = value || '';

      if (value) {
        try {
          const parsed = JSON.parse(value);
          if (parsed && parsed.label) {
            label = parsed.label;
          }
        } catch (error) {
          // a timezone value round-trips as offset then label so show just the label
          // anything else is corrupt so leave the box empty
          if (typeof value === 'string' && value !== '0') {
            const commaIndex = value.indexOf(',');
            if (commaIndex !== -1) {
              label = value.substring(commaIndex + 1);
            }
          }
        }
      }

      (root.querySelector('.loc-query') as HTMLInputElement).value = label;
    },

    /**
     * Returns the value to persist. A timezone field emits "offset,label" for the
     * watch, any other location field keeps the raw JSON blob.
     */
    get: function(this: ClayComponentContext) {
      const val = (this.$element[0].querySelector('.loc-value') as HTMLInputElement).value || '';
      if (!val) {
        return '';
      }

      // for a timezone field pull the offset and label out for the watch
      // match any timezone-named key (like CLOCK_TIMEZONE_1 or TIME_ZONE_OFFSET) so the
      // watch gets "offset,label" while a plain location key keeps the raw json blob
      if (this.config.messageKey && /TIME_?ZONE/i.test(this.config.messageKey)) {
        try {
          const obj = JSON.parse(val);
          // the watch header fonts are glyph-subset so an accented letter draws as a box. NFD
          // splits a letter from its accent and the range strip drops the accent, so a place
          // like Zurich spelled with an umlaut goes over as plain ASCII. this runs serialised
          // in the Clay webview so it stays self-contained
          const label = String(obj.label || '').normalize('NFD').replace(/[\u0300-\u036f]/g, '');
          return obj.offset + ',' + label;
        } catch (error) {
          return val; // fallback if already formatted
        }
      }

      // otherwise return the raw json blob
      return val;
    },
  },

  /**
   * Wires up the live autocomplete: a debounced geocoder query, a results
   * dropdown, and persisting the chosen place's coordinates (plus offset).
   */
  initialize: function(this: ClayComponentContext) {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const self = this;
    const root = self.$element[0];

    const queryEl = root.querySelector('.loc-query') as HTMLInputElement;
    const hiddenEl = root.querySelector('.loc-value') as HTMLInputElement;
    const listEl = root.querySelector('.loc-list') as HTMLElement;
    let timer: ReturnType<typeof setTimeout> | null = null;
    let seq = 0;

    (root.querySelector('.loc-label') as HTMLElement).textContent = self.config.label || 'Location';
    if (self.config.attributes && self.config.attributes.placeholder) {
      queryEl.placeholder = self.config.attributes.placeholder;
    }
    if (self.config.description) {
      const descEl = root.querySelector('.description') as HTMLElement;
      descEl.textContent = self.config.description;
      descEl.style.display = 'block';
    }

    /** Joins a place's name, region, and country into one label, dropping missing parts. */
    function labelFor(place: GeoPlace) {
      return [place.name, place.admin1, place.country].filter(Boolean).join(', ');
    }

    /** Clears and hides the suggestions dropdown. */
    function hideList() {
      while (listEl.firstChild) {
        listEl.removeChild(listEl.firstChild);
      }
      listEl.classList.remove('show');
    }

    /**
     * Looks up the selected place's UTC offset and stores it alongside the
     * coordinates, so a timezone field can ship "offset,label" to the watch.
     */
    function resolveOffset(place: GeoPlace) {
      const xhr = new XMLHttpRequest();
      const url = 'https://api.open-meteo.com/v1/forecast?latitude=' + place.latitude + '&longitude=' + place.longitude + '&current_weather=true&timezone=auto';

      xhr.onload = function() {
        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            const data = JSON.parse(xhr.responseText);
            let offsetMinutes = 0;
            if (typeof data.utc_offset_seconds !== 'undefined') {
              offsetMinutes = Math.round(data.utc_offset_seconds / 60);
            }
            hiddenEl.value = JSON.stringify({
              lat: place.latitude,
              lon: place.longitude,
              label: labelFor(place),
              offset: offsetMinutes,
            });
          } catch (error) {}
        }
      };
      xhr.open('GET', url);
      xhr.send();
    }

    /** Renders the geocoder results as a clickable dropdown, persisting the pick on selection. */
    function renderResults(results: GeoPlace[]) {
      hideList();
      if (!results || !results.length) {
        return;
      }

      results.forEach(function(place: GeoPlace) {
        const item = document.createElement('li');
        item.className = 'loc-item';
        item.textContent = labelFor(place);
        item.addEventListener('click', function(event) {
          event.stopPropagation();
          queryEl.value = labelFor(place);

          // pre-fill the JSON in case the offset fetch never comes back
          hiddenEl.value = JSON.stringify({
            lat: place.latitude,
            lon: place.longitude,
            label: labelFor(place),
            offset: 0,
          });

          resolveOffset(place);
          hideList();
        });
        listEl.appendChild(item);
      });

      listEl.classList.add('show');
    }

    /** Queries the Open-Meteo geocoder for the typed term, ignoring a stale response that lands after a newer query. */
    function search(query: string) {
      const mySeq = ++seq;
      const xhr = new XMLHttpRequest();
      const url = 'https://geocoding-api.open-meteo.com/v1/search?name=' +
        encodeURIComponent(query) + '&count=5&language=en&format=json';

      xhr.onload = function() {
        if (mySeq !== seq) {
          return;
        }
        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            const data = JSON.parse(xhr.responseText);
            renderResults(data && data.results);
          } catch (error) {
            hideList();
          }
        } else {
          hideList();
        }
      };

      xhr.onerror = function() {
        if (mySeq === seq) {
          hideList();
        }
      };
      xhr.ontimeout = function() {
        if (mySeq === seq) {
          hideList();
        }
      };
      xhr.timeout = 10000;

      xhr.open('GET', url);
      xhr.send();
    }

    queryEl.addEventListener('input', function() {
      hiddenEl.value = '';
      const query = (queryEl.value || '').trim();

      if (timer) {
        clearTimeout(timer);
      }
      if (query.length < 2) {
        seq++;
        return hideList();
      }

      timer = setTimeout(function() { search(query); }, 300);
    });

    document.addEventListener('click', function(event) {
      if (!root.contains(event.target as Node)) {hideList();}
    });
  },
};
