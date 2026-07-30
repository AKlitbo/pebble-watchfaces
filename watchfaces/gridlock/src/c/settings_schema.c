/**
 * @file settings_schema.c
 * @brief Gridlock settings schema.
 *
 * Settings are split into per-domain blobs (core, weather, health, clock, bluetooth, stocks)
 * plus the companion colour table, each under its own persist key so a domain grows on its own
 * without disturbing the rest. The layout is one compact string put together by the Clay layout
 * component. Each block is a chunk split off by a semicolon. Inside a chunk the values are split
 * by commas: "module,row,col,w,h". It gets parsed once into a small cache that the block
 * accessors read. Goals stay as plain selects.
 *
 * @ingroup gridlock_settings
 */
#include "settings_schema.h"
#include "persist_keys.h"
#include "system/settings/settings_catalog.h"
#include "system/settings/setting_values.h"
#include "units/wind.h"
#include "clock/nightsched.h"
#include "engine/layouts.h"
#include "engine/catalog.h"
#include "draw/header_fonts.h" // HEADER_FONT_COUNT for the header font enum bound

#include <pebble.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// --- persist keys and versions ---
//
// settings are split into small per-domain blobs, each under its own persist key with its own
// version byte. a domain grows on its own: append a field to its struct, bump only that
// domain's version, and an older (shorter) saved blob short-reads into the current struct so
// the new trailing field keeps its default while every existing field survives. never reorder,
// retype, or resize a field already in a struct. the static asserts below freeze each size so a
// stray change breaks the build instead of watches in the field. the persist keys themselves
// live in persist_keys.h so every key across the face sits in one place
#define GRIDLOCK_CORE_VERSION 2
#define GRIDLOCK_CORE_SIZE    131
// frozen size of the very first (v1) core blob: version + theme + layout[128], before
// header_font was appended. min_versioned_size must stay pinned to the v1 size so an older
// short blob still short-reads instead of being rejected and reset. bump this only if v1 is
// ever retired, never when appending a new trailing field
#define GRIDLOCK_CORE_V1_SIZE 130

// the per-module custom colours keep their own key so the table never crowds the core blob.
// read only when the theme is custom. the wire format is the sparse "~3 colours | flags"
// string. on the watch it splits into two persist keys (colours here, flags below) so the
// worst case never crowds one 256 byte key. this key holds only the colour section.
// NOTE this schema is version 2 with min_versioned_size == SIZE, which is safe ONLY because
// v1 to v2 was an in-buffer format change on a fixed 251 byte struct, not an appended field.
// if a field is ever appended here (v3), freeze the v2 size into a *_V1_SIZE-style constant
// for min_versioned_size like GRIDLOCK_CORE_V1_SIZE does, or every custom theme gets wiped
#define GRIDLOCK_CUSTOM_THEME_VERSION 2
#define GRIDLOCK_CUSTOM_THEME_SIZE    251
#define GRIDLOCK_CUSTOM_COLORS_LEN 250

// the flag section (per-module header/border) of the same custom string, in its own key so
// colours + flags each stay inside the 256 byte persist limit
#define GRIDLOCK_CUSTOM_FLAGS_VERSION 1
#define GRIDLOCK_CUSTOM_FLAGS_SIZE    251
#define GRIDLOCK_CUSTOM_FLAGS_LEN 250

#define GRIDLOCK_WEATHER_VERSION 1
#define GRIDLOCK_WEATHER_SIZE    3

#define GRIDLOCK_HEALTH_VERSION 1
#define GRIDLOCK_HEALTH_SIZE    9

// the Goal Met Vibe rides its own key, not the health blob: the pick and the custom pattern are
// short strings (the dropdown value is either a sentinel or a comma list of milliseconds), too big
// and too variable to sit in the fixed health struct. so they get a storage blob of their own
#define GRIDLOCK_GOAL_VIBE_VERSION    1
#define GRIDLOCK_GOAL_VIBE_PICK_LEN   64  // the longest preset dropdown value plus room to spare
#define GRIDLOCK_GOAL_VIBE_CUSTOM_LEN 122 // the config caps the input at 120 characters
#define GRIDLOCK_GOAL_VIBE_SIZE       (1 + GRIDLOCK_GOAL_VIBE_PICK_LEN + GRIDLOCK_GOAL_VIBE_CUSTOM_LEN)

#define GRIDLOCK_CLOCK_VERSION 3
#define GRIDLOCK_CLOCK_SIZE    52
// frozen size of the v1 clock blob, before hourly_vibe and week_start were appended.
// min_versioned_size stays pinned here so an older short blob still short-reads and the
// new fields keep their defaults
#define GRIDLOCK_CLOCK_V1_SIZE 50

#define GRIDLOCK_BLUETOOTH_VERSION 1
#define GRIDLOCK_BLUETOOTH_SIZE    4

#define GRIDLOCK_STOCKS_VERSION 1
#define GRIDLOCK_STOCKS_SIZE    2

// analog face style rides its own key so the picker can grow (more dials, per-style extras)
// without touching the core blob
#define GRIDLOCK_ANALOG_VERSION 1
#define GRIDLOCK_ANALOG_SIZE    2

#define GRIDLOCK_CALENDAR_VERSION 2
#define GRIDLOCK_CALENDAR_SIZE    6
// frozen size of the v1 calendar blob, before the event reminder fields were appended.
// min_versioned_size stays pinned here so an older short blob still short-reads and the new
// fields keep their defaults
#define GRIDLOCK_CALENDAR_V1_SIZE 2

// the night layout rides its own key rather than the core blob. core is already 131 bytes and a
// second 128-byte layout would take it to 259, past the 256 a persist key holds, and
// persist_write_data does not report that: it simply writes nothing, so every core setting would
// stop persisting at once. min_versioned_size is pinned to the v1 size from the first release so
// appending a field later cannot wipe a night layout in the field
#define GRIDLOCK_NIGHT_VERSION 1
#define GRIDLOCK_NIGHT_SIZE    132
#define GRIDLOCK_NIGHT_V1_SIZE 132

// the wire value for "no night layout". an empty string cannot say this: settings_apply_inbox
// skips an empty cstring rather than storing it, so a cleared grid would never reach the watch.
// the same sentinel the custom colours use for their own empty state
#define GRIDLOCK_NO_LAYOUT "0"

// four themes. 0 mono. 1 vibrant. 2 custom. 3 mono inverse (see theme.h GridlockTheme)
#define GRIDLOCK_THEME_COUNT 4

