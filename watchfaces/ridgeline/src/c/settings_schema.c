/**
 * @file settings_schema.c
 * @brief Ridgeline settings schema.
 *
 * This face has never shipped without any of these fields, so it owns version 1 with no
 * legacy migration. It keeps its own struct and persist key, so its blob and versioning are
 * independent of any other face.
 *
 * @ingroup watchface-ridgeline
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "persist_keys.h"
#include "theme/theme.h"

#include <stddef.h>

#define RIDGELINE_SETTINGS_VERSION 1

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

/**
 * @brief Ridgeline's persisted settings.
 *
 * Version 1 carries every field, so there is no older layout to bring forward. Fields only
 * ever get added to the end from here. A new one bumps the version and a blob already on the
 * watch still reads back fine.
 */
typedef struct RidgelineSettings
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
    uint8_t hourly_vibe;
    bool    quiet_time_icon;
} RidgelineSettings;

static RidgelineSettings s_settings;

// ridgeline subscribes to every known setting. the date shows as a readout-style "SAT 19 JUN"
// rather than the library's numeric default, which suits the hand-drawn lettering
static const SettingField s_fields[] = {
    // temperature unit is an inline SETTING_ENUM_U8, not the shared KNOWN_ macro. the config
    // page shows a Celsius/Fahrenheit dropdown, so Clay sends a select (a cstring "0"/"1")
    { .id = SETTING_TEMPERATURE_UNIT, .message_key = &MESSAGE_KEY_WEATHER_TEMPERATURE_UNIT,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, temperature_unit),
      .enum_count = 2, .default_num = 0, .affects_weather = true },
    KNOWN_DATE_FORMAT(offsetof(RidgelineSettings, date_format), "%a %d %b"),
    KNOWN_THEME(offsetof(RidgelineSettings, theme), THEME_COUNT),
    KNOWN_STEPS_MODE(offsetof(RidgelineSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(RidgelineSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(RidgelineSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(RidgelineSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(RidgelineSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_HOURLY_VIBE(offsetof(RidgelineSettings, hourly_vibe), VIBE_COUNT),
    KNOWN_QUIET_TIME_ICON(offsetof(RidgelineSettings, quiet_time_icon)),
};

static const SettingsSchema s_schema = {
    .key = RIDGELINE_SETTINGS_KEY,
    .version = RIDGELINE_SETTINGS_VERSION,
    // ridgeline is unshipped so its v1 is the current struct. once it ships freeze this to a
    // literal byte count and bump the version on any further field append
    .min_versioned_size = sizeof(RidgelineSettings),
    .blob = &s_settings,
    .blob_size = sizeof(RidgelineSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // no legacy blobs. this face never existed without these fields
};

const SettingsSchema *ridgeline_settings_schema(void)
{
    return &s_schema;
}

/** @} */
