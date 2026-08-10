/**
 * @file settings_schema.h
 * @brief sidereel's settings schema and its face-only reads.
 *
 * It sits at version 1 with no migration behind it. The shared fields go through
 * settings_u8/settings_str as usual. The colour slots and the two widget choices are face-only,
 * so they get named accessors here instead.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "system/settings/settings.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

/**
 * @brief Gets the settings schema for this face.
 *
 * @return A pointer to the schema.
 */
const SettingsSchema *sidereel_settings_schema(void);

/**
 * @brief The panel layout as the drag builder's wire string ("module,row,col,w,h;...").
 *
 * Parsed into placed blocks by layout_string.c rather than read directly.
 *
 * @return The stored string, never NULL.
 */
const char *sidereel_layout(void);

/**
 * @brief The second time zone as its wire string ("<offset minutes>,<place name>").
 *
 * @return The stored string, never NULL.
 */
const char *sidereel_timezone_1(void);

/**
 * @brief The panel style pick, which is the corner radius every panel is framed with.
 *
 * @return A PANEL_STYLE_ choice.
 */
uint8_t sidereel_panel_style(void);

/**
 * @brief Whether the four colours below paint over the running theme's chrome.
 *
 * @return True to use them, false to leave every theme as it comes.
 */
bool sidereel_face_colors(void);

/**
 * @brief The hour pointer's fill.
 *
 * @return The picked colour.
 */
GColor sidereel_pointer_color(void);

/**
 * @brief The hour digits and status glyphs on the pointer.
 *
 * @return The picked colour.
 */
GColor sidereel_pointer_ink(void);

/**
 * @brief The reel panel and the perforated strip beside it.
 *
 * @return The picked colour.
 */
GColor sidereel_reel_color(void);

/**
 * @brief The minute digits on the reel.
 *
 * @return The picked colour.
 */
GColor sidereel_reel_ink(void);

/**
 * @brief Sets the layout in memory only (it is not saved). The dev harness uses this to force a
 * layout without going through the config page.
 *
 * @param layout A LAYOUT wire string ("module,row,col,w,h;...").
 */
void sidereel_set_layout(const char *layout);

/**
 * @brief Sets the panel style in memory only (it is not saved). The dev harness uses this to
 * shoot the same layout square and rounded.
 *
 * @param style A PANEL_STYLE_ choice.
 */
void sidereel_set_panel_style(uint8_t style);

/**
 * @brief Sets the four face colours and their switch in memory only (they are not saved). The
 * dev harness uses this to paint the reel and the pointer without a config push.
 *
 * @param on True to paint these over whichever theme is running.
 * @param pointer The pennant fill.
 * @param pointer_ink The hour digits and status glyphs on it.
 * @param reel The reel panel and the perforated strip.
 * @param reel_ink The minute digits.
 */
void sidereel_set_face_colors(bool on, GColor pointer, GColor pointer_ink, GColor reel, GColor reel_ink);

// --- what the shared panel bodies ask their face for ---
// declared under the names those files already call, so a panel from core needs no edit. the
// goals, units and week start come straight off this face's own settings, the same as gridlock
// answers them. only the clock and time zone shapes are face-specific, and those live in
// engine/grid_shim.c
int gridlock_goal_steps(void);
int gridlock_goal_calories(void);
int gridlock_goal_distance_m(void);
int gridlock_goal_sleep_min(void);
int gridlock_goal_active_min(void);
int gridlock_hr_limit(void);
uint8_t gridlock_wind_unit(void);
uint8_t gridlock_week_start(void);
bool gridlock_distance_in_miles(void);
int gridlock_wind_value(int kmh);
const char *gridlock_wind_unit_label(void);
const char *gridlock_temp_unit_label(void);
int16_t gridlock_time_zone_offset_minutes(uint8_t index);
const char *gridlock_time_zone_name(uint8_t index);
bool gridlock_clock_is_24h(void);
bool gridlock_format_clock(char *out, size_t n, int hour24, int minute);

// the Goal Met Vibe, read by the shared goal_vibe.c. the pick is a sentinel or a comma list of
// milliseconds, and the custom box holds the user's own list for when the pick is Custom
const char *gridlock_goal_vibe_pick(void);
const char *gridlock_goal_vibe_custom(void);

/** @} */