// default layout that matches the Clay config. one block per entry, "module,row,col,w,h"
#define GRIDLOCK_DEFAULT_LAYOUT "2,0,0,2,2;12,0,2,2,1;13,1,2,2,1;1,2,0,4,1;3,3,0,2,2;6,3,2,2,2"

/**
 * @addtogroup gridlock_settings
 * @{
 */

// --- the per-domain settings structs, each with the version byte first ---

/** @brief Core face settings: theme, the block layout string, and the header font choice. */
typedef struct GridlockCore
{
    uint8_t version;
    uint8_t theme;
    char    layout[128]; // "module,row,col,w,h;..." with one entry per placed block
    uint8_t header_font; // Header Font pick (index into the header_fonts table)
} GridlockCore;
_Static_assert(sizeof(GridlockCore) == GRIDLOCK_CORE_SIZE, "core blob size is frozen; fields append only");

/**
 * @brief The night layout and when to show it.
 *
 * Scalars first and the string last, so a field appended later lands after the layout rather than
 * shifting it.
 */
typedef struct GridlockNight
{
    uint8_t version;
    uint8_t mode;       // a NightSchedMode: off, follow the sun, or the fixed pair
    uint8_t start_slot; // half-hour slot 0..47, so 21:00 is 42
    uint8_t end_slot;
    char    layout[128];
} GridlockNight;
_Static_assert(sizeof(GridlockNight) == GRIDLOCK_NIGHT_SIZE, "night blob size is frozen; fields append only");

/** @brief Weather settings. */
typedef struct GridlockWeather
{
    uint8_t version;
    uint8_t temperature_unit;
    uint8_t wind_unit;
} GridlockWeather;
_Static_assert(sizeof(GridlockWeather) == GRIDLOCK_WEATHER_SIZE, "weather blob size is frozen; fields append only");

/** @brief Health settings: goals and the step and distance readouts. */
typedef struct GridlockHealth
{
    uint8_t version;
    uint8_t steps_mode;
    uint8_t distance_unit;
    // goals (stored as the menu choice and turned into a value by the tables below)
    uint8_t goal_steps;
    uint8_t goal_calories;
    uint8_t goal_sleep;
    uint8_t goal_active;
    uint8_t goal_hr;
    uint8_t goal_distance;
} GridlockHealth;
_Static_assert(sizeof(GridlockHealth) == GRIDLOCK_HEALTH_SIZE, "health blob size is frozen; fields append only");

/** @brief Clock settings: date and time formats and the extra time zone. */
typedef struct GridlockClock
{
    uint8_t version;
    char    date_format[16];
    uint8_t time_format;
    char    time_zone_offset_1[32]; // "offset,name"
    uint8_t hourly_vibe; // VibeChoice fired at the top of each hour (VIBE_NONE is off)
    uint8_t week_start;  // 0 starts the week on Sunday and 1 on Monday
} GridlockClock;
_Static_assert(sizeof(GridlockClock) == GRIDLOCK_CLOCK_SIZE, "clock blob size is frozen; fields append only");

/** @brief Bluetooth settings: the icon and the connect and disconnect vibes. */
typedef struct GridlockBluetooth
{
    uint8_t version;
    bool    bluetooth_icon;
    uint8_t vibe_connect;
    uint8_t vibe_disconnect;
} GridlockBluetooth;
_Static_assert(sizeof(GridlockBluetooth) == GRIDLOCK_BLUETOOTH_SIZE, "bluetooth blob size is frozen; fields append only");

/** @brief Stock settings. */
typedef struct GridlockStocks
{
    uint8_t version;
    uint8_t stock_poll; // menu choice into s_stock_poll_min, minutes between quote requests
} GridlockStocks;
_Static_assert(sizeof(GridlockStocks) == GRIDLOCK_STOCKS_SIZE, "stocks blob size is frozen; fields append only");

/** @brief Analog face settings: which dial an Analog Clock panel draws. */
typedef struct GridlockAnalog
{
    uint8_t version;
    uint8_t style; // Analog Face Style pick (0 classic round, 1..6 rectangular dials)
} GridlockAnalog;
_Static_assert(sizeof(GridlockAnalog) == GRIDLOCK_ANALOG_SIZE, "analog blob size is frozen; fields append only");

/** @brief Calendar settings: how often the watch pulls a fresh agenda from the phone. */
typedef struct GridlockCalendar
{
    uint8_t version;
    uint8_t calendar_poll; // menu choice into s_calendar_poll_min, minutes between agenda requests
    uint8_t vibe_15min;    // VibeChoice for the 15-min-before reminder
    uint8_t vibe_5min;     // VibeChoice for the 5-min-before reminder
    uint8_t vibe_start;    // VibeChoice for the on-start reminder
    uint8_t vibe_mode;     // which reminders fire (a CalendarVibeMode, CAL_VIBE_MODE_NONE is off)
} GridlockCalendar;
_Static_assert(sizeof(GridlockCalendar) == GRIDLOCK_CALENDAR_SIZE, "calendar blob size is frozen; fields append only");

/** @brief Goal Met Vibe: the dropdown pick and the custom pattern, both short strings. */
typedef struct GridlockGoalVibe
{
    uint8_t version;
    char    pick[GRIDLOCK_GOAL_VIBE_PICK_LEN];     // a sentinel (empty/S/L/D/C) or a comma list of milliseconds
    char    custom[GRIDLOCK_GOAL_VIBE_CUSTOM_LEN]; // the user's own comma list, used when the pick is Custom
} GridlockGoalVibe;
_Static_assert(sizeof(GridlockGoalVibe) == GRIDLOCK_GOAL_VIBE_SIZE, "goal vibe blob size is frozen; fields append only");

static GridlockCore      s_core;
static GridlockWeather   s_weather;
static GridlockHealth    s_health;
static GridlockClock     s_clock;
static GridlockBluetooth s_bluetooth;
static GridlockStocks    s_stocks;
static GridlockAnalog    s_analog;
static GridlockCalendar  s_calendar;
static GridlockGoalVibe  s_goal_vibe;
static GridlockNight     s_night;

// goal option values looked up by the saved menu choice
static const int16_t s_steps_goals[]   = {5000, 7500, 10000, 12500, 15000, 20000, 25000};
static const int16_t s_cal_goals[]     = {500, 1000, 1500, 2000, 2500, 3000};
static const int16_t s_sleep_goals_h[] = {6, 7, 8, 9, 10};
static const int16_t s_active_goals[]  = {15, 30, 45, 60, 90};
static const int16_t s_hr_limits[]     = {150, 160, 170, 180, 190, 200};
// distance goals in meters. the readout converts to the user's miles/km at draw time
static const int16_t s_dist_goals_m[]  = {2000, 3000, 5000, 8000, 10000, 15000, 20000};
// how often the watch asks the phone for fresh quotes, in minutes
static const int16_t s_stock_poll_min[] = {1, 5, 10, 15, 20, 30};
// how often the watch asks the phone for a fresh agenda, in minutes
static const int16_t s_calendar_poll_min[] = {5, 10, 15, 20, 30, 60};

