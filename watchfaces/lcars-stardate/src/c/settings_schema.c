/**
 * @file settings_schema.c
 * @brief lcars-stardate-emery settings schema: this face's persisted-settings identity.
 *
 * key 5 and the 21-byte v1 floor are frozen. Changing either makes watches in the
 * field lose their settings.
 *
 * @ingroup watchface-lcars
 */
#include "settings_schema.h"
#include "ops/ops.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "persist_keys.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define LCARS_SETTINGS_VERSION 6
// smallest versioned blob accepted. fields are append-only so this never changes
#define LCARS_SETTINGS_V1_SIZE 21

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief lcars's persisted settings.
 *
 * The byte layout is frozen: it shipped as a flat blob (version byte, then
 * each field in this order), so members may be renamed but never reordered,
 * retyped, or inserted. Only appended. The static asserts below guard it.
 */
typedef struct LcarsSettings
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
    bool    quiet_time_icon;
    uint8_t hourly_vibe;
    uint8_t slot_lt;
    uint8_t slot_lb;
    uint8_t slot_rt;
    uint8_t slot_rb;
    char    time_zone_offset_1[32]; // "offset,City, Region, CC" as Clay's location search sends it
} LcarsSettings;

_Static_assert(sizeof(LcarsSettings) == 62, "lcars blob size is frozen at 62 bytes (v6)");
_Static_assert(offsetof(LcarsSettings, bluetooth_icon) == LCARS_SETTINGS_V1_SIZE,
               "lcars v1 floor is frozen; the v2 fields must stay appended after it");

static LcarsSettings s_settings;

// the four ops slots are this face's own and are numbered past the shared ones
// the library still defaults and cleans and saves them. it will not index them for
// a typed read though so the accessors below reach into the struct
// the cast is there because the field table's id is a SettingId and mixing enums
// is an error
#define SETTING_SLOT_LT ((SettingId)(SETTING_COUNT))
#define SETTING_SLOT_LB ((SettingId)(SETTING_COUNT + 1))
#define SETTING_SLOT_RT ((SettingId)(SETTING_COUNT + 2))
#define SETTING_SLOT_RB ((SettingId)(SETTING_COUNT + 3))

// lcars subscribes to every known setting in its frozen struct order. "%Y.%m%d" is
// its numeric stardate-style date default
static const SettingField s_fields[] = {
    KNOWN_TEMPERATURE_UNIT(offsetof(LcarsSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(LcarsSettings, date_format), "%Y.%m%d"),
    KNOWN_THEME(offsetof(LcarsSettings, theme), 9),
    KNOWN_STEPS_MODE(offsetof(LcarsSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(LcarsSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(LcarsSettings, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(LcarsSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(LcarsSettings, vibe_disconnect), VIBE_COUNT),
    KNOWN_QUIET_TIME_ICON(offsetof(LcarsSettings, quiet_time_icon)),
    KNOWN_HOURLY_VIBE(offsetof(LcarsSettings, hourly_vibe), VIBE_COUNT),

    // --- this face's own ---
    // an untouched face shows the weather block filling the left column with the
    // heart rate over the step count on the right. picking these defaults means
    // someone upgrading keeps the watch they are already looking at
    { .id = SETTING_SLOT_LT, .message_key = &MESSAGE_KEY_APPEARANCE_SLOT_LEFT_TOP,
      .type = SETTING_ENUM_U8, .offset = offsetof(LcarsSettings, slot_lt),
      .enum_count = OPS_COUNT, .default_num = OPS_SENSORS },
    { .id = SETTING_SLOT_LB, .message_key = &MESSAGE_KEY_APPEARANCE_SLOT_LEFT_BOTTOM,
      .type = SETTING_ENUM_U8, .offset = offsetof(LcarsSettings, slot_lb),
      .enum_count = OPS_COUNT, .default_num = OPS_EMPTY },
    { .id = SETTING_SLOT_RT, .message_key = &MESSAGE_KEY_APPEARANCE_SLOT_RIGHT_TOP,
      .type = SETTING_ENUM_U8, .offset = offsetof(LcarsSettings, slot_rt),
      .enum_count = OPS_COUNT, .default_num = OPS_HEART },
    { .id = SETTING_SLOT_RB, .message_key = &MESSAGE_KEY_APPEARANCE_SLOT_RIGHT_BOTTOM,
      .type = SETTING_ENUM_U8, .offset = offsetof(LcarsSettings, slot_rb),
      .enum_count = OPS_COUNT, .default_num = OPS_STEPS },

    // the ZONE 1 readout's zone. the id is SETTING_COUNT because nothing reads this through the
    // library's typed accessors, only the two helpers at the bottom of this file
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_CLOCK_TIMEZONE_1, .type = SETTING_CSTRING,
      .offset = offsetof(LcarsSettings, time_zone_offset_1),
      .size = sizeof(s_settings.time_zone_offset_1),
      .default_str = "60,London, England, GB" },
};

/**
 * @brief The pre-versioning layout (no version field) that shipped under key 5.
 *
 * Kept only so the migration can lift an existing blob into the current struct.
 */
typedef struct ClaySettingsV0
{
    bool TemperatureUnit;
    char DateFormat[16];
    uint8_t Theme;
    uint8_t StepsMode;
    uint8_t TimeFormat;
} ClaySettingsV0;

/**
 * @brief Lift a pre-versioning blob, preserving every field.
 *
 * Recognised purely by its size, which never collides with a versioned blob.
 *
 * @param stored_size The size of the stored blob.
 * @return True if migrated, false otherwise.
 */
static bool lcars_migrate(int stored_size)
{
    if (stored_size != (int)sizeof(ClaySettingsV0))
    {
        return false;
    }

    ClaySettingsV0 legacy;
    persist_read_data(LCARS_SETTINGS_KEY, &legacy, sizeof(legacy));

    s_settings.temperature_unit = legacy.TemperatureUnit;
    memcpy(s_settings.date_format, legacy.DateFormat, sizeof(s_settings.date_format));
    s_settings.theme = legacy.Theme;
    s_settings.steps_mode = legacy.StepsMode;
    s_settings.time_format = legacy.TimeFormat;

    return true;
}

static const SettingsSchema s_schema = {
    .key = LCARS_SETTINGS_KEY,
    .version = LCARS_SETTINGS_VERSION,
    .min_versioned_size = LCARS_SETTINGS_V1_SIZE,
    .blob = &s_settings,
    .blob_size = sizeof(LcarsSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = lcars_migrate,
};

const SettingsSchema *lcars_settings_schema(void)
{
    return &s_schema;
}

uint8_t lcars_slot_lt(void)
{
    return s_settings.slot_lt;
}

uint8_t lcars_slot_lb(void)
{
    return s_settings.slot_lb;
}

uint8_t lcars_slot_rt(void)
{
    return s_settings.slot_rt;
}

uint8_t lcars_slot_rb(void)
{
    return s_settings.slot_rb;
}

int16_t lcars_zone_1_offset_minutes(void)
{
    return (int16_t)atoi(s_settings.time_zone_offset_1);
}

const char *lcars_zone_1_name(void)
{
    const char *comma = strchr(s_settings.time_zone_offset_1, ',');
    return comma ? comma + 1 : "";
}

/** @} */
