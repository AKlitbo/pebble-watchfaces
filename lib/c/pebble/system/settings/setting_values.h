/**
 * @file setting_values.h
 * @brief Named values for the multi-choice settings.
 *
 * The wire format and Clay config encode these as bare integers. C branches read
 * them by name instead. The order mirrors each setting's config.js option list,
 * and the trailing *_COUNT sentinel feeds the schema's enum_count so the bound
 * can't drift from the values.
 *
 * @ingroup lib
 */
#pragma once

/** @addtogroup lib @{ */

/**
 * @brief SETTING_TIME_FORMAT choices.
 */
typedef enum
{
    TIME_FORMAT_SYSTEM      = 0,
    TIME_FORMAT_12H         = 1,  // 12 hour with a leading zero  06:30
    TIME_FORMAT_24H         = 2,
    TIME_FORMAT_BEATS       = 3,  // kept for the wire format. not offered in the config
    TIME_FORMAT_12H_NO_LEAD = 4,  // 12 hour without a leading zero  6:30
    TIME_FORMAT_COUNT
} TimeFormat;

/**
 * @brief SETTING_STEPS_MODE choices.
 */
typedef enum
{
    STEPS_MODE_STEPS = 0,
    STEPS_MODE_MILES = 1,
    STEPS_MODE_KM    = 2,
    STEPS_MODE_COUNT
} StepsMode;

/**
 * @brief SETTING_DISTANCE_UNIT choices. The standalone Distance panel picks its own unit
 * separate from the Steps panel's STEPS_MODE.
 */
typedef enum
{
    DISTANCE_UNIT_KM    = 0,
    DISTANCE_UNIT_MILES = 1,
    DISTANCE_UNIT_COUNT
} DistanceUnit;

/**
 * @brief Bluetooth connect/disconnect vibe pattern choices.
 */
typedef enum
{
    VIBE_NONE   = 0,
    VIBE_SHORT  = 1,
    VIBE_LONG   = 2,
    VIBE_DOUBLE = 3,
    VIBE_COUNT
} VibeChoice;

/**
 * @brief SETTING_BATTERY_DISPLAY choices: what the battery readout shows.
 */
typedef enum
{
    BATTERY_DISPLAY_BOTH    = 0,  // icon + percent
    BATTERY_DISPLAY_ICON    = 1,  // icon only
    BATTERY_DISPLAY_PERCENT = 2,  // percent only
    BATTERY_DISPLAY_COUNT
} BatteryDisplay;

/** @} */
