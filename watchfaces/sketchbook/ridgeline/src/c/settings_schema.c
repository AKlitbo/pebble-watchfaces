/**
 * @file settings_schema.c
 * @brief Ridgeline settings schema.
 *
 * It keeps its own struct and persist key, so its blob and versioning are independent of any
 * other face. Version 1 is what 1.0.0 shipped and version 2 appends the layout choice, which a
 * watch updating from 1.0.0 picks up as a short read rather than as a reset.
 *
 * @ingroup watchface-ridgeline
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "sketchbook/persist_keys.h"
#include "theme/theme.h"

#include <stddef.h>

#define RIDGELINE_SETTINGS_VERSION 2

/// What v1 shipped with, frozen now that it has. Never change it: this is the size of a blob
/// already on watches, not the size of the struct below
#define RIDGELINE_SETTINGS_V1_SIZE 26

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

/**
 * @brief Ridgeline's persisted settings.
 *
 * Fields are only ever appended, never reordered or removed. A watch on 1.0.0 holds a v1 blob
 * ending at quiet_time_icon, and the loader reads it short and leaves the v2 fields on their
 * defaults, so an update keeps the settings the user already chose.
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
    // --- added in v2 ---
    uint8_t layout;
    bool    meridiem;
} RidgelineSettings;

// every member is a single byte and the struct needs no padding, so v1 is just the field count
// above the marker
_Static_assert(offsetof(RidgelineSettings, layout) == RIDGELINE_SETTINGS_V1_SIZE,
    "v1 ended at quiet_time_icon; RIDGELINE_SETTINGS_V1_SIZE no longer matches that blob");

static RidgelineSettings s_settings;

// this face's own settings, numbered past the shared ones. the library still defaults, sanitises
// and serialises them, but will not index them for a typed read, so the accessors below reach
// into the struct. cast because the field table's id is a SettingId and mixing enums is an error
#define SETTING_LAYOUT   ((SettingId)(SETTING_COUNT))
#define SETTING_MERIDIEM ((SettingId)(SETTING_COUNT + 1))

// ridgeline subscribes to every known setting. the date shows as a readout-style "SAT 19 JUN"
// rather than the library's numeric default, which suits the hand-drawn lettering
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(RidgelineSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(RidgelineSettings, date_format), "%a %d %b"),
    KNOWN_THEME(offsetof(RidgelineSettings, theme), THEME_COUNT),
    KNOWN_STEPS_MODE(offsetof(RidgelineSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(RidgelineSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(RidgelineSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(RidgelineSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(RidgelineSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_HOURLY_VIBE(offsetof(RidgelineSettings, hourly_vibe), VIBE_COUNT),
    KNOWN_QUIET_TIME_ICON(offsetof(RidgelineSettings, quiet_time_icon)),

    // --- this face's own ---
    { .id = SETTING_LAYOUT, .message_key = &MESSAGE_KEY_APPEARANCE_LAYOUT,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, layout),
      .enum_count = LAYOUT_COUNT, .default_num = LAYOUT_STANDARD, .affects_layout = true },
    { .id = SETTING_MERIDIEM, .message_key = &MESSAGE_KEY_CLOCK_MERIDIEM,
      .type = SETTING_BOOL, .offset = offsetof(RidgelineSettings, meridiem),
      .default_num = 1 },
};

static const SettingsSchema s_schema = {
    .key = SKETCHBOOK_SETTINGS_KEY,
    .version = RIDGELINE_SETTINGS_VERSION,
    // frozen at what 1.0.0 shipped, so a blob from a watch running it is still accepted
    .min_versioned_size = RIDGELINE_SETTINGS_V1_SIZE,
    .blob = &s_settings,
    .blob_size = sizeof(RidgelineSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    // nothing to migrate: v1 and v2 differ only by an appended field, which the loader handles
    .migrate = NULL,
};

const SettingsSchema *ridgeline_settings_schema(void)
{
    return &s_schema;
}

uint8_t ridgeline_layout(void)
{
    return s_settings.layout;
}

bool ridgeline_show_meridiem(void)
{
    return s_settings.meridiem;
}

/** @} */
