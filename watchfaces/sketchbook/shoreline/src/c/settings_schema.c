/**
 * @file settings_schema.c
 * @brief Shoreline settings schema.
 *
 * It keeps its own struct and persist key, so its blob and versioning are independent of any
 * other face. A brand-new face, so it starts clean at version 1 with no migration history.
 *
 * @ingroup watchface-shoreline
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "sketchbook/persist_keys.h"
#include "theme/theme.h"

#include <stddef.h>

#define SHORELINE_SETTINGS_VERSION 1

/**
 * @addtogroup watchface-shoreline
 * @{
 */

/**
 * @brief Shoreline's persisted settings.
 *
 * Fields are only ever appended, never reordered or removed, so a later version can be read
 * short off a watch running this one.
 */
typedef struct ShorelineSettings
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
    uint8_t layout;
    bool    meridiem;
} ShorelineSettings;

static ShorelineSettings s_settings;

// this face's own settings, numbered past the shared ones. the library still defaults, sanitises
// and serialises them, but will not index them for a typed read, so the accessors below reach
// into the struct. cast because the field table's id is a SettingId and mixing enums is an error
#define SETTING_LAYOUT   ((SettingId)(SETTING_COUNT))
#define SETTING_MERIDIEM ((SettingId)(SETTING_COUNT + 1))

// shoreline subscribes to every known setting. the date shows as a readout-style "SAT 19 JUN"
// rather than the library's numeric default, which suits the hand-drawn lettering
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(ShorelineSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(ShorelineSettings, date_format), "%a %d %b"),
    KNOWN_THEME(offsetof(ShorelineSettings, theme), THEME_COUNT),
    KNOWN_STEPS_MODE(offsetof(ShorelineSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(ShorelineSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(ShorelineSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(ShorelineSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(ShorelineSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_HOURLY_VIBE(offsetof(ShorelineSettings, hourly_vibe), VIBE_COUNT),
    KNOWN_QUIET_TIME_ICON(offsetof(ShorelineSettings, quiet_time_icon)),

    // --- this face's own ---
    { .id = SETTING_LAYOUT, .message_key = &MESSAGE_KEY_APPEARANCE_LAYOUT,
      .type = SETTING_ENUM_U8, .offset = offsetof(ShorelineSettings, layout),
      .enum_count = LAYOUT_COUNT, .default_num = LAYOUT_STANDARD, .affects_layout = true },
    { .id = SETTING_MERIDIEM, .message_key = &MESSAGE_KEY_CLOCK_MERIDIEM,
      .type = SETTING_BOOL, .offset = offsetof(ShorelineSettings, meridiem),
      .default_num = 1 },
};

static const SettingsSchema s_schema = {
    .key = SKETCHBOOK_SETTINGS_KEY,
    .version = SHORELINE_SETTINGS_VERSION,
    .blob = &s_settings,
    .blob_size = sizeof(ShorelineSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,
};

const SettingsSchema *shoreline_settings_schema(void)
{
    return &s_schema;
}

uint8_t shoreline_layout(void)
{
    return s_settings.layout;
}

bool shoreline_show_meridiem(void)
{
    return s_settings.meridiem;
}

/** @} */
