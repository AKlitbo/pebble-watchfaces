// the persisted ClaySettings struct and its storage key
#pragma once
#include <pebble.h>

#define SETTINGS_KEY 5  // bumped: DistanceUnit toggle -> TraversalMode select

typedef struct ClaySettings
{
    bool TemperatureUnit;   // false = Celsius, true = Fahrenheit
    char DateFormat[16];    // strftime() pattern for the date line
    uint8_t Theme;          // LCARS frame theme (0=Classic, 1=Lower Decks, 2=PADD, 3=Nemesis Blue)
    uint8_t TraversalMode;  // TRAVERSAL readout: 0=Steps, 1=Miles, 2=Kilometers
    uint8_t TimeFormat;     // 0=System, 1=12-hour, 2=24-hour, 3=Swatch .beats
} ClaySettings;

extern ClaySettings g_settings;

void settings_init(void);
void settings_save(void);
