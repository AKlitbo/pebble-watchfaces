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
#include "sketchbook/theme/palette_active.h"
#include "theme/theme.h"

#include <stddef.h>

#define RIDGELINE_SETTINGS_VERSION 3

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
    // --- added in v3 ---
    uint8_t palette_mode;
    uint8_t shuffle_interval;
    bool    use_night_palette;
    uint8_t night_palette;
    bool    use_quiet_palette;
    uint8_t quiet_palette;
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
#define SETTING_PALETTE_MODE     ((SettingId)(SETTING_COUNT + 2))
#define SETTING_SHUFFLE_INTERVAL ((SettingId)(SETTING_COUNT + 3))
#define SETTING_NIGHT_PALETTE_ON ((SettingId)(SETTING_COUNT + 4))
#define SETTING_NIGHT_PALETTE    ((SettingId)(SETTING_COUNT + 5))
#define SETTING_QUIET_PALETTE_ON ((SettingId)(SETTING_COUNT + 6))
#define SETTING_QUIET_PALETTE    ((SettingId)(SETTING_COUNT + 7))

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
    { .id = SETTING_PALETTE_MODE, .message_key = &MESSAGE_KEY_APPEARANCE_PALETTE_MODE,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, palette_mode),
      .enum_count = PALETTE_MODE_COUNT, .default_num = PALETTE_MODE_FIXED },
    { .id = SETTING_SHUFFLE_INTERVAL, .message_key = &MESSAGE_KEY_APPEARANCE_SHUFFLE_INTERVAL,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, shuffle_interval),
      .enum_count = SHUFFLE_INTERVAL_COUNT, .default_num = SHUFFLE_HOURLY },
    { .id = SETTING_NIGHT_PALETTE_ON, .message_key = &MESSAGE_KEY_APPEARANCE_NIGHT_PALETTE_ON,
      .type = SETTING_BOOL, .offset = offsetof(RidgelineSettings, use_night_palette),
      .default_num = 0 },
    { .id = SETTING_NIGHT_PALETTE, .message_key = &MESSAGE_KEY_APPEARANCE_NIGHT_PALETTE,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, night_palette),
      .enum_count = THEME_COUNT, .default_num = 0 },
    { .id = SETTING_QUIET_PALETTE_ON, .message_key = &MESSAGE_KEY_APPEARANCE_QUIET_PALETTE_ON,
      .type = SETTING_BOOL, .offset = offsetof(RidgelineSettings, use_quiet_palette),
      .default_num = 0 },
    { .id = SETTING_QUIET_PALETTE, .message_key = &MESSAGE_KEY_APPEARANCE_QUIET_PALETTE,
      .type = SETTING_ENUM_U8, .offset = offsetof(RidgelineSettings, quiet_palette),
      .enum_count = THEME_COUNT, .default_num = 0 },
};

static const SettingsSchema s_schema = {
    .key = SKETCHBOOK_SETTINGS_KEY,
    .version = RIDGELINE_SETTINGS_VERSION,
    // frozen at what 1.0.0 shipped, so a blob from a watch running it is still accepted
    .min_versioned_size = RIDGELINE_SETTINGS_V1_SIZE,
    .blob = &s_settings,
    .blob_size = SETTINGS_BLOB_SIZE(RidgelineSettings),
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
    // a round screen ships one layout. the settings store lives on the phone per app, so a value
    // set on emery follows the user across to a gabbro, and the other two layouts exist in the
    // code without ever having been laid out for a circle. hiding the picker in the config page
    // is cosmetic, this is the part that holds
#if defined(PBL_ROUND)
    return LAYOUT_STANDARD;
#else
    return s_settings.layout;
#endif
}

bool ridgeline_show_meridiem(void)
{
    return s_settings.meridiem;
}

void ridgeline_palette_pick(PalettePick *out)
{
    out->mode = s_settings.palette_mode;
    out->day_theme = settings_u8(SETTING_THEME);
    out->shuffle_interval = s_settings.shuffle_interval;
    out->use_night = s_settings.use_night_palette;
    out->night_theme = s_settings.night_palette;
    out->use_quiet = s_settings.use_quiet_palette;
    out->quiet_theme = s_settings.quiet_palette;
    // what is firing now is a live reading, so the caller fills these from the clock and Quiet Time
    out->night_on = false;
    out->quiet_on = false;
}

/** @} */
