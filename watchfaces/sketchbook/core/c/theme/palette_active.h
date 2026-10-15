/**
 * @file palette_active.h
 * @brief Which palette a Sketchbook face should paint with right now.
 *
 * A face holds one palette setting and three ways to use it. Fixed keeps the chosen one. Shuffle
 * rotates through them all on a cadence. Day/Night/Quiet keeps the chosen one by day and swaps to a
 * different one after dark or during Quiet Time, each of those an override the wearer can turn off.
 *
 * This works out the theme index. The face still passes that index through `palette_for_theme` with
 * the daylight, so whichever theme is up still shows its own day or night colours.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * @addtogroup family-sketchbook
 * @{
 */

/// How the face uses its palette settings, in the picker's option order.
typedef enum
{
    PALETTE_MODE_FIXED = 0,  ///< The one chosen palette, always
    PALETTE_MODE_SHUFFLE,    ///< Rotate through them all on the interval
    PALETTE_MODE_DAYNIGHT,   ///< The day one, with a night and a Quiet Time override
    PALETTE_MODE_COUNT
} PaletteMode;

/// How often Shuffle moves on, in the picker's option order.
typedef enum
{
    SHUFFLE_EVERY_15M = 0,
    SHUFFLE_EVERY_30M,
    SHUFFLE_HOURLY,
    SHUFFLE_EVERY_3H,
    SHUFFLE_EVERY_6H,
    SHUFFLE_DAILY,
    SHUFFLE_INTERVAL_COUNT
} ShuffleInterval;

/// Everything the pick needs: the mode, the themes, and what is firing right now.
typedef struct
{
    PaletteMode mode;
    uint8_t day_theme;         ///< The fixed palette, and the daytime one in Day/Night/Quiet
    uint8_t shuffle_interval;  ///< A ShuffleInterval
    bool night_on;             ///< True after sunset right now
    bool use_night;            ///< True when a night override is set
    uint8_t night_theme;       ///< The palette to use after dark
    bool quiet_on;             ///< True while Quiet Time is on
    bool use_quiet;            ///< True when a Quiet Time override is set
    uint8_t quiet_theme;       ///< The palette to use during Quiet Time
} PalettePick;

/**
 * @brief The theme index to paint with now, from the settings and what is firing.
 *
 * The time comes in as the fields a struct tm carries, which Shuffle needs and the other modes
 * ignore, so the caller hands them straight from its broken-apart clock.
 *
 * @param pick The mode, the themes, and the day/night and Quiet Time readings.
 * @param year Years since 1900, as tm_year.
 * @param yday Days into the year, 0 to 365, as tm_yday.
 * @param hour The hour, 0 to 23, as tm_hour.
 * @param minute The minute, 0 to 59, as tm_min.
 * @return The theme index, which the face still pairs with the daylight in palette_for_theme.
 */
uint8_t sketchbook_active_theme(const PalettePick *pick, int year, int yday, int hour, int minute);

/** @} */