// face-only enum field. rides its own message key and is read straight off its owning struct
#define FACE_ENUM(owner, member, key, count, dflt) \
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_##key, .type = SETTING_ENUM_U8, \
      .offset = offsetof(owner, member), .enum_count = (count), .default_num = (dflt) }

// --- core (key 1) ---
static const SettingField s_core_fields[] = {
    KNOWN_THEME(offsetof(GridlockCore, theme), GRIDLOCK_THEME_COUNT),
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT, .type = SETTING_CSTRING,
      .offset = offsetof(GridlockCore, layout), .size = sizeof(s_core.layout),
      .default_str = GRIDLOCK_DEFAULT_LAYOUT },
    // shared id (not FACE_ENUM) so the header font is readable with settings_u8(SETTING_HEADER_FONT)
    { .id = SETTING_HEADER_FONT, .message_key = &MESSAGE_KEY_APPEARANCE_HEADER_FONT,
      .type = SETTING_ENUM_U8, .offset = offsetof(GridlockCore, header_font),
      .enum_count = HEADER_FONT_COUNT, .default_num = 0 },
};

// --- night (key 12) ---
// the times are half-hour slots rather than an "HH:MM" string: one byte instead of six, no parse,
// and the framework's enum_count clamp sanitises them for free
static const SettingField s_night_fields[] = {
    FACE_ENUM(GridlockNight, mode, LAYOUT_NIGHT_MODE, NIGHT_SCHED_COUNT, NIGHT_SCHED_OFF),
    FACE_ENUM(GridlockNight, start_slot, LAYOUT_NIGHT_START, 48, 42), // 21:00
    FACE_ENUM(GridlockNight, end_slot, LAYOUT_NIGHT_END, 48, 14),     // 07:00
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_LAYOUT_NIGHT, .type = SETTING_CSTRING,
      .offset = offsetof(GridlockNight, layout), .size = sizeof(s_night.layout),
      .default_str = GRIDLOCK_NO_LAYOUT },
};

// --- weather (key 3) ---
static const SettingField s_weather_fields[] = {
    // temperature unit is an inline SETTING_ENUM_U8, not the shared KNOWN_ macro. the config
    // page shows a dropdown, so a select sends a cstring "0" or "1" on the wire rather than the
    // bool the shared macro expects
    { .id = SETTING_TEMPERATURE_UNIT, .message_key = &MESSAGE_KEY_WEATHER_TEMPERATURE_UNIT,
      .type = SETTING_ENUM_U8, .offset = offsetof(GridlockWeather, temperature_unit),
      .enum_count = 2, .affects_weather = true, .default_num = 0 },
    FACE_ENUM(GridlockWeather, wind_unit, WEATHER_WIND_UNIT, 4, 0),
};

// --- health (key 4) ---
static const SettingField s_health_fields[] = {
    KNOWN_STEPS_MODE(offsetof(GridlockHealth, steps_mode), STEPS_MODE_COUNT),
    KNOWN_DISTANCE_UNIT(offsetof(GridlockHealth, distance_unit), DISTANCE_UNIT_COUNT),
    FACE_ENUM(GridlockHealth, goal_steps, HEALTH_GOAL_STEPS, ARRAY_LENGTH(s_steps_goals), 2),
    FACE_ENUM(GridlockHealth, goal_calories, HEALTH_GOAL_CALORIES, ARRAY_LENGTH(s_cal_goals), 3),
    FACE_ENUM(GridlockHealth, goal_sleep, HEALTH_GOAL_SLEEP, ARRAY_LENGTH(s_sleep_goals_h), 2),
    FACE_ENUM(GridlockHealth, goal_active, HEALTH_GOAL_ACTIVE, ARRAY_LENGTH(s_active_goals), 1),
    FACE_ENUM(GridlockHealth, goal_hr, HEALTH_GOAL_HR, ARRAY_LENGTH(s_hr_limits), 3),
    FACE_ENUM(GridlockHealth, goal_distance, HEALTH_GOAL_DISTANCE, ARRAY_LENGTH(s_dist_goals_m), 2),
};

// --- goal vibe (key 11) ---
// both ride as plain strings. the dropdown value is the pick (a sentinel or a comma list of
// milliseconds), and the input holds the user's own comma list for when the pick is Custom
static const SettingField s_goal_vibe_fields[] = {
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_HEALTH_GOAL_VIBE, .type = SETTING_CSTRING,
      .offset = offsetof(GridlockGoalVibe, pick), .size = sizeof(s_goal_vibe.pick), .default_str = "" },
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_HEALTH_GOAL_VIBE_CUSTOM, .type = SETTING_CSTRING,
      .offset = offsetof(GridlockGoalVibe, custom), .size = sizeof(s_goal_vibe.custom), .default_str = "" },
};

// --- clock (key 5) ---
static const SettingField s_clock_fields[] = {
    KNOWN_DATE_FORMAT(offsetof(GridlockClock, date_format), "%a %b %d"),
    { .id = SETTING_TIME_FORMAT, .message_key = &MESSAGE_KEY_CLOCK_TIME_FORMAT,
      .type = SETTING_ENUM_U8, .offset = offsetof(GridlockClock, time_format),
      .enum_count = TIME_FORMAT_COUNT, .affects_layout = true, .default_num = 0 },
    { .id = SETTING_COUNT, .message_key = &MESSAGE_KEY_CLOCK_TIMEZONE_1, .type = SETTING_CSTRING,
      .offset = offsetof(GridlockClock, time_zone_offset_1), .size = sizeof(s_clock.time_zone_offset_1),
      .default_str = "60,London, England, GB" },
    FACE_ENUM(GridlockClock, hourly_vibe, CLOCK_HOURLY_VIBE, VIBE_COUNT, 0),
    FACE_ENUM(GridlockClock, week_start, CLOCK_WEEK_START, 2, 0),
};

// --- bluetooth (key 6) ---
static const SettingField s_bluetooth_fields[] = {
    KNOWN_BLUETOOTH_ICON(offsetof(GridlockBluetooth, bluetooth_icon)),
    KNOWN_BLUETOOTH_VIBE_CONNECT(offsetof(GridlockBluetooth, vibe_connect), VIBE_COUNT),
    KNOWN_BLUETOOTH_VIBE_DISCONNECT(offsetof(GridlockBluetooth, vibe_disconnect), VIBE_COUNT),
};

