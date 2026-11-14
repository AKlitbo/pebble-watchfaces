/**
 * @file settings_schema.c
 * @brief ide-vscode-emery settings schema.
 *
 * This face ships its fields from day one, so it owns version 1 with no legacy
 * migration. It keeps its own struct and persist key, so its blob and versioning
 * are independent of any other face.
 *
 * @ingroup watchface-vscode
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "persist_keys.h"

#include <stddef.h>

#define VSCODE_SETTINGS_VERSION 1

/**
 * @addtogroup watchface-vscode
 * @{
 */

/**
 * @brief vscode's persisted settings.
 *
 * Fields only ever get added to the end, so there is no older layout to bring forward.
 * A new one bumps the version and a blob already on the watch still reads back fine.
 * The schema's first size is the 25 bytes the first release saved, without the hourly
 * buzz at the end, so a save that short still loads with the buzz on its default.
 */
typedef struct VscodeSettings
{
    uint8_t version;
    uint8_t temperature_unit;
    char    date_format[16];
    uint8_t theme;
    uint8_t steps_mode;
    uint8_t time_format;
    bool    bluetooth_icon;
    uint8_t vibe_connect;
    uint8_t vibe_disconnect;
    uint8_t battery_display;
    uint8_t hourly_vibe;
} VscodeSettings;

static VscodeSettings s_settings;

// every KNOWN_ entry takes its default from the settings page, so the date opens on the compact
// single line ("JUN 18") the page sets to fit the terminal panel
// 6 themes: Dark+ / Light / Terminal / Cyberpunk / Synthwave / Mono
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(VscodeSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(VscodeSettings, date_format)),
    KNOWN_THEME(offsetof(VscodeSettings, theme), 6),
    KNOWN_STEPS_MODE(offsetof(VscodeSettings, steps_mode)),
    KNOWN_TIME_FORMAT(offsetof(VscodeSettings, time_format)),
    KNOWN_BLUETOOTH_ICON(offsetof(VscodeSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(VscodeSettings, vibe_connect)),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(VscodeSettings, vibe_disconnect)),
    KNOWN_BATTERY_DISPLAY(offsetof(VscodeSettings, battery_display)),
    KNOWN_HOURLY_VIBE(offsetof(VscodeSettings, hourly_vibe)),
};

static const SettingsSchema s_schema = {
    .key = VSCODE_SETTINGS_KEY,
    .version = VSCODE_SETTINGS_VERSION,
    SETTINGS_BLOB(s_settings, 25),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // no legacy blobs. this face never existed without these fields
};

const SettingsSchema *vscode_settings_schema(void)
{
    return &s_schema;
}

/** @} */
