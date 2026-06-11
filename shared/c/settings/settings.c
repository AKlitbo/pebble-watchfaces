// ClaySettings persistence: defaults, load, save
#include "../settings/settings.h"

ClaySettings g_settings;

// seed g_settings with first-run defaults
static void prv_default_settings(void)
{
    g_settings.TemperatureUnit = false;
    strncpy(g_settings.DateFormat, "%Y.%m%d", sizeof(g_settings.DateFormat) - 1);
    g_settings.DateFormat[sizeof(g_settings.DateFormat) - 1] = '\0';
    g_settings.Theme = 0;          // Classic
    g_settings.TraversalMode = 0;  // Steps
    g_settings.TimeFormat = 0;     // System
}

// load defaults, then overlay any persisted settings
void settings_init(void)
{
    prv_default_settings();
    persist_read_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));
}

// persist g_settings
void settings_save(void)
{
    persist_write_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));
}