// --- stocks (key 7) ---
static const SettingField s_stocks_fields[] = {
    // default choice 5 is 30 minutes
    FACE_ENUM(GridlockStocks, stock_poll, STOCK_POLL, ARRAY_LENGTH(s_stock_poll_min), 5),
};

// --- analog face (key 9) ---
static const SettingField s_analog_fields[] = {
    // 0 classic round (default) then four rectangular and four other round faces
    FACE_ENUM(GridlockAnalog, style, APPEARANCE_ANALOG_STYLE, GRIDLOCK_ANALOG_STYLE_COUNT, 0),
};

// --- calendar (key 10) ---
static const SettingField s_calendar_fields[] = {
    // default choice 4 is 30 minutes
    FACE_ENUM(GridlockCalendar, calendar_poll, CALENDAR_POLL, ARRAY_LENGTH(s_calendar_poll_min), 4),
    // the mode picks which reminders fire and defaults to None so nobody gets buzzed until they
    // opt in. the per-reminder patterns keep sensible defaults for when they turn one on
    FACE_ENUM(GridlockCalendar, vibe_mode,  CALENDAR_VIBE_MODE,  CAL_VIBE_MODE_COUNT, 0),
    FACE_ENUM(GridlockCalendar, vibe_15min, CALENDAR_VIBE_15MIN, VIBE_COUNT, VIBE_SHORT),
    FACE_ENUM(GridlockCalendar, vibe_5min,  CALENDAR_VIBE_5MIN,  VIBE_COUNT, VIBE_DOUBLE),
    FACE_ENUM(GridlockCalendar, vibe_start, CALENDAR_VIBE_START, VIBE_COUNT, VIBE_LONG),
};

// the companion colour blob. an empty string (or "0") carries no records so the parser leaves
// every module plain. the schema declares no fields because the string arrives on its own key
// rather than through the settings table, which also means the framework's sanitize pass never
// looks at it: every read of colors bounds itself instead of trusting a terminator
typedef struct GridlockCustomTheme
{
    uint8_t version;
    char    colors[GRIDLOCK_CUSTOM_COLORS_LEN];
} GridlockCustomTheme;
_Static_assert(sizeof(GridlockCustomTheme) == GRIDLOCK_CUSTOM_THEME_SIZE, "custom theme blob size is frozen; fields append only");

static GridlockCustomTheme s_custom_theme;

// the flag section (per-module header/border) of the same custom string, in its own persist
// blob. "0"/empty is the no-flags state
typedef struct GridlockCustomFlags
{
    uint8_t version;
    char    flags[GRIDLOCK_CUSTOM_FLAGS_LEN];
} GridlockCustomFlags;
_Static_assert(sizeof(GridlockCustomFlags) == GRIDLOCK_CUSTOM_FLAGS_SIZE, "custom flags blob size is frozen; fields append only");

static GridlockCustomFlags s_custom_flags;

// APPEARANCE_CUSTOM_COLORS is not a framework field: the combined "~3 colours | flags" string can exceed a
// single 256 byte persist key, so appmessage.c hands it to gridlock_set_custom_colors (which splits
// it into the two blobs above) and reads it back via gridlock_get_custom_colors. both blobs still
// persist through their schemas below. they just carry no message field.

// the schemas are chained tail first so each .companion points at an already-defined link.
// each key loads independently, so the order carries no meaning beyond the forward reference
static const SettingsSchema s_calendar_schema = {
    .key = GRIDLOCK_CALENDAR_KEY,
    .version = GRIDLOCK_CALENDAR_VERSION,
    .min_versioned_size = GRIDLOCK_CALENDAR_V1_SIZE,
    .blob = &s_calendar,
    .blob_size = sizeof(s_calendar),
    .fields = s_calendar_fields,
    .field_count = ARRAY_LENGTH(s_calendar_fields),
    .migrate = NULL,
};

static const SettingsSchema s_analog_schema = {
    .key = GRIDLOCK_ANALOG_KEY,
    .version = GRIDLOCK_ANALOG_VERSION,
    .min_versioned_size = GRIDLOCK_ANALOG_SIZE,
    .blob = &s_analog,
    .blob_size = sizeof(s_analog),
    .fields = s_analog_fields,
    .field_count = ARRAY_LENGTH(s_analog_fields),
    .migrate = NULL,
    .companion = &s_calendar_schema,
};

static const SettingsSchema s_stocks_schema = {
    .key = GRIDLOCK_STOCKS_KEY,
    .version = GRIDLOCK_STOCKS_VERSION,
    .min_versioned_size = GRIDLOCK_STOCKS_SIZE,
    .blob = &s_stocks,
    .blob_size = sizeof(s_stocks),
    .fields = s_stocks_fields,
    .field_count = ARRAY_LENGTH(s_stocks_fields),
    .migrate = NULL,
    .companion = &s_analog_schema,
};

static const SettingsSchema s_bluetooth_schema = {
    .key = GRIDLOCK_BLUETOOTH_KEY,
    .version = GRIDLOCK_BLUETOOTH_VERSION,
    .min_versioned_size = GRIDLOCK_BLUETOOTH_SIZE,
    .blob = &s_bluetooth,
    .blob_size = sizeof(s_bluetooth),
    .fields = s_bluetooth_fields,
    .field_count = ARRAY_LENGTH(s_bluetooth_fields),
    .migrate = NULL,
    .companion = &s_stocks_schema,
};

static const SettingsSchema s_clock_schema = {
    .key = GRIDLOCK_CLOCK_KEY,
    .version = GRIDLOCK_CLOCK_VERSION,
    .min_versioned_size = GRIDLOCK_CLOCK_V1_SIZE,
    .blob = &s_clock,
    .blob_size = sizeof(s_clock),
    .fields = s_clock_fields,
    .field_count = ARRAY_LENGTH(s_clock_fields),
    .migrate = NULL,
    .companion = &s_bluetooth_schema,
};

static const SettingsSchema s_health_schema = {
    .key = GRIDLOCK_HEALTH_KEY,
    .version = GRIDLOCK_HEALTH_VERSION,
    .min_versioned_size = GRIDLOCK_HEALTH_SIZE,
    .blob = &s_health,
    .blob_size = sizeof(s_health),
    .fields = s_health_fields,
    .field_count = ARRAY_LENGTH(s_health_fields),
    .migrate = NULL,
    .companion = &s_clock_schema,
};

