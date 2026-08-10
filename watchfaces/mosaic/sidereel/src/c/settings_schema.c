/**
 * @file settings_schema.c
 * @brief sidereel's settings schema.
 *
 * The look is mostly one theme choice rather than a colour per part: the panels take their
 * colours from the theme (and, under Custom, from the packed string theme/custom_colors.c owns),
 * and the day track follows the same choice through theme.c. The four colours below are the
 * exception, painting the reel and the hour pointer on top of whichever theme is running.
 *
 * @ingroup watchface-sidereel
 */
#include "settings_schema.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "persist_keys.h"
#include "mosaic/engine/catalog.h"
#include "layout.h"
#include "theme/custom_colors.h"
#include "theme/theme.h"
#include "mosaic/draw/header_fonts.h"
#include "mosaic/draw/panel_styles.h"
#include "units/wind.h"

#include <stddef.h>
#include <string.h>

#define SIDEREEL_SETTINGS_VERSION 1
#define SIDEREEL_GOAL_VIBE_VERSION 1

// the Goal Met Vibe strings, sized as gridlock sizes them: the longest preset dropdown value
// plus room to spare, and the config's 120 character cap on the custom box
#define SIDEREEL_GOAL_VIBE_PICK_LEN   64
#define SIDEREEL_GOAL_VIBE_CUSTOM_LEN 122
#define SIDEREEL_GOAL_VIBE_SIZE       (1 + SIDEREEL_GOAL_VIBE_PICK_LEN + SIDEREEL_GOAL_VIBE_CUSTOM_LEN)

// what a fresh install shows: the battery and the date above the pointer, a tall weather panel
// below. keep in step with the "default" row of src/data/layout-presets.json
#define SIDEREEL_DEFAULT_LAYOUT "2,0,0,2,1;17,1,0,2,1;3,3,0,2,2"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief sidereel's persisted settings.
 *
 * Version 1 carries every field, so there is no older layout to bring forward. Fields only ever
 * get added to the end from here.
 */
typedef struct SidereelSettings
{
    uint8_t version;
    uint8_t temperature_unit;
    char    date_format[16];
    uint8_t steps_mode;
    uint8_t time_format;
    uint8_t hourly_vibe;
    bool    bluetooth_icon;
    bool    quiet_time_icon;
    uint8_t vibe_connect;
    uint8_t vibe_disconnect;
    char    layout[SIDEREEL_LAYOUT_LEN];
    uint8_t theme;
    char    time_zone_1[40];
    // the goals the health panels measure against, each an index into its table below
    uint8_t goal_steps;
    uint8_t goal_calories;
    uint8_t goal_sleep;
    uint8_t goal_active;
    uint8_t goal_hr;
    uint8_t goal_distance;
    uint8_t distance_unit;
    uint8_t wind_unit;
    uint8_t week_start;
    uint8_t panel_style;
    uint8_t header_font;
    // the face's own colours, and the switch that lets them sit on top of whichever theme is
    // running. off, every theme keeps the row theme.c holds for it
    bool    face_colors;
    GColor  pointer_color;
    GColor  pointer_ink;
    GColor  reel_color;
    GColor  reel_ink;
} SidereelSettings;

static SidereelSettings s_settings;

/**
 * @brief The Goal Met Vibe, in its own blob because the two rhythm strings will not fit beside
 * the layout and the place name in one persist slot.
 */
typedef struct SidereelGoalVibe
{
    uint8_t version;
    char    pick[SIDEREEL_GOAL_VIBE_PICK_LEN];     // a sentinel (empty/S/L/D/C) or a comma list of milliseconds
    char    custom[SIDEREEL_GOAL_VIBE_CUSTOM_LEN]; // the user's own comma list, used when the pick is Custom
} SidereelGoalVibe;
_Static_assert(sizeof(SidereelGoalVibe) == SIDEREEL_GOAL_VIBE_SIZE, "goal vibe blob size is frozen; fields append only");

static SidereelGoalVibe s_goal_vibe;

// goal option values looked up by the saved menu choice, matching gridlock's tables so the same
// pick means the same number on either face
static const int16_t s_steps_goals[]   = {5000, 7500, 10000, 12500, 15000, 20000, 25000};
static const int16_t s_cal_goals[]     = {500, 1000, 1500, 2000, 2500, 3000};
static const int16_t s_sleep_goals_h[] = {6, 7, 8, 9, 10};
static const int16_t s_active_goals[]  = {15, 30, 45, 60, 90};
static const int16_t s_hr_limits[]     = {150, 160, 170, 180, 190, 200};
// distance goals in meters. the readout converts to the user's miles/km at draw time
static const int16_t s_dist_goals_m[]  = {2000, 3000, 5000, 8000, 10000, 15000, 20000};

// face-only enum field. rides its own message key and is read straight off the face struct
#define FACE_ENUM(member, key, count, dflt)                                          \
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_##key, .type = SETTING_ENUM_U8, \
      .offset = offsetof(SidereelSettings, member), .enum_count = (count), .default_num = (dflt) }

