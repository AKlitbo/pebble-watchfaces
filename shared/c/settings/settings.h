// the persisted ClaySettings struct and its storage key
#pragma once
#include <pebble.h>

#define SETTINGS_KEY 5       // bumped: DistanceUnit toggle -> TraversalMode select
#define SETTINGS_VERSION 1   // ClaySettings schema version, stamped into Version below
#define SETTINGS_V1_SIZE 21  // size of the first versioned layout, fields are append-only so this is the smallest blob accepted as versioned. never change it

// valid option counts, used by settings_init to clamp a damaged blob to a legal value
#define THEME_COUNT 4           // Classic, Lower Decks, PADD, Nemesis Blue
#define TRAVERSAL_MODE_COUNT 3  // Steps, Miles, Kilometers
#define TIME_FORMAT_COUNT 4     // System, 12-hour, 24-hour, Swatch .beats

typedef struct ClaySettings
{
    uint8_t Version;        // schema version (SETTINGS_VERSION), drives settings_init migration
    bool TemperatureUnit;   // false = Celsius, true = Fahrenheit
    char DateFormat[16];    // strftime() pattern for the date line
    uint8_t Theme;          // LCARS frame theme (0=Classic, 1=Lower Decks, 2=PADD, 3=Nemesis Blue)
    uint8_t TraversalMode;  // TRAVERSAL readout: 0=Steps, 1=Miles, 2=Kilometers
    uint8_t TimeFormat;     // 0=System, 1=12-hour, 2=24-hour, 3=Swatch .beats
} ClaySettings;

extern ClaySettings g_settings;

void settings_init(void);
void settings_save(void);