static const SettingsSchema s_weather_schema = {
    .key = GRIDLOCK_WEATHER_KEY,
    .version = GRIDLOCK_WEATHER_VERSION,
    .min_versioned_size = GRIDLOCK_WEATHER_SIZE,
    .blob = &s_weather,
    .blob_size = sizeof(s_weather),
    .fields = s_weather_fields,
    .field_count = ARRAY_LENGTH(s_weather_fields),
    .migrate = NULL,
    .companion = &s_health_schema,
};

// storage-only schemas (no message field): the APPEARANCE_CUSTOM_COLORS string is split into these two
// blobs by gridlock_set_custom_colors and read back by gridlock_get_custom_colors
static const SettingsSchema s_custom_flags_schema = {
    .key = GRIDLOCK_CUSTOM_FLAGS_KEY,
    .version = GRIDLOCK_CUSTOM_FLAGS_VERSION,
    .min_versioned_size = GRIDLOCK_CUSTOM_FLAGS_SIZE,
    .blob = &s_custom_flags,
    .blob_size = sizeof(s_custom_flags),
    .fields = NULL,
    .field_count = 0,
    .migrate = NULL,
    .companion = &s_weather_schema,
};

static const SettingsSchema s_custom_theme_schema = {
    .key = GRIDLOCK_CUSTOM_THEME_KEY,
    .version = GRIDLOCK_CUSTOM_THEME_VERSION,
    .min_versioned_size = GRIDLOCK_CUSTOM_THEME_SIZE,
    .blob = &s_custom_theme,
    .blob_size = sizeof(s_custom_theme),
    .fields = NULL,
    .field_count = 0,
    .migrate = NULL,
    .companion = &s_custom_flags_schema,
};

static const SettingsSchema s_goal_vibe_schema = {
    .key = GRIDLOCK_GOAL_VIBE_KEY,
    .version = GRIDLOCK_GOAL_VIBE_VERSION,
    .min_versioned_size = GRIDLOCK_GOAL_VIBE_SIZE,
    .blob = &s_goal_vibe,
    .blob_size = sizeof(s_goal_vibe),
    .fields = s_goal_vibe_fields,
    .field_count = ARRAY_LENGTH(s_goal_vibe_fields),
    .migrate = NULL,
    .companion = &s_custom_theme_schema,
};

// chained straight after core on purpose: settings_serialize walks the chain in order and stops
// at the first dict failure, so keeping both layouts at the front keeps the night one out of the
// tail that a squeezed outbox would drop
static const SettingsSchema s_night_schema = {
    .key = GRIDLOCK_NIGHT_KEY,
    .version = GRIDLOCK_NIGHT_VERSION,
    .min_versioned_size = GRIDLOCK_NIGHT_V1_SIZE,
    .blob = &s_night,
    .blob_size = sizeof(s_night),
    .fields = s_night_fields,
    .field_count = ARRAY_LENGTH(s_night_fields),
    .migrate = NULL,
    .companion = &s_goal_vibe_schema,
};

static const SettingsSchema s_core_schema = {
    .key = GRIDLOCK_CORE_KEY,
    .version = GRIDLOCK_CORE_VERSION,
    .min_versioned_size = GRIDLOCK_CORE_V1_SIZE,
    .blob = &s_core,
    .blob_size = sizeof(s_core),
    .fields = s_core_fields,
    .field_count = ARRAY_LENGTH(s_core_fields),
    .migrate = NULL,
    .companion = &s_night_schema,
};

const SettingsSchema *gridlock_settings_schema(void)
{
    return &s_core_schema;
}

// --- layout string parsing ---
//
// the layout is one block per entry joined with semicolons. an entry is "module,row,col,w,h":
// the module id then its top-left grid cell and size. it is a free grid, so a block can sit
// anywhere that fits and the watch places each one on its own with no fixed row shapes.

static GridlockBlock s_blocks[GRIDLOCK_MAX_CELLS];
static uint8_t s_block_count;
static char s_parsed_src[sizeof(s_core.layout)]; // last string we parsed so we can skip doing it twice
static bool s_night_active;                      // which of the two layouts the parser is reading

/**
 * @brief Reads a run of digits and moves the cursor past them.
 *
 * @param p The cursor (moved past the digits).
 * @return The value read (0 when there are no digits).
 */
static int parse_int(const char **p)
{
    int value = 0;
    while (**p >= '0' && **p <= '9')
    {
        value = value * 10 + (**p - '0');
        (*p)++;
    }

    return value;
}

/**
 * @brief Whether a layout string holds at least one placeable block.
 *
 * One test covers the three ways a layout can be nothing: the "0" sentinel, an empty string, and
 * whatever a corrupt blob left behind.
 *
 * @param layout The wire string.
 * @return True when at least one record names a real module.
 */
static bool layout_has_any_block(const char *layout)
{
    for (const char *p = layout; p && *p; )
    {
        int type = parse_int(&p);
        if (type > 0 && type < MOD_TYPE_COUNT)
        {
            return true;
        }

        while (*p && *p != ';')
        {
            p++;
        }
        if (*p == ';')
        {
            p++;
        }
    }

    return false;
}

/**
 * @brief The layout the block cache should be reading.
 *
 * The night one only wins while it is switched on and actually holds blocks, so a cleared or
 * corrupt night grid quietly falls back to the day layout rather than showing an empty screen.
 */
static const char *active_layout(void)
{
    return (s_night_active && layout_has_any_block(s_night.layout)) ? s_night.layout : s_core.layout;
}

/**
 * @brief Parses the layout string into the block cache, unless the cache is already up to date.
 */
static void ensure_parsed(void)
{
    const char *layout = active_layout();

    if (strncmp(s_parsed_src, layout, sizeof(s_parsed_src)) == 0)
    {
        return;
    }

    strncpy(s_parsed_src, layout, sizeof(s_parsed_src) - 1);
    s_parsed_src[sizeof(s_parsed_src) - 1] = '\0';

    s_block_count = 0;
    const char *p = layout;

    while (*p && s_block_count < GRIDLOCK_MAX_CELLS)
    {
        int vals[5] = {0, 0, 0, 0, 0};
        for (int i = 0; i < 5; i++)
        {
            while (*p == ' ')
            {
                p++;
            }

            vals[i] = parse_int(&p);
            if (*p == ',')
            {
                p++;
            }
        }

        // skip anything trailing up to the block separator
        while (*p && *p != ';')
        {
            p++;
        }

        if (*p == ';')
        {
            p++;
        }

        // vals = module,row,col,w,h. module 0 (MOD_EMPTY) or an unknown id is not a real block
        uint8_t module = vals[0] > 0 && vals[0] < MOD_TYPE_COUNT ? (uint8_t)vals[0] : MOD_EMPTY;
        if (module == MOD_EMPTY)
        {
            continue;
        }

        GridlockBlock *blk = &s_blocks[s_block_count++];
        blk->module = module;
        blk->col = vals[2] >= 2 ? 2 : 0;
        blk->w   = vals[3] >= 4 ? 4 : 2;
        blk->h   = vals[4] >= 2 ? 2 : 1;
        blk->row = vals[1] < GRIDLOCK_ROWS ? (uint8_t)vals[1] : 0;

        // keep the block inside the grid so a bad string can never place it off the bottom
        if (blk->row + blk->h > GRIDLOCK_ROWS)
        {
            blk->row = GRIDLOCK_ROWS - blk->h;
        }
    }
}