// face-only colour field. the default is the 0xRRGGBB the picker would show
#define FACE_COLOR(member, key, dflt)                                             \
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_##key, .type = SETTING_COLOR, \
      .offset = offsetof(SidereelSettings, member), .default_num = (dflt) }

static const SettingField s_fields[] = {
    KNOWN_THEME(offsetof(SidereelSettings, theme), THEME_COUNT),
    KNOWN_TEMPERATURE_UNIT(offsetof(SidereelSettings, temperature_unit)),
    KNOWN_DATE_FORMAT(offsetof(SidereelSettings, date_format), "%b %d"),
    KNOWN_STEPS_MODE(offsetof(SidereelSettings, steps_mode), STEPS_MODE_COUNT),
    KNOWN_TIME_FORMAT(offsetof(SidereelSettings, time_format), TIME_FORMAT_COUNT),
    KNOWN_HOURLY_VIBE(offsetof(SidereelSettings, hourly_vibe), VIBE_COUNT),
    KNOWN_BLUETOOTH_ICON(offsetof(SidereelSettings, bluetooth_icon)),
    KNOWN_QUIET_TIME_ICON(offsetof(SidereelSettings, quiet_time_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(SidereelSettings, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(SidereelSettings, vibe_disconnect), VIBE_COUNT),

    // the whole panel layout rides one string the drag builder writes, so a placement is a
    // block with its own row and height rather than a fixed slot with a size flag beside it
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelSettings, layout), .size = sizeof(s_settings.layout),
      .default_str = SIDEREEL_DEFAULT_LAYOUT, .affects_layout = true },


    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_CLOCK_TIMEZONE_1, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelSettings, time_zone_1), .size = sizeof(s_settings.time_zone_1),
      .default_str = "60,London, England, GB" },

    // the goals every health panel measures against, and the units and week start the shared
    // panel bodies read. defaults match gridlock's so a panel reads the same on either face
    KNOWN_DISTANCE_UNIT(offsetof(SidereelSettings, distance_unit), DISTANCE_UNIT_COUNT),
    FACE_ENUM(goal_steps,    HEALTH_GOAL_STEPS,    ARRAY_LENGTH(s_steps_goals),   2),
    FACE_ENUM(goal_calories, HEALTH_GOAL_CALORIES, ARRAY_LENGTH(s_cal_goals),     3),
    FACE_ENUM(goal_sleep,    HEALTH_GOAL_SLEEP,    ARRAY_LENGTH(s_sleep_goals_h), 2),
    FACE_ENUM(goal_active,   HEALTH_GOAL_ACTIVE,   ARRAY_LENGTH(s_active_goals),  1),
    FACE_ENUM(goal_hr,       HEALTH_GOAL_HR,       ARRAY_LENGTH(s_hr_limits),     3),
    FACE_ENUM(goal_distance, HEALTH_GOAL_DISTANCE, ARRAY_LENGTH(s_dist_goals_m),  2),
    FACE_ENUM(wind_unit,     WEATHER_WIND_UNIT,    4, 0),
    FACE_ENUM(week_start,    CLOCK_WEEK_START,     2, 0),

    // how the face is framed. the header font is a shared id so panel.c can read it with
    // settings_u8(SETTING_HEADER_FONT) rather than reaching into this struct
    FACE_ENUM(panel_style, APPEARANCE_PANEL_STYLE, PANEL_STYLE_COUNT, 0),
    { .id = SETTING_HEADER_FONT, .message_key = &MESSAGE_KEY_APPEARANCE_HEADER_FONT,
      .type = SETTING_ENUM_U8, .offset = offsetof(SidereelSettings, header_font),
      .enum_count = HEADER_FONT_COUNT, .default_num = 0 },

    // the four face colours, and their switch. they start on the Vibrant row's chrome, so
    // turning them on lands somewhere other than mono before a single pick
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_APPEARANCE_FACE_COLORS,
      .type = SETTING_BOOL, .offset = offsetof(SidereelSettings, face_colors), .default_num = 0 },
    FACE_COLOR(pointer_color, APPEARANCE_POINTER_COLOR, 0xFF0000),
    FACE_COLOR(pointer_ink,   APPEARANCE_POINTER_INK,   0xFFFFFF),
    FACE_COLOR(reel_color,    APPEARANCE_REEL_COLOR,    0xFFFFFF),
    FACE_COLOR(reel_ink,      APPEARANCE_REEL_INK,      0x000000),
};

// --- goal vibe (key 4) ---
// both ride as plain strings. the dropdown value is the pick (a sentinel or a comma list of
// milliseconds), and the input holds the user's own comma list for when the pick is Custom
static const SettingField s_goal_vibe_fields[] = {
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_HEALTH_GOAL_VIBE, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelGoalVibe, pick), .size = sizeof(s_goal_vibe.pick), .default_str = "" },
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_HEALTH_GOAL_VIBE_CUSTOM, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelGoalVibe, custom), .size = sizeof(s_goal_vibe.custom), .default_str = "" },
};

