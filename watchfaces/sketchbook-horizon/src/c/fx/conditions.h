/**
 * @file conditions.h
 * @brief The phone's condition token, read into a condition and a sky.
 *
 * Every scene agrees on what each condition does to the sky: how much cloud, how much of the disc
 * it leaves, what falls and how hard, whether a wash goes over the scene and whether a bolt forks
 * out of it. So that table lives here, indexed by the framework's WxCond.
 *
 * What a scene does with the condition beyond the sky is its own, and it is not a field on
 * SketchbookSky, which only says what the sky looks like. Ridgeline caps its peaks in snow and
 * Treeline lies snow along its firs, and both ask sketchbook_cond_snows, so the two always agree on
 * when snow is lying. Shoreline roughens its water on the wet and stormy conditions and keeps that
 * rule in its own file, since chop follows wind where snow follows cold.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "weather/wx_cond.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/** @brief How much cloud is in the sky. */
typedef enum
{
    CLOUDS_NONE,      ///< Open sky
    CLOUDS_DRIFTING,  ///< One cloud, parked over the disc
    CLOUDS_OVERCAST   ///< A band of puffs running right across the sky
} CloudCover;

/** @brief How much of the sun or moon the clouds leave showing. */
typedef enum
{
    CEL_FULL,   ///< Open sky, the disc draws in full
    CEL_PEEK,   ///< A cloud is parked over it, so it only half shows
    CEL_HIDDEN  ///< Overcast: no disc at all
} CelestialVis;

/** @brief What is falling out of the sky, if anything. */
typedef enum
{
    PRECIP_NONE,
    PRECIP_DROPS,  ///< Fine dots for drizzle
    PRECIP_DASHES, ///< Slanted strokes for rain
    PRECIP_FLAKES, ///< Little crosses for snow
    PRECIP_MIXED   ///< Dots and strokes together for sleet and freezing rain
} PrecipKind;

/**
 * @brief What one condition does to the sky.
 *
 * Every scene agrees on all six fields. Anything a scene does with the condition beyond the sky
 * stays out of here, as the file's header says.
 */
typedef struct SketchbookSky
{
    uint8_t clouds;    ///< A CloudCover
    uint8_t celestial; ///< A CelestialVis
    uint8_t precip;    ///< A PrecipKind
    uint8_t drops;     ///< How many particles fall
    bool    wash;      ///< Lay the scene's wash (its fog, haze or mist) over it
    bool    bolt;      ///< Fork a bolt out of the cloud
} SketchbookSky;

/**
 * @brief Read the phone's condition token.
 *
 * The trailing "_NIGHT" marker is ignored: night is decided from sunrise and sunset, so the
 * same sky reads the same either way.
 *
 * @param condition The condition abbreviation, e.g. "RAIN" or "PCLDY_NIGHT".
 * @return The condition, or WX_COND_CLEAR for an empty or unrecognized token. Never WX_COND_UNKNOWN.
 */
WxCond sketchbook_cond_parse(const char *condition);

/**
 * @brief What that condition does to the sky.
 *
 * @param cond The condition.
 * @return The sky to draw. Out of range reads as open sky.
 */
SketchbookSky sketchbook_sky_recipe(WxCond cond);

/**
 * @brief Whether snow is lying on the ground in this condition.
 *
 * Only snow and snow showers. Sleet and freezing rain are left off, since they do not settle.
 *
 * @param cond The condition.
 * @return True to lay snow on the scene.
 */
bool sketchbook_cond_snows(WxCond cond);

/** @} */
