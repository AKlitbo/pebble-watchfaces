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
 * this module's scope (module-level constants, requires, etc.). Those are
 * undefined in the webview and will throw. Keep such values inline.
 */

module.exports = {
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
    '</div>'
  ].join(''),

  style: [
    '.loc-search .input { position: relative; }',
    '.loc-search .loc-list { position: absolute; left: 0; right: 0; top: 100%; z-index: 10; list-style: none; margin: 0; padding: 0; background: #fff; color: #000; border: 1px solid #ccc; border-radius: 0 0 4px 4px; max-height: 180px; overflow-y: auto; }',
    '.loc-search .loc-list:not(.show) { display: none; }',
    '.loc-search .loc-item { padding: 10px 12px; cursor: pointer; border-bottom: 1px solid #eee; }',
    '.loc-search .loc-item:last-child { border-bottom: none; }',
    '.loc-search .loc-item:hover, .loc-search .loc-item:active { background: #ff4700; color: #fff; }'
  ].join(''),

  manipulator: {
    set: function(value) {
      var root = this.$element[0];
      var label = '';

      root.querySelector('.loc-value').value = value || '';

      if (value) {
        try {
          var parsed = JSON.parse(value);
          if (parsed && parsed.label) {
            label = parsed.label;
          }
        } catch (error) {
          // a timezone value round-trips as offset then label so show just the label
          // anything else is corrupt so leave the box empty
          if (typeof value === 'string' && value !== '0') {
            var commaIndex = value.indexOf(',');
            if (commaIndex !== -1) {
              label = value.substring(commaIndex + 1);
            }
          }
        }
      }

      root.querySelector('.loc-query').value = label;
    },

    get: function() {
      var val = this.$element[0].querySelector('.loc-value').value || '';
      if (!val) {
        return '';
      }

      // for a timezone field pull the offset and label out for the watch
      if (this.config.messageKey && this.config.messageKey.indexOf('TIME_ZONE_OFFSET') !== -1) {
        try {
          var obj = JSON.parse(val);
          return obj.offset + ',' + obj.label;
        } catch (error) {
          return val; // fallback if already formatted
        }
      }

      // otherwise return the raw json blob
      return val;
    }
  },

  initialize: function() {
    var self = this;
    var root = self.$element[0];

    var queryEl = root.querySelector('.loc-query');
    var hiddenEl = root.querySelector('.loc-value');
    var listEl = root.querySelector('.loc-list');
    var timer = null;
    var seq = 0;

    root.querySelector('.loc-label').textContent = self.config.label || 'Location';
    if (self.config.attributes && self.config.attributes.placeholder) {
      queryEl.placeholder = self.config.attributes.placeholder;
    }
    if (self.config.description) {
      var descEl = root.querySelector('.description');
      descEl.textContent = self.config.description;
      descEl.style.display = 'block';
    }

    function labelFor(place) {
      return [place.name, place.admin1, place.country].filter(Boolean).join(', ');
    }

    function hideList() {
      while (listEl.firstChild) {
        listEl.removeChild(listEl.firstChild);
      }
      listEl.classList.remove('show');
    }

    function resolveOffset(place) {
      var xhr = new XMLHttpRequest();
      var url = 'https://api.open-meteo.com/v1/forecast?latitude=' + place.latitude + '&longitude=' + place.longitude + '&current_weather=true&timezone=auto';

      xhr.onload = function() {
        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            var data = JSON.parse(xhr.responseText);
            var offsetMinutes = 0;
            if (typeof data.utc_offset_seconds !== 'undefined') {
              offsetMinutes = Math.round(data.utc_offset_seconds / 60);
            }
            hiddenEl.value = JSON.stringify({
              lat: place.latitude,
              lon: place.longitude,
              label: labelFor(place),
              offset: offsetMinutes
            });
          } catch (error) {}
        }
      };
      xhr.open('GET', url);
      xhr.send();
    }

    function renderResults(results) {
      hideList();
      if (!results || !results.length) {
        return;
      }

      results.forEach(function(place) {
        var item = document.createElement('li');
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
            offset: 0
          });

          resolveOffset(place);
          hideList();
        });
        listEl.appendChild(item);
      });

      listEl.classList.add('show');
    }

    function search(query) {
      var mySeq = ++seq;
      var xhr = new XMLHttpRequest();
      var url = 'https://geocoding-api.open-meteo.com/v1/search?name=' +
        encodeURIComponent(query) + '&count=5&language=en&format=json';

      xhr.onload = function() {
        if (mySeq !== seq) {
          return;
        }
        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            var data = JSON.parse(xhr.responseText);
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
      var query = (queryEl.value || '').trim();

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
      if (!root.contains(event.target)) hideList();
    });
  }
};