/**
 * @file settings_schema.c
 * @brief ide-vscode-emery settings schema.
 *
 * The face has only ever added fields to the end, so it stays on version 1 and needs no
 * migration. It keeps its own struct and persist key, so its blob and versioning are
 * independent of any other face.
 *
 * @ingroup watchface-vscode
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "persist_keys.h"
#include "desktop/desktop.h"

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
 * A field added to the end needs no version bump, since a shorter save already on the
 * watch reads back with the new field on its default. The schema's first size is the
 * 25 bytes the first release saved, without the hourly buzz and the desktop settings at the end,
 * so a save that short still loads with each on its default.
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
    uint8_t desktop;
    uint8_t desktop_theme;
} VscodeSettings;

static VscodeSettings s_settings;

// the face's own settings, numbered past the shared ones. the library still defaults, sanitises
// and serialises them, but will not index them for a typed read, so the accessor below reaches
// into the struct. cast because the field table's id is a SettingId and mixing enums is an error
#define SETTING_DESKTOP ((SettingId)(SETTING_COUNT))
#define SETTING_DESKTOP_THEME ((SettingId)(SETTING_COUNT + 1))

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

    // --- the face's own ---
    // VS Code is the default, so a save from before the Desktop setting reads back as today's look
    { .id = SETTING_DESKTOP, .message_key = &MESSAGE_KEY_APPEARANCE_DESKTOP,
      .type = SETTING_ENUM_U8, .offset = offsetof(VscodeSettings, desktop),
      .enum_count = DESKTOP_COUNT, .default_num = DESKTOP_VSCODE },
    // kept apart from the VS Code theme, so each one survives a switch between desktops
    { .id = SETTING_DESKTOP_THEME, .message_key = &MESSAGE_KEY_APPEARANCE_DESKTOP_THEME,
      .type = SETTING_ENUM_U8, .offset = offsetof(VscodeSettings, desktop_theme),
      .enum_count = DESKTOP_THEME_COUNT, .default_num = DESKTOP_THEME_CLASSIC },
};

static const SettingsSchema s_schema = {
    .key = VSCODE_SETTINGS_KEY,
    .version = VSCODE_SETTINGS_VERSION,
    SETTINGS_BLOB(s_settings, 25),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // every older save is this layout cut short, which the first size already covers
};

const SettingsSchema *vscode_settings_schema(void)
{
    return &s_schema;
}

uint8_t vscode_desktop(void)
{
    return s_settings.desktop;
}

uint8_t vscode_desktop_theme(void)
{
    return s_settings.desktop_theme;
}

/** @} */