uint8_t gridlock_block_count(void)
{
    ensure_parsed();
    return s_block_count;
}

const GridlockBlock *gridlock_block(uint8_t index)
{
    ensure_parsed();
    return index < s_block_count ? &s_blocks[index] : NULL;
}

bool gridlock_has_module(uint8_t type)
{
    ensure_parsed();
    for (uint8_t i = 0; i < s_block_count; i++)
    {
        if (s_blocks[i].module == type)
        {
            return true;
        }
    }
    return false;
}

/** @brief Whether a layout string places a module, without disturbing the block cache. */
static bool layout_string_has_module(const char *layout, uint8_t type)
{
    for (const char *p = layout; p && *p; )
    {
        if (parse_int(&p) == type)
        {
            return true;
        }

        while (*p && *p != ';')
        {
            p++;
        }
        if (*p == ';')
        {
            p++;
        }
    }

    return false;
}

bool gridlock_has_module_either(uint8_t type)
{
    return gridlock_has_module(type)
        || layout_string_has_module(s_core.layout, type)
        || layout_string_has_module(s_night.layout, type);
}

uint8_t gridlock_night_mode(void)
{
    return s_night.mode;
}

int gridlock_night_start_min(void)
{
    return s_night.start_slot * 30;
}

int gridlock_night_end_min(void)
{
    return s_night.end_slot * 30;
}

bool gridlock_night_layout_set(void)
{
    return layout_has_any_block(s_night.layout);
}

bool gridlock_active_layout_is_night(void)
{
    return s_night_active;
}

void gridlock_set_active_layout(bool night)
{
    s_night_active = night;
}

void gridlock_set_night_layout(const char *layout)
{
    if (layout)
    {
        strncpy(s_night.layout, layout, sizeof(s_night.layout) - 1);
        s_night.layout[sizeof(s_night.layout) - 1] = '\0';
    }
}

void gridlock_set_night_mode(uint8_t mode)
{
    if (mode < NIGHT_SCHED_COUNT)
    {
        s_night.mode = mode;
    }
}

// --- custom colour string parsing ---
//
// one fixed 6-char record per module, laid out positionally and indexed by module id, so
// module m's record starts at offset MARKER + m * 6. a record is four colour chars then two
// flag chars: [accent][value][icon][subtitle][flagsLo][flagsHi]. a colour char is one base64
// symbol holding the palette index 0..63, or '.' (or any non-base64 char) to leave that
// channel mono.
//
// the two flag chars carry an 8-bit value split across two base64 symbols (6 bits each, 8
// used) so the header/border toggles can be kept PER SIZE rather than per module. the byte is
// Σ (headerless[s] ? 1<<s : 0) | (borderless[s] ? 1<<(4+s) : 0) for size index s (matches
// ModuleSize: 1x2=0, 2x2=1, 1x4=2, 2x4=3). flagsLo = alphabet[byte & 63], flagsHi =
// alphabet[(byte >> 6) & 63]. this makes the flag section opaque codes (not the old readable
// H/B/X letters) but keeps the packed table well under the persist ceiling.
//
// a v2 string is tagged with a leading '~' marker. an untagged string carries no records at all
// and leaves every module unset, so a blob in any older shape reads as plain. the "0" default is
// the empty state and a record that runs past the end of the string is unset. the colours only
// apply under THEME_CUSTOM but the header and border flags read in every theme.

// a v2 record is four colour chars then two flag chars
#define CUSTOM_RECORD_LEN 6

// leads a tagged string so we can tell it from an empty or unrecognized one. keep in step with
// src/pkjs/clay/builder/ts/theme/codec.ts
#define CUSTOM_FORMAT_MARKER '~'

// the url-safe base64 alphabet, one symbol per 64-colour palette index. keep in step with
// the JS encoder in src/pkjs/clay/builder/ts/theme/codec.ts: both map index 0..63 onto the same char
static const char s_color_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static GridlockCustomColor s_custom[MOD_TYPE_COUNT];
// one packed byte of per-size flags per module: bit s is "hide header" for size s, bit 4+s is
// "hide border". kept packed (rather than a bool per size) to stay small in RAM
static uint8_t s_flags[MOD_TYPE_COUNT];
// false until the persisted blobs have been parsed into s_custom/s_flags. gridlock_set_custom_colors
// clears it so the next read re-parses. cheaper than holding a copy of the string to diff against
static bool s_custom_parsed;

/**
 * @brief Turns one colour char into its palette index.
 *
 * @param ch The character.
 * @return 0 to 63, or -1 when it names no colour (the '.' sentinel or any junk stays mono).
 */
static int color_index(char ch)
{
    if (ch == '\0')
    {
        return -1;
    }
    const char *at = strchr(s_color_alphabet, ch);
    return at ? (int)(at - s_color_alphabet) : -1;
}

/**
 * @brief Reads one colour channel char into col.
 *
 * The alpha bits are forced opaque so a bad byte can never paint a panel invisible.
 *
 * @param ch The channel character.
 * @param col Filled in with the colour when the char names one.
 * @return True when a real colour was read.
 */
static bool read_channel(char ch, GColor *col)
{
    int idx = color_index(ch);
    if (idx < 0)
    {
        return false;
    }

    col->argb = (uint8_t)(idx | 0xC0);
    return true;
}

/**
 * @brief Unpacks the two flag chars (already color_index'd to lo/hi) into a module's per-size flags.
 *
 * The two chars pack into one byte: bit s is headerless[s], bit 4+s is borderless[s].
 *
 * @param id The module id.
 * @param lo The low flag nibble (or negative for an unreadable char, treated as 0).
 * @param hi The high flag nibble (or negative, treated as 0).
 */
static void unpack_flags(uint8_t id, int lo, int hi)
{
    s_flags[id] = (uint8_t)((lo < 0 ? 0 : lo) | ((hi < 0 ? 0 : hi) << 6));
}

