// theme to background-resource and accent-color mapping
#pragma once
#include <pebble.h>

// maps the Theme setting (0=Classic, 1=Lower Decks, 2=PADD, 3=Nemesis Blue) to
// its baked LCARS background resource
uint32_t bg_resource_for_theme(uint8_t theme);

// the theme's top-left cap color, so painted overlays (e.g. the battery gauge)
// can match the panel they sit on instead of staying a fixed classic violet
GColor panel_accent_for_theme(uint8_t theme);