static const SettingsSchema s_goal_vibe_schema = {
    .key = SIDEREEL_GOAL_VIBE_KEY,
    .version = SIDEREEL_GOAL_VIBE_VERSION,
    .min_versioned_size = SIDEREEL_GOAL_VIBE_SIZE,
    .blob = &s_goal_vibe,
    .blob_size = sizeof(s_goal_vibe),
    .fields = s_goal_vibe_fields,
    .field_count = ARRAY_LENGTH(s_goal_vibe_fields),
    .migrate = NULL,
    .companion = &sidereel_custom_theme_schema,  // the packed appearance string, in its own two slots
};

static const SettingsSchema s_schema = {
    .key = SIDEREEL_SETTINGS_KEY,
    .version = SIDEREEL_SETTINGS_VERSION,
    // sidereel is unshipped so its v1 is the current struct. once it ships freeze this to a
    // literal byte count and bump the version on any further field append
    .min_versioned_size = sizeof(SidereelSettings),
    .blob = &s_settings,
    .blob_size = sizeof(SidereelSettings),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // v1 is the only shape a stored blob can have, so nothing to bring forward
    // the chain runs on to the goal vibe, which carries the packed appearance string behind it
    .companion = &s_goal_vibe_schema,
};

const SettingsSchema *sidereel_settings_schema(void)
{
    return &s_schema;
}

const char *sidereel_layout(void)
{
    return s_settings.layout;
}

const char *sidereel_timezone_1(void)
{
    return s_settings.time_zone_1;
}

uint8_t sidereel_panel_style(void)
{
    return s_settings.panel_style;
}

bool sidereel_face_colors(void)
{
    return s_settings.face_colors;
}

GColor sidereel_pointer_color(void)
{
    return s_settings.pointer_color;
}

GColor sidereel_pointer_ink(void)
{
    return s_settings.pointer_ink;
}

GColor sidereel_reel_color(void)
{
    return s_settings.reel_color;
}

GColor sidereel_reel_ink(void)
{
    return s_settings.reel_ink;
}

void sidereel_set_layout(const char *layout)
{
    // in memory only. layout_string.c re-reads the string on the next access, so the parsed
    // blocks pick the change up without anything else being told
    strncpy(s_settings.layout, layout, sizeof(s_settings.layout) - 1);
    s_settings.layout[sizeof(s_settings.layout) - 1] = '\0';
}

void sidereel_set_panel_style(uint8_t style)
{
    s_settings.panel_style = style;
}

void sidereel_set_face_colors(bool on, GColor pointer, GColor pointer_ink, GColor reel, GColor reel_ink)
{
    s_settings.face_colors = on;
    s_settings.pointer_color = pointer;
    s_settings.pointer_ink = pointer_ink;
    s_settings.reel_color = reel;
    s_settings.reel_ink = reel_ink;
}

/**
 * @brief The number behind a goal pick, falling back to the first option if the index is past
 * the end of its table.
 *
 * @param table The goal values, in menu order.
 * @param count How many the table holds.
 * @param index The saved menu choice.
 * @return The goal value.
 */
static int goal_value(const int16_t *table, uint8_t count, uint8_t index)
{
    return table[index < count ? index : 0];
}

int gridlock_goal_steps(void)
{
    return goal_value(s_steps_goals, ARRAY_LENGTH(s_steps_goals), s_settings.goal_steps);
}

int gridlock_goal_calories(void)
{
    return goal_value(s_cal_goals, ARRAY_LENGTH(s_cal_goals), s_settings.goal_calories);
}

int gridlock_goal_distance_m(void)
{
    return goal_value(s_dist_goals_m, ARRAY_LENGTH(s_dist_goals_m), s_settings.goal_distance);
}

int gridlock_goal_sleep_min(void)
{
    return goal_value(s_sleep_goals_h, ARRAY_LENGTH(s_sleep_goals_h), s_settings.goal_sleep) * 60;
}

int gridlock_goal_active_min(void)
{
    return goal_value(s_active_goals, ARRAY_LENGTH(s_active_goals), s_settings.goal_active);
}

int gridlock_hr_limit(void)
{
    return goal_value(s_hr_limits, ARRAY_LENGTH(s_hr_limits), s_settings.goal_hr);
}

uint8_t gridlock_wind_unit(void)
{
    return s_settings.wind_unit;
}

uint8_t gridlock_week_start(void)
{
    return s_settings.week_start;
}

bool gridlock_distance_in_miles(void)
{
    return s_settings.distance_unit == DISTANCE_UNIT_MILES;
}

int gridlock_wind_value(int kmh)
{
    // the stored setting byte lines up with WindUnit (0 km/h, 1 mph, 2 kts, 3 m/s)
    return wind_from_kmh(kmh, (WindUnit) s_settings.wind_unit);
}

const char *gridlock_wind_unit_label(void)
{
    return wind_unit_label((WindUnit) s_settings.wind_unit);
}

const char *gridlock_temp_unit_label(void)
{
    return s_settings.temperature_unit ? "°F" : "°C";
}

const char *gridlock_goal_vibe_pick(void)
{
    return s_goal_vibe.pick;
}

const char *gridlock_goal_vibe_custom(void)
{
    return s_goal_vibe.custom;
}

/** @} */