/**
 * @brief Parses the custom colour string into the per-module cache, unless it is already up to date.
 */
static void ensure_custom_parsed(void)
{
    if (s_custom_parsed)
    {
        return;
    }
    s_custom_parsed = true;

    // recombine the two persisted sections into the single "~3 colours | flags" string the parser
    // understands. a lone v2/legacy blob in the colours key (empty flags key) recombines to itself
    // the precision bounds each read to its own buffer. these two blobs sit outside the settings
    // table so nothing NUL-checks them on load, and a plain %s would run off the end of a damaged
    // one and read whatever bss follows
    char combined[GRIDLOCK_CUSTOM_COLORS_LEN + GRIDLOCK_CUSTOM_FLAGS_LEN + 2];
    if (s_custom_flags.flags[0] != '\0')
    {
        snprintf(combined, sizeof(combined), "%.*s|%.*s",
                 (int)sizeof(s_custom_theme.colors), s_custom_theme.colors,
                 (int)sizeof(s_custom_flags.flags), s_custom_flags.flags);
    }
    else
    {
        snprintf(combined, sizeof(combined), "%.*s",
                 (int)sizeof(s_custom_theme.colors), s_custom_theme.colors);
    }

    memset(s_custom, 0, sizeof(s_custom));
    memset(s_flags, 0, sizeof(s_flags));

    const char *src = combined;

    // the empty string and the "0" default carry no records, so leave everything unset
    if (src[0] == '\0' || (src[0] == '0' && src[1] == '\0'))
    {
        return;
    }

    // ~3 sparse format: "~3" + 5-char colour records (id + four channels) then "|" then 3-char
    // flag records (id + two packed flag chars). colours are keyed by the group's primary id
    // (a grouped member inherits via resolve_theme_module), flags by each module's own id. only
    // customised modules appear, so the string stays small
    if (src[0] == CUSTOM_FORMAT_MARKER && src[1] == '3')
    {
        const char *body = src + 2;
        const char *bar = strchr(body, '|');
        size_t color_len = bar ? (size_t)(bar - body) : strlen(body);

        for (size_t off = 0; off + 5 <= color_len; off += 5)
        {
            int id = color_index(body[off]);
            if (id <= 0 || id >= MOD_TYPE_COUNT)
            {
                continue;
            }
            GridlockCustomColor color = {0};
            color.accent_set   = read_channel(body[off + 1], &color.accent);
            color.value_set    = read_channel(body[off + 2], &color.value);
            color.icon_set     = read_channel(body[off + 3], &color.icon);
            color.subtitle_set = read_channel(body[off + 4], &color.subtitle);
            s_custom[id] = color;
        }

        if (bar)
        {
            const char *flags = bar + 1;
            size_t flag_len = strlen(flags);
            for (size_t off = 0; off + 3 <= flag_len; off += 3)
            {
                int id = color_index(flags[off]);
                if (id <= 0 || id >= MOD_TYPE_COUNT)
                {
                    continue;
                }
                unpack_flags((uint8_t)id, color_index(flags[off + 1]), color_index(flags[off + 2]));
            }
        }
        return;
    }

    // a v2 string is tagged with the marker and packs the flags per size. anything untagged is
    // unrecognized (an old blob or stray data), so carries no records and leaves everything unset
    if (src[0] != CUSTOM_FORMAT_MARKER)
    {
        return;
    }
    const char *data = src + 1; // step past the marker
    size_t len = strlen(data);

    // each module owns a fixed CUSTOM_RECORD_LEN record at offset module * CUSTOM_RECORD_LEN.
    // module 0 is MOD_EMPTY so its slot is skipped. a record that runs past the end of the string
    // is missing (the "0" default or a short blob), and so is every record after it
    for (uint8_t module = 1; module < MOD_TYPE_COUNT; module++)
    {
        size_t base = (size_t)module * CUSTOM_RECORD_LEN;
        if (base + CUSTOM_RECORD_LEN > len)
        {
            break;
        }

        GridlockCustomColor color = {0};
        color.accent_set   = read_channel(data[base + 0], &color.accent);
        color.value_set    = read_channel(data[base + 1], &color.value);
        color.icon_set     = read_channel(data[base + 2], &color.icon);
        color.subtitle_set = read_channel(data[base + 3], &color.subtitle);
        s_custom[module] = color;

        unpack_flags(module, color_index(data[base + 4]), color_index(data[base + 5]));
    }
}

GridlockCustomColor gridlock_custom_color(uint8_t module)
{
    ensure_custom_parsed();
    if (module < MOD_TYPE_COUNT)
    {
        return s_custom[module];
    }
    return (GridlockCustomColor){0};
}

bool gridlock_module_headerless(uint8_t module, ModuleSize size)
{
    ensure_custom_parsed();
    if (module < MOD_TYPE_COUNT && size < MSIZE_COUNT)
    {
        return (s_flags[module] & (1 << size)) != 0;
    }
    return false;
}

bool gridlock_module_borderless(uint8_t module, ModuleSize size)
{
    ensure_custom_parsed();
    if (module < MOD_TYPE_COUNT && size < MSIZE_COUNT)
    {
        return (s_flags[module] & (1 << (4 + size))) != 0;
    }
    return false;
}

// --- goals ---

/**
 * @brief Looks up a goal value by its saved menu choice, kept inside the table.
 *
 * @param table The table of option values.
 * @param count How long the table is.
 * @param index The saved menu choice.
 * @return The value found.
 */
static int goal_value(const int16_t *table, uint8_t count, uint8_t index)
{
    return table[index < count ? index : 0];
}

int gridlock_goal_steps(void)
{
    return goal_value(s_steps_goals, ARRAY_LENGTH(s_steps_goals), s_health.goal_steps);
}

int gridlock_goal_calories(void)
{
    return goal_value(s_cal_goals, ARRAY_LENGTH(s_cal_goals), s_health.goal_calories);
}

int gridlock_goal_distance_m(void)
{
    return goal_value(s_dist_goals_m, ARRAY_LENGTH(s_dist_goals_m), s_health.goal_distance);
}

int gridlock_goal_sleep_min(void)
{
    return goal_value(s_sleep_goals_h, ARRAY_LENGTH(s_sleep_goals_h), s_health.goal_sleep) * 60;
}

int gridlock_goal_active_min(void)
{
    return goal_value(s_active_goals, ARRAY_LENGTH(s_active_goals), s_health.goal_active);
}

int gridlock_hr_limit(void)
{
    return goal_value(s_hr_limits, ARRAY_LENGTH(s_hr_limits), s_health.goal_hr);
}

