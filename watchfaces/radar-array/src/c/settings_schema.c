/**
 * @file settings_schema.c
 * @brief radar-array-emery settings schema.
 *
 * This face never existed without the bluetooth fields, so it owns version 1
 * with no legacy migration. It keeps its own struct and persist key, so its
 * blob and versioning are independent of any other face.
 *
 * @ingroup watchface-radar
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"

#include <stddef.h>

#define RADAR_SETTINGS_KEY 1
#define RADAR_SETTINGS_VERSION 1

/**
 * @addtogroup watchface-radar
 * @{
 */

/**
 * @brief radar's persisted settings.
 *
 * Version 1 carries every field, so there is no older layout to bring forward.
 * Fields only ever get added to the end from here. A new one bumps the version
 * and a blob already on the watch still reads back fine.
 */
typedef struct RadarSettings
{
    uint8_t version;
    bool    temperature_unit;
    char    date_format[16];
    uint8_t theme;
    uint8_t steps_mode;
    uint8_t time_format;
    bool    bluetooth_icon;
    uint8_t vibe_connect;
    uint8_t vibe_disconnect;
} RadarSettings;

static RadarSettings s_settings;

// radar subscribes to every known setting. it shows a readout-style date
// ("SAT 19 JUN") in place of the library's numeric default
static const SettingField s_fields[] = {
    // temperature unit is an inline SETTING_BOOL. a °F toggle on the config page
    // the shared KNOWN_ macro does not cover it since other faces send it as a select
    { .id = SETTING_TEMPERATURE_UNIT, .message_key = &MESSAGE_KEY_WEATHER_TEMPERATURE_UNIT,
      .type = SETTING_BOOL, .offset = offsetof(RadarSettings, temperature_unit), .affects_weather = true },
    KNOWN_DATE_FORMAT(offsetof(RadarSettings, date_format), "%a %d %b"),
    KNOWN_THEME(offsetof(RadarSettings, theme), 7),
    KNOWN_STEPS_MODE(offsetof(RadarSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(RadarSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(RadarSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(RadarSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(RadarSettings, vibe_disconnect), VIBE_COUNT),
};

static const SettingsSchema s_schema = {
    .key = RADAR_SETTINGS_KEY,
    .version = RADAR_SETTINGS_VERSION,
    // radar is unshipped so its v1 is the current struct. once radar ships freeze
    // this to a literal byte count and bump the version on any further field append
    .min_versioned_size = sizeof(RadarSettings),
    .blob = &s_settings,
    .blob_size = sizeof(RadarSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // no legacy blobs. this face never existed without these fields
};

const SettingsSchema *radar_settings_schema(void)
{
    return &s_schema;
}

/** @} */
