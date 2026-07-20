/**
 * @file settings_schema.h
 * @brief Ways to read the Gridlock settings schema.
 *
 * @ingroup gridlock_settings
 */
#pragma once
#include "system/settings/settings.h"
#include "engine/layouts.h" // GridlockBlock

/**
 * @addtogroup gridlock_settings
 * @{
 */

/**
 * @brief This face's storage schema (its own key and its own version).
 *
 * @return The schema to hand to settings_init.
 */
const SettingsSchema *gridlock_settings_schema(void);

/**
 * @brief How many placed blocks the current layout has.
 *
 * @return The block count (0 to GRIDLOCK_MAX_CELLS).
 */
uint8_t gridlock_block_count(void);

/**
 * @brief One placed block of the current layout.
 *
 * @param index The block index (0 to gridlock_block_count()-1).
 * @return The block, or NULL when the index is out of range.
 */
const GridlockBlock *gridlock_block(uint8_t index);

/**
 * @brief Checks if a module type is present anywhere in the current layout.
 *
 * @param type The module type.
 * @return True if the module is present.
 */
bool gridlock_has_module(uint8_t type);

/**
 * @brief One module's custom colours, plus a flag per channel saying whether it was set.
 *
 * A channel the user never picked keeps its flag false so the engine leaves it mono. The
 * subtitle caption follows the accent unless its own colour is set.
 */
typedef struct
{
    uint8_t accent_set : 1, value_set : 1, icon_set : 1, subtitle_set : 1;
    GColor  accent, value, icon, subtitle;
} GridlockCustomColor;

/**
 * @brief The custom colours chosen for a module type on the config page.
 *
 * Only meaningful under THEME_CUSTOM. Reads from the companion colour key.
 *
 * @param module The module type (a ModuleType value).
 * @return The module's colours and which channels were set.
 */
GridlockCustomColor gridlock_custom_color(uint8_t module);

/**
 * @brief Whether a module's header strip should be hidden (the no-header variant).
 *
 * Per module type and size, set from the "Header" toggle in the module appearance editor so a
 * module placed at two sizes can keep its header on one and drop it on the other. Read in
 * every theme (unlike the custom colours, which only apply under THEME_CUSTOM).
 *
 * @param module The module type (a ModuleType value).
 * @param size The placement size to read the flag for.
 * @return True to draw the module without its header.
 */
bool gridlock_module_headerless(uint8_t module, ModuleSize size);

/**
 * @brief Whether a module's outer border should be hidden (the frameless variant).
 *
 * Per module type and size, set from the "Border" toggle in the module appearance editor. Read
 * in every theme (unlike the custom colours, which only apply under THEME_CUSTOM).
 *
 * @param module The module type (a ModuleType value).
 * @param size The placement size to read the flag for.
 * @return True to draw the module without its outer outline.
 */
bool gridlock_module_borderless(uint8_t module, ModuleSize size);

/**
 * @brief The goal targets worked out (the saved menu choice turned into a real value).
 * @{
 */
int gridlock_goal_steps(void);
int gridlock_goal_calories(void);
int gridlock_goal_distance_m(void); ///< Today's distance goal in meters
int gridlock_goal_sleep_min(void);
int gridlock_goal_active_min(void);
int gridlock_hr_limit(void);
/** @brief The Goal Met Vibe pick string: empty (off), a sentinel (S/L/D/C), or a comma list of milliseconds. */
const char *gridlock_goal_vibe_pick(void);
/** @brief The user's own Goal Met Vibe pattern, a comma list of milliseconds, used when the pick is Custom. */
const char *gridlock_goal_vibe_custom(void);
uint8_t gridlock_wind_unit(void);
/// Number of Analog Face styles (0 classic round, then the rectangular and round dials)
#define GRIDLOCK_ANALOG_STYLE_COUNT 9
uint8_t gridlock_analog_style(void); ///< Analog Face Style pick (0 classic round, 1 to 6 other dials)
uint8_t gridlock_hourly_vibe(void); ///< VibeChoice buzzed at the top of each hour (VIBE_NONE is off)
uint8_t gridlock_week_start(void); ///< First day of the week for the calendar panels (0 Sunday, 1 Monday)
int gridlock_stock_poll_min(void); ///< Minutes between stock quote requests
int gridlock_calendar_poll_min(void); ///< Minutes between agenda requests

/**
 * @brief Which event reminders fire. The buzz pattern for each is a separate setting.
 */
typedef enum
{
    CAL_VIBE_MODE_NONE       = 0, ///< No event reminders
    CAL_VIBE_MODE_15_5_START = 1, ///< 15 min, 5 min, and on start
    CAL_VIBE_MODE_5_START    = 2, ///< 5 min and on start
    CAL_VIBE_MODE_START      = 3, ///< On start only
    CAL_VIBE_MODE_COUNT
} CalendarVibeMode;

