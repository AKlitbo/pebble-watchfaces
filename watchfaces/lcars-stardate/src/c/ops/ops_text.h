/**
 * @file ops_text.h
 * @brief The value formatters behind the ops catalog, plus the two hooks whose word or glyph
 * moves with the reading. Each formatter matches the engine's text-slot signature, so the
 * catalog table binds straight to one.
 *
 * The shared lib already formats the heart rate and the step count, so those are not repeated
 * here. Everything below is a reading no other face on this frame shows.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-lcars
 * @{
 */

// --- system ---
void ops_text_battery(char *out, size_t n);

// --- health ---
void ops_text_calories(char *out, size_t n);
void ops_text_sleep(char *out, size_t n);
void ops_text_active(char *out, size_t n);

// --- moon ---
void ops_text_moon_pct(char *out, size_t n);
void ops_text_moon_phase(char *out, size_t n);
void ops_text_moon_next(char *out, size_t n);

/** @brief The phase glyph for right now, one of the eight in the ring. */
uint32_t ops_moon_icon(void);

/** @brief FULL IN or NEW IN, whichever the countdown is running to. */
const char *ops_moon_next_label(void);

// --- sun ---
// these read the sunrise and sunset the phone sent, so they need the five extra weather keys
// declared in the appinfo or they sit on "--" forever
void ops_text_sunrise(char *out, size_t n);
void ops_text_sunset(char *out, size_t n);
void ops_text_daylight(char *out, size_t n);
void ops_text_sun_next(char *out, size_t n);

/** @brief DAWN IN or DUSK IN, whichever event comes next. */
const char *ops_sun_next_label(void);

/** @brief The sunrise or sunset glyph, matching whichever event comes next. */
uint32_t ops_sun_next_icon(void);

// --- weather ---
/** @brief The current condition's glyph at ops size, since the SENSORS one is cut for a 24px box. */
uint32_t ops_wx_icon(void);

void ops_text_humidity(char *out, size_t n);
void ops_text_wind(char *out, size_t n);
void ops_text_uv(char *out, size_t n);
void ops_text_hilo(char *out, size_t n);

// --- calendar ---
void ops_text_julian(char *out, size_t n);
void ops_text_day_of_year(char *out, size_t n);
void ops_text_week(char *out, size_t n);

// --- other clocks ---
void ops_text_epoch(char *out, size_t n);
void ops_text_beats(char *out, size_t n);
void ops_text_zone_1(char *out, size_t n);

/** @brief The configured zone's own name, so the holder box reads LONDON rather than ZONE 1. */
const char *ops_zone_1_label(void);

/** @} */
