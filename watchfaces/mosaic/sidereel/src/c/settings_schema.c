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
#include "mosaic/layout_places.h"
#include "units/wind.h"
#include "clock/nightsched.h"
#include "role/role.h"
#include "layout/layout_string.h"

#include <stddef.h>
#include <string.h>

#define SIDEREEL_SETTINGS_VERSION 1
#define SIDEREEL_GOAL_VIBE_VERSION 1
#define SIDEREEL_NIGHT_VERSION 1
#define SIDEREEL_QUIET_VERSION 1

#define SIDEREEL_NIGHT_SIZE (4 + SIDEREEL_LAYOUT_LEN)
#define SIDEREEL_QUIET_SIZE (1 + SIDEREEL_LAYOUT_LEN)

// the wire value for "no layout here". settings_apply_inbox keeps an empty string only for a
// setting whose default is empty, and this is the default, so an empty one would leave the old
// grid in place
#define SIDEREEL_NO_LAYOUT "0"

// the Goal Met Vibe strings, sized as the catalogue's KNOWN_GOAL_VIBE entries write them: the longest
// pick plus room to spare, and the 120 character cap on the custom box with its end
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
 * get added to the end from here. The schema's first size stays at the 146 bytes the first release
 * saved, so a save from before an append still loads with the new field on its default.
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
    // the goals the health panels measure against, each an index into the framework's goals.h
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
    // on, the reel jumps to each new minute rather than scrolling to it
    bool    reduce_motion;
} SidereelSettings;

static SidereelSettings s_settings;

/**
 * @brief The night layout and when to show it.
 *
 * Scalars first and the string last, so a field appended later lands after the layout rather than
 * shifting it.
 */
typedef struct SidereelNight
{
    uint8_t version;
    uint8_t mode;       // a NightSchedMode: off, follow the sun, or the fixed pair
    uint8_t start_slot; // half-hour slot 0..47, so 21:00 is 42
    uint8_t end_slot;
    char    layout[SIDEREEL_LAYOUT_LEN];
} SidereelNight;
_Static_assert(sizeof(SidereelNight) == SIDEREEL_NIGHT_SIZE, "night blob size is frozen; fields append only");

static SidereelNight s_night;

/**
 * @brief The layout that takes over while Quiet Time is on.
 *
 * There is no mode beside it the way night has one. Leaving the layout unassigned is the off
 * switch, which is the same test that already decides whether the night layout counts.
 */
typedef struct SidereelQuiet
{
    uint8_t version;
    char    layout[SIDEREEL_LAYOUT_LEN];
} SidereelQuiet;
_Static_assert(sizeof(SidereelQuiet) == SIDEREEL_QUIET_SIZE, "quiet blob size is frozen; fields append only");

static SidereelQuiet s_quiet;

