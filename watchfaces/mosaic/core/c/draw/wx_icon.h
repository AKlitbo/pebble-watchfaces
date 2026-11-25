#pragma once
#include <pebble.h>
#include "ui/icon_cache.h"

// the current-condition weather picture. it changes with the weather, so unlike the
// fixed-id icon cache this keeps its own bitmap and reloads when weather_store_cond()
// changes. shared by the weather modules that draw the condition glyph

// reloads the picture when the condition changed, then returns it (NULL when there is no
// real condition or the load failed). fills *margin with the art's transparent margins
GBitmap *wx_icon_get(IconMargins *margin);

// the same glyph at 40px, for a panel with room for a bigger picture. it keeps its own bitmap, so
// a 1x2 and a 2x2 showing the weather at once each hold the size they draw
GBitmap *wx_icon_get_big(IconMargins *margin);

// frees both cached pictures. safe to call from any weather module's cleanup and safe to call twice
void wx_icon_cleanup(void);
