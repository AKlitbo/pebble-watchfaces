/**
 * @file settings_schema.c
 * @brief Treeline settings schema.
 *
 * It keeps its own struct and persist key, so its blob and versioning are independent of any
 * other face. A brand-new face, so it starts clean at version 1 with no migration history.
 *
 * @ingroup watchface-treeline
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "sketchbook/persist_keys.h"
#include "sketchbook/theme/palette_active.h"
#include "theme/theme.h"

#include <stddef.h>

#define TREELINE_SETTINGS_VERSION 2

/// What v1 shipped with, frozen now that it has. Never change it: this is the size of a blob
/// already on watches, not the size of the struct below
#define TREELINE_SETTINGS_V1_SIZE 28

/**
 * @addtogroup watchface-treeline
 * @{
 */

/**
 * @brief Treeline's persisted settings.
 *
 * Fields are only ever appended, never reordered or removed, so a later version can be read
 * short off a watch running this one.
 */
typedef struct TreelineSettings
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
    // --- added in v2 ---
    uint8_t palette_mode;
    uint8_t shuffle_interval;
    bool    use_night_palette;
    uint8_t night_palette;
    bool    use_quiet_palette;
    uint8_t quiet_palette;
} TreelineSettings;

// every member is a single byte and the struct needs no padding, so v1 is just the field count
// above the marker
_Static_assert(offsetof(TreelineSettings, palette_mode) == TREELINE_SETTINGS_V1_SIZE,
    "v1 ended at meridiem; TREELINE_SETTINGS_V1_SIZE no longer matches that blob");

static TreelineSettings s_settings;

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

// treeline subscribes to every known setting. the date shows as a readout-style "SAT 19 JUN"
// rather than the library's numeric default, which suits the hand-drawn lettering
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(TreelineSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(TreelineSettings, date_format), "%a %d %b"),
    KNOWN_THEME(offsetof(TreelineSettings, theme), THEME_COUNT),
    KNOWN_STEPS_MODE(offsetof(TreelineSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(TreelineSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(TreelineSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(TreelineSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(TreelineSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_HOURLY_VIBE(offsetof(TreelineSettings, hourly_vibe), VIBE_COUNT),
    KNOWN_QUIET_TIME_ICON(offsetof(TreelineSettings, quiet_time_icon)),

    // --- this face's own ---
    { .id = SETTING_LAYOUT, .message_key = &MESSAGE_KEY_APPEARANCE_LAYOUT,
      .type = SETTING_ENUM_U8, .offset = offsetof(TreelineSettings, layout),
      .enum_count = LAYOUT_COUNT, .default_num = LAYOUT_STANDARD, .affects_layout = true },
    { .id = SETTING_MERIDIEM, .message_key = &MESSAGE_KEY_CLOCK_MERIDIEM,
      .type = SETTING_BOOL, .offset = offsetof(TreelineSettings, meridiem),
      .default_num = 1 },
    { .id = SETTING_PALETTE_MODE, .message_key = &MESSAGE_KEY_APPEARANCE_PALETTE_MODE,
      .type = SETTING_ENUM_U8, .offset = offsetof(TreelineSettings, palette_mode),
      .enum_count = PALETTE_MODE_COUNT, .default_num = PALETTE_MODE_FIXED },
    { .id = SETTING_SHUFFLE_INTERVAL, .message_key = &MESSAGE_KEY_APPEARANCE_SHUFFLE_INTERVAL,
      .type = SETTING_ENUM_U8, .offset = offsetof(TreelineSettings, shuffle_interval),
      .enum_count = SHUFFLE_INTERVAL_COUNT, .default_num = SHUFFLE_HOURLY },
    { .id = SETTING_NIGHT_PALETTE_ON, .message_key = &MESSAGE_KEY_APPEARANCE_NIGHT_PALETTE_ON,
      .type = SETTING_BOOL, .offset = offsetof(TreelineSettings, use_night_palette),
      .default_num = 0 },
    { .id = SETTING_NIGHT_PALETTE, .message_key = &MESSAGE_KEY_APPEARANCE_NIGHT_PALETTE,
      .type = SETTING_ENUM_U8, .offset = offsetof(TreelineSettings, night_palette),
      .enum_count = THEME_COUNT, .default_num = 0 },
    { .id = SETTING_QUIET_PALETTE_ON, .message_key = &MESSAGE_KEY_APPEARANCE_QUIET_PALETTE_ON,
      .type = SETTING_BOOL, .offset = offsetof(TreelineSettings, use_quiet_palette),
      .default_num = 0 },
    { .id = SETTING_QUIET_PALETTE, .message_key = &MESSAGE_KEY_APPEARANCE_QUIET_PALETTE,
      .type = SETTING_ENUM_U8, .offset = offsetof(TreelineSettings, quiet_palette),
      .enum_count = THEME_COUNT, .default_num = 0 },
};

static const SettingsSchema s_schema = {
    .key = SKETCHBOOK_SETTINGS_KEY,
    .version = TREELINE_SETTINGS_VERSION,
    // frozen at what v1 shipped, so a blob from a watch running it is still accepted
    .min_versioned_size = TREELINE_SETTINGS_V1_SIZE,
    .blob = &s_settings,
    .blob_size = SETTINGS_BLOB_SIZE(TreelineSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,
};

const SettingsSchema *treeline_settings_schema(void)
{
    return &s_schema;
}

uint8_t treeline_layout(void)
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

bool treeline_show_meridiem(void)
{
    return s_settings.meridiem;
}

void treeline_palette_pick(PalettePick *out)
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