const char *gridlock_goal_vibe_pick(void)
{
    return s_goal_vibe.pick;
}

const char *gridlock_goal_vibe_custom(void)
{
    return s_goal_vibe.custom;
}

uint8_t gridlock_wind_unit(void)
{
    return s_weather.wind_unit;
}

uint8_t gridlock_analog_style(void)
{
    return s_analog.style;
}

uint8_t gridlock_hourly_vibe(void)
{
    return s_clock.hourly_vibe;
}

uint8_t gridlock_week_start(void)
{
    return s_clock.week_start;
}

int gridlock_stock_poll_min(void)
{
    return goal_value(s_stock_poll_min, ARRAY_LENGTH(s_stock_poll_min), s_stocks.stock_poll);
}

int gridlock_calendar_poll_min(void)
{
    return goal_value(s_calendar_poll_min, ARRAY_LENGTH(s_calendar_poll_min), s_calendar.calendar_poll);
}

uint8_t gridlock_calendar_vibe_mode(void)
{
    return s_calendar.vibe_mode;
}

uint8_t gridlock_calendar_vibe_15min(void)
{
    return s_calendar.vibe_15min;
}

uint8_t gridlock_calendar_vibe_5min(void)
{
    return s_calendar.vibe_5min;
}

uint8_t gridlock_calendar_vibe_start(void)
{
    return s_calendar.vibe_start;
}

bool gridlock_distance_in_miles(void)
{
    return s_health.distance_unit == DISTANCE_UNIT_MILES;
}

int gridlock_wind_value(int kmh)
{
    // the stored setting byte lines up with WindUnit (0 km/h, 1 mph, 2 kts, 3 m/s)
    return wind_from_kmh(kmh, (WindUnit) s_weather.wind_unit);
}

const char *gridlock_wind_unit_label(void)
{
    return wind_unit_label((WindUnit) s_weather.wind_unit);
}

const char *gridlock_temp_unit_label(void)
{
    return s_weather.temperature_unit ? "°F" : "°C";
}

bool gridlock_format_clock(char *out, size_t n, int hour24, int minute)
{
    // keep the numbers in range so the formatted string can never overrun
    if (hour24 < 0 || hour24 > 23) hour24 = 0;
    if (minute < 0 || minute > 59) minute = 0;

    uint8_t fmt = s_clock.time_format;

    // one source of truth for the 24h decision, shared with gridlock_clock_is_24h
    bool h24 = gridlock_clock_is_24h();

    if (h24)
    {
        snprintf(out, n, "%02d:%02d", hour24, minute);
        return false;
    }

    int h12 = hour24 % 12;
    if (h12 == 0)
    {
        h12 = 12;
    }

    if (fmt == TIME_FORMAT_12H_NO_LEAD)
    {
        snprintf(out, n, "%d:%02d", h12, minute);
    }
    else
    {
        snprintf(out, n, "%02d:%02d", h12, minute);
    }

    return true;
}

bool gridlock_clock_is_24h(void)
{
    uint8_t fmt = s_clock.time_format;

    if (fmt == TIME_FORMAT_24H)
    {
        return true;
    }
    if (fmt == TIME_FORMAT_12H || fmt == TIME_FORMAT_12H_NO_LEAD)
    {
        return false;
    }
    return clock_is_24h_style();
}

void gridlock_set_theme(uint8_t theme)
{
    s_core.theme = theme;
}

void gridlock_set_temperature_unit(uint8_t unit)
{
    s_weather.temperature_unit = unit;
}

void gridlock_set_analog_style(uint8_t style)
{
    s_analog.style = style;
}

void gridlock_set_layout(const char *layout)
{
    // copy into the in-memory layout. ensure_parsed re-reads it on the next access so the
    // row cache picks the change up on its own
    strncpy(s_core.layout, layout, sizeof(s_core.layout) - 1);
    s_core.layout[sizeof(s_core.layout) - 1] = '\0';
}

void gridlock_set_time_zone_1(const char *value)
{
    strncpy(s_clock.time_zone_offset_1, value, sizeof(s_clock.time_zone_offset_1) - 1);
    s_clock.time_zone_offset_1[sizeof(s_clock.time_zone_offset_1) - 1] = '\0';
}

void gridlock_set_custom_colors(const char *value)
{
    // the combined "~3 colours | flags" string can exceed one 256 byte persist key, so split it
    // into the two blobs at the '|'. no '|' (a v2/legacy blob, or colour-only) keeps it all in the
    // colours key with empty flags. ensure_custom_parsed recombines + re-parses on the next access
    if (!value)
    {
        value = "";
    }

    const char *bar = strchr(value, '|');
    size_t colors_len = bar ? (size_t)(bar - value) : strlen(value);
    if (colors_len >= sizeof(s_custom_theme.colors))
    {
        colors_len = sizeof(s_custom_theme.colors) - 1;
    }
    memcpy(s_custom_theme.colors, value, colors_len);
    s_custom_theme.colors[colors_len] = '\0';

    const char *flags = bar ? bar + 1 : "";
    strncpy(s_custom_flags.flags, flags, sizeof(s_custom_flags.flags) - 1);
    s_custom_flags.flags[sizeof(s_custom_flags.flags) - 1] = '\0';

    // the blobs changed, so the parsed cache is stale. the next read re-parses
    s_custom_parsed = false;
}

void gridlock_get_custom_colors(char *out, size_t n)
{
    // rebuild the single interchange string from the two blobs for the outbound settings seed
    if (n == 0)
    {
        return;
    }
    // same bounded reads as ensure_custom_parsed: neither blob is NUL-checked on load
    if (s_custom_flags.flags[0] != '\0')
    {
        snprintf(out, n, "%.*s|%.*s",
                 (int)sizeof(s_custom_theme.colors), s_custom_theme.colors,
                 (int)sizeof(s_custom_flags.flags), s_custom_flags.flags);
    }
    else
    {
        snprintf(out, n, "%.*s",
                 (int)sizeof(s_custom_theme.colors), s_custom_theme.colors);
    }
}

void gridlock_set_time_format(uint8_t format)
{
    s_clock.time_format = format;
}

int16_t gridlock_time_zone_offset_minutes(uint8_t index)
{
    if (index == 0)
    {
        return atoi(s_clock.time_zone_offset_1);
    }
    return 0;
}

const char* gridlock_time_zone_name(uint8_t index)
{
    if (index == 0)
    {
        const char *comma = strchr(s_clock.time_zone_offset_1, ',');
        return comma ? (comma + 1) : "TZ";
    }
    return "TZ";
}

/** @} */