// a LayoutRole: which of the three the layout reader is handing out
static uint8_t s_active_role;

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
    KNOWN_DATE_FORMAT(SidereelSettings, date_format),
    KNOWN_STEPS_MODE(offsetof(SidereelSettings, steps_mode)),
    KNOWN_TIME_FORMAT(offsetof(SidereelSettings, time_format)),
    KNOWN_HOURLY_VIBE(offsetof(SidereelSettings, hourly_vibe)),
    KNOWN_BLUETOOTH_ICON(offsetof(SidereelSettings, bluetooth_icon)),
    KNOWN_QUIET_TIME_ICON(offsetof(SidereelSettings, quiet_time_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(SidereelSettings, vibe_connect)),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(SidereelSettings, vibe_disconnect)),

    // the whole panel layout rides one string the drag builder writes, so a placement is a
    // block with its own row and height rather than a fixed slot with a size flag beside it
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelSettings, layout), .size = sizeof(s_settings.layout),
      .default_str = SIDEREEL_DEFAULT_LAYOUT, .affects_layout = true },


    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_CLOCK_TIMEZONE_1, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelSettings, time_zone_1), .size = sizeof(s_settings.time_zone_1),
      .default_str = "" },

    // the goals every health panel measures against, and the units and week start the shared
    // panel bodies read. the goals are the catalogue's entries, so the panels and vibe_goal read
    // them by setting id
    KNOWN_DISTANCE_UNIT(offsetof(SidereelSettings, distance_unit)),
    KNOWN_GOAL_STEPS(offsetof(SidereelSettings, goal_steps)),
    KNOWN_GOAL_CALORIES(offsetof(SidereelSettings, goal_calories)),
    KNOWN_GOAL_SLEEP(offsetof(SidereelSettings, goal_sleep)),
    KNOWN_GOAL_ACTIVE(offsetof(SidereelSettings, goal_active)),
    KNOWN_GOAL_HR(offsetof(SidereelSettings, goal_hr)),
    KNOWN_GOAL_DISTANCE(offsetof(SidereelSettings, goal_distance)),
    KNOWN_WIND_UNIT(offsetof(SidereelSettings, wind_unit)),
    FACE_ENUM(week_start,    CLOCK_WEEK_START,     2, 0),

    // how the face is framed. the page's header font count is held to the header font table, so a
    // choice the page offers always has a font
    FACE_ENUM(panel_style, APPEARANCE_PANEL_STYLE, PANEL_STYLE_COUNT, 0),
    FACE_ENUM(header_font, APPEARANCE_HEADER_FONT, SETTINGS_COUNT_CHECKED(APPEARANCE_HEADER_FONT, HEADER_FONT_COUNT),
              SETTINGS_DEFAULT(APPEARANCE_HEADER_FONT)),

    // the four face colours, and their switch. they start on the Vibrant row's chrome, so
    // turning them on lands somewhere other than mono before a single pick
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_APPEARANCE_FACE_COLORS,
      .type = SETTING_BOOL, .offset = offsetof(SidereelSettings, face_colors), .default_num = 0 },
    FACE_COLOR(pointer_color, APPEARANCE_POINTER_COLOR, 0xFF0000),
    FACE_COLOR(pointer_ink,   APPEARANCE_POINTER_INK,   0xFFFFFF),
    FACE_COLOR(reel_color,    APPEARANCE_REEL_COLOR,    0xFFFFFF),
    FACE_COLOR(reel_ink,      APPEARANCE_REEL_INK,      0x000000),

    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_APPEARANCE_REDUCE_MOTION,
      .type = SETTING_BOOL, .offset = offsetof(SidereelSettings, reduce_motion), .default_num = 0 },
};

// --- goal vibe (key 4) ---
// both ride as plain strings. the dropdown value is the pick (a marker or a comma list of
// milliseconds), and the input holds the user's own comma list for when the pick is Custom
static const SettingField s_goal_vibe_fields[] = {
    KNOWN_GOAL_VIBE(SidereelGoalVibe, pick),
    KNOWN_GOAL_VIBE_CUSTOM(SidereelGoalVibe, custom),
};

// --- night (key 5) ---
// the times are half-hour slots rather than an "HH:MM" string: one byte instead of six, no parse,
// and the framework's enum_count clamp sanitises them for free
static const SettingField s_night_fields[] = {
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_NIGHT_MODE, .type = SETTING_ENUM_U8,
      .offset = offsetof(SidereelNight, mode), .enum_count = NIGHT_SCHED_COUNT,
      .affects_layout = true, .default_num = NIGHT_SCHED_OFF },
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_NIGHT_START, .type = SETTING_ENUM_U8,
      .offset = offsetof(SidereelNight, start_slot), .enum_count = 48,
      .affects_layout = true, .default_num = 42 }, // 21:00
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_NIGHT_END, .type = SETTING_ENUM_U8,
      .offset = offsetof(SidereelNight, end_slot), .enum_count = 48,
      .affects_layout = true, .default_num = 14 },  // 07:00
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_NIGHT, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelNight, layout), .size = sizeof(s_night.layout),
      .default_str = SIDEREEL_NO_LAYOUT, .affects_layout = true },
};

// --- quiet (key 6) ---
static const SettingField s_quiet_fields[] = {
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_QUIET, .type = SETTING_CSTRING,
      .offset = offsetof(SidereelQuiet, layout), .size = sizeof(s_quiet.layout),
      .default_str = SIDEREEL_NO_LAYOUT, .affects_layout = true },
};