uint8_t gridlock_calendar_vibe_mode(void);  ///< Which event reminders fire (a CalendarVibeMode)
uint8_t gridlock_calendar_vibe_15min(void); ///< VibeChoice for the 15-min-before reminder
uint8_t gridlock_calendar_vibe_5min(void);  ///< VibeChoice for the 5-min-before reminder
uint8_t gridlock_calendar_vibe_start(void); ///< VibeChoice for the on-start reminder
bool gridlock_distance_in_miles(void); ///< True when the Distance panel shows miles

/**
 * @brief Turns a km/h wind speed into whatever unit the user picked in the config.
 *
 * @param kmh The wind speed in km/h.
 * @return The same speed in the chosen unit.
 */
int gridlock_wind_value(int kmh);

/**
 * @brief The short label for the user's chosen wind unit.
 *
 * @return One of "KM/H" "MPH" "KTS" or "M/S".
 */
const char *gridlock_wind_unit_label(void);

/**
 * @brief The trailing unit label for the user's chosen temperature unit.
 *
 * @return "°C" or "°F".
 */
const char *gridlock_temp_unit_label(void);
/** @} */

/**
 * @brief The timezone offset in minutes for the given time zone index (0-3 for Time Zones 1-4).
 *
 * @param index The time zone index (0 to 3).
 * @return The offset in minutes.
 */
int16_t gridlock_time_zone_offset_minutes(uint8_t index);

/**
 * @brief The timezone name for the given time zone index.
 *
 * @param index The time zone index.
 * @return The name string.
 */
const char* gridlock_time_zone_name(uint8_t index);

/**
 * @brief Writes a clock time in whatever 12/24 hour style the user picked.
 *
 * The hour is 0 to 23. System default follows the watch, 12 hour comes with or without a
 * leading zero, and 24 hour is always two digits. This is the one place that decision
 * lives so the clock, the big date bar, and the sunrise and sunset panels all agree.
 *
 * @param out The buffer to write into.
 * @param n The size of the buffer.
 * @param hour24 The hour from 0 to 23.
 * @param minute The minute from 0 to 59.
 * @return True when the written time is a 12 hour one, so the caller can add AM or PM.
 */
bool gridlock_format_clock(char *out, size_t n, int hour24, int minute);

/**
 * @brief Whether the clock reads as 24 hour, following the same rule gridlock_format_clock
 * uses. Handy for a bare hour label that wants to match the clock style without a colon.
 *
 * @return True for a 24 hour clock, false for a 12 hour one.
 */
bool gridlock_clock_is_24h(void);

/**
 * @brief Sets the theme in memory only (it is not saved). The dev preview uses this to
 * force a theme without going through the config page.
 *
 * @param theme A GridlockTheme value.
 */
void gridlock_set_theme(uint8_t theme);

/**
 * @brief Sets the temperature unit in memory only (it is not saved). The dev harness uses this so
 * the seeded (Fahrenheit) fixtures render with the right unit without a config push.
 *
 * @param unit 0 for Celsius, 1 for Fahrenheit.
 */
void gridlock_set_temperature_unit(uint8_t unit);

/**
 * @brief Sets the analog face style in memory only (it is not saved). The dev harness uses this
 * to flip the Analog Clock dial without going through the config page.
 *
 * @param style An APPEARANCE_ANALOG_STYLE value (0 to GRIDLOCK_ANALOG_STYLE_COUNT - 1).
 */
void gridlock_set_analog_style(uint8_t style);

/**
 * @brief Sets the layout in memory only (it is not saved). The dev harness uses this to
 * force a layout without going through the config page.
 *
 * @param layout A LAYOUT wire string ("type,lt,lb,rt,rb;...").
 */
void gridlock_set_layout(const char *layout);

/**
 * @brief Sets the Time Zone 1 offset string in memory only (it is not saved). The dev
 * harness uses this so the panel header can read a generic name for screenshots.
 *
 * @param value An "offset,name" string (e.g. "0,Time Zone 1").
 */
void gridlock_set_time_zone_1(const char *value);

/**
 * @brief Sets the custom colours string in memory only (it is not saved). The dev harness
 * uses this to flip per-module colours and header/border on/off without the config page.
 *
 * @param value An APPEARANCE_CUSTOM_COLORS wire string. Either the "~3" sparse form ("~3", the
 *              5-char colour records, "|", then the 3-char flag records) or the "~" positional
 *              form (a 6-char record per module id), or "0" for none. An untagged string carries
 *              no records and leaves every module plain.
 */
void gridlock_set_custom_colors(const char *value);

/**
 * @brief Rebuilds the combined APPEARANCE_CUSTOM_COLORS wire string from the two persisted sections.
 *
 * The inverse of gridlock_set_custom_colors: used to seed the phone (outbound settings) with the
 * single "~3 colours | flags" string even though the watch stores the two halves separately.
 *
 * @param out Buffer that receives the combined string.
 * @param n Size of out.
 */
void gridlock_get_custom_colors(char *out, size_t n);

/**
 * @brief Sets the time format in memory only (it is not saved). The dev harness uses this
 * to force a clock format without going through the config page.
 *
 * @param format A TimeFormat value.
 */
void gridlock_set_time_format(uint8_t format);

/** @} */
