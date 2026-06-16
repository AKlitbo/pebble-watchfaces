// ClaySettings persistence: defaults, versioned load/migration, save
#include "../settings/settings.h"

#include <string.h>

ClaySettings g_settings;

// the pre-versioning layout (no Version field) that shipped under SETTINGS_KEY 5,
// kept only so settings_init can lift an existing blob into the versioned struct
typedef struct ClaySettingsV0
{
    bool TemperatureUnit;
    char DateFormat[16];
    uint8_t Theme;
    uint8_t TraversalMode;
    uint8_t TimeFormat;
} ClaySettingsV0;

// seed g_settings with first-run defaults
static void prv_default_settings(void)
{
    g_settings.Version = SETTINGS_VERSION;
    g_settings.TemperatureUnit = false;
    strncpy(g_settings.DateFormat, "%Y.%m%d", sizeof(g_settings.DateFormat) - 1);
    g_settings.DateFormat[sizeof(g_settings.DateFormat) - 1] = '\0';
    g_settings.Theme = 0;          // Classic
    g_settings.TraversalMode = 0;  // Steps
    g_settings.TimeFormat = 0;     // System
}

// lift a pre-versioning blob into the current struct, preserving every field
static void prv_migrate_v0(void)
{
    ClaySettingsV0 legacy;
    persist_read_data(SETTINGS_KEY, &legacy, sizeof(legacy));

    g_settings.TemperatureUnit = legacy.TemperatureUnit;
    memcpy(g_settings.DateFormat, legacy.DateFormat, sizeof(g_settings.DateFormat));
    g_settings.Theme = legacy.Theme;
    g_settings.TraversalMode = legacy.TraversalMode;
    g_settings.TimeFormat = legacy.TimeFormat;
    g_settings.Version = SETTINGS_VERSION;
}

// clamp every field to a legal value so a damaged blob never renders as garbage
// returns true if anything was corrected so the caller can re-save
static bool prv_sanitize(void)
{
    bool changed = false;

    // a bool is only 0 or 1
    if (g_settings.TemperatureUnit != false && g_settings.TemperatureUnit != true)
    {
        g_settings.TemperatureUnit = false;
        changed = true;
    }

    // DateFormat must be NUL-terminated, non-empty, printable ASCII
    bool date_ok = false;
    for (size_t i = 0; i < sizeof(g_settings.DateFormat); i++)
    {
        char c = g_settings.DateFormat[i];
        if (c == '\0')
        {
            date_ok = (i > 0);
            break;
        }
        
        if (c < 0x20 || c > 0x7E)
        {
            break;
        }
    }

    if (!date_ok)
    {
        strncpy(g_settings.DateFormat, "%Y.%m%d", sizeof(g_settings.DateFormat) - 1);
        g_settings.DateFormat[sizeof(g_settings.DateFormat) - 1] = '\0';
        changed = true;
    }

    if (g_settings.Theme >= THEME_COUNT)
    {
        g_settings.Theme = 0;
        changed = true;
    }

    if (g_settings.TraversalMode >= TRAVERSAL_MODE_COUNT)
    {
        g_settings.TraversalMode = 0;
        changed = true;
    }

    if (g_settings.TimeFormat >= TIME_FORMAT_COUNT)
    {
        g_settings.TimeFormat = 0;
        changed = true;
    }

    return changed;
}

// load defaults, then overlay a trusted blob. any blob we cannot trust is discarded
// so the watch always boots into a valid state
void settings_init(void)
{
    prv_default_settings();

    if (!persist_exists(SETTINGS_KEY))
    {
        return;  // first run keeps the defaults
    }

    int stored = persist_get_size(SETTINGS_KEY);

    // pre-versioning blob (no Version byte): lift each field across, then re-save versioned
    if (stored == (int)sizeof(ClaySettingsV0))
    {
        prv_migrate_v0();
        prv_sanitize();
        settings_save();        
        return;
    }

    // versioned blob: v1 size or larger (smaller would shift fields), no bigger than the
    // current struct. a partial read upgrades an older, shorter version to current defaults
    if (stored >= SETTINGS_V1_SIZE && stored <= (int)sizeof(g_settings))
    {
        persist_read_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));

        if (g_settings.Version >= 1 && g_settings.Version <= SETTINGS_VERSION)
        {
            // NOTE: non-append schema changes (reorder/remove/retype) need a per-version fixup here

            bool healed = prv_sanitize();

            // re-save if we upgraded an older version or had to repair a damaged field
            if (g_settings.Version < SETTINGS_VERSION || healed)
            {
                g_settings.Version = SETTINGS_VERSION;
                settings_save();
            }

            return;
        }
    }

    // unrecognized size or bad version: reset (the bad read may have clobbered g_settings)
    prv_default_settings();
    settings_save();
}

// persist g_settings
void settings_save(void)
{
    persist_write_data(SETTINGS_KEY, &g_settings, sizeof(g_settings));
}