static const SettingsSchema s_goal_vibe_schema = {
    .key = SIDEREEL_GOAL_VIBE_KEY,
    .version = SIDEREEL_GOAL_VIBE_VERSION,
    SETTINGS_BLOB(s_goal_vibe, 187),
    .fields = s_goal_vibe_fields,
    .field_count = ARRAY_LENGTH(s_goal_vibe_fields),
    .migrate = NULL,
    .companion = &sidereel_custom_theme_schema,  // the packed appearance string, in its own two slots
};

// chained straight after the main blob: settings_serialize walks the chain in order and stops at
// the first dict failure, so keeping the layouts at the front keeps them out of the tail that a
// squeezed outbox would drop
static const SettingsSchema s_quiet_schema = {
    .key = SIDEREEL_QUIET_KEY,
    .version = SIDEREEL_QUIET_VERSION,
    SETTINGS_BLOB(s_quiet, 65),
    .fields = s_quiet_fields,
    .field_count = ARRAY_LENGTH(s_quiet_fields),
    .migrate = NULL,
    .companion = &s_goal_vibe_schema,
};

static const SettingsSchema s_night_schema = {
    .key = SIDEREEL_NIGHT_KEY,
    .version = SIDEREEL_NIGHT_VERSION,
    SETTINGS_BLOB(s_night, 68),
    .fields = s_night_fields,
    .field_count = ARRAY_LENGTH(s_night_fields),
    .migrate = NULL,
    .companion = &s_quiet_schema,
};

static const SettingsSchema s_schema = {
    .key = SIDEREEL_SETTINGS_KEY,
    .version = SIDEREEL_SETTINGS_VERSION,
    SETTINGS_BLOB(s_settings, 146),
    .fields = s_fields,
    .field_count = ARRAY_LENGTH(s_fields),
    .migrate = NULL,  // v1 is the only shape a stored blob can have, so nothing to bring forward
    // the chain runs on through the two alternate layouts to the goal vibe, which carries the
    // packed appearance string behind it
    .companion = &s_night_schema,
};

const SettingsSchema *sidereel_settings_schema(void)
{
    return &s_schema;
}

const char *sidereel_layout(void)
{
    // an alternate layout only wins while it actually holds blocks, so a cleared or corrupt grid
    // quietly falls back to the day one rather than showing an empty screen. layout_string.c
    // re-reads this and notices the string changed, so nothing else has to be told about a swap
    if (s_active_role == ROLE_QUIET && layout_has_any_block(s_quiet.layout, MOD_TYPE_COUNT))
    {
        return s_quiet.layout;
    }

    if (s_active_role == ROLE_NIGHT && layout_has_any_block(s_night.layout, MOD_TYPE_COUNT))
    {
        return s_night.layout;
    }

    return s_settings.layout;
}

uint8_t sidereel_night_mode(void)
{
    return s_night.mode;
}

int sidereel_night_start_min(void)
{
    return s_night.start_slot * 30;
}

int sidereel_night_end_min(void)
{
    return s_night.end_slot * 30;
}

bool sidereel_night_layout_set(void)
{
    return layout_has_any_block(s_night.layout, MOD_TYPE_COUNT);
}

bool sidereel_quiet_layout_set(void)
{
    return layout_has_any_block(s_quiet.layout, MOD_TYPE_COUNT);
}

bool mosaic_places_module(uint8_t type)
{
    return layout_places_module(s_settings.layout, type) || layout_places_module(s_night.layout, type) ||
           layout_places_module(s_quiet.layout, type);
}

uint8_t sidereel_active_layout_role(void)
{
    return s_active_role;
}

void sidereel_set_active_layout_role(uint8_t role)
{
    if (role < ROLE_COUNT)
    {
        s_active_role = role;
    }
}

const char *sidereel_timezone_1(void)
{
    return s_settings.time_zone_1;
}

uint8_t sidereel_panel_style(void)
{
    return s_settings.panel_style;
}

uint8_t sidereel_header_font(void)
{
    return s_settings.header_font;
}

bool sidereel_face_colors(void)
{
    return s_settings.face_colors;
}

bool sidereel_reduce_motion(void)
{
    return s_settings.reduce_motion;
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

uint8_t gridlock_hrv_count(void)
{
    return 0;
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

/** @} */
