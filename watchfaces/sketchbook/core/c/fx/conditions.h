/**
 * @file conditions.h
 * @brief The phone's condition token, read into a row and a sky.
 *
 * Every face in the family agrees on what each condition does to the sky: how much cloud, how
 * much of the disc it leaves, what falls and how hard, whether a wash goes over the scene and
 * whether a bolt forks out of it. So that table lives here.
 *
 * What a face does with the row *beyond* the sky is its own. Ridgeline caps its peaks in snow,
 * Shoreline roughens its water, Treeline lies snow along the firs, and those are not the same
 * rows: chop follows wind and snow follows cold. That is exactly why there is no seventh field
 * on SketchbookSky to hold it. A face keeps its own rule as a mask over SketchbookCond and asserts it, so a
 * table copied between faces cannot quietly bring the wrong meaning with it.
 *
 * @ingroup family-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup family-sketchbook
 * @{
 */

/**
 * @brief The condition vocabulary, in the order lib/ts/weather/conditions.ts lists it.
 *
 * A face indexes its own per-condition rules off these, so the values are the shared contract.
 */
typedef enum
{
    SKETCHBOOK_COND_CLEAR = 0,
    SKETCHBOOK_COND_PCLDY,
    SKETCHBOOK_COND_CLDY,
    SKETCHBOOK_COND_FOGGY,
    SKETCHBOOK_COND_DRZL,
    SKETCHBOOK_COND_FZDZ,
    SKETCHBOOK_COND_RAIN,
    SKETCHBOOK_COND_FZRN,
    SKETCHBOOK_COND_SNOW,
    SKETCHBOOK_COND_SHWR,
    SKETCHBOOK_COND_SNSH,
    SKETCHBOOK_COND_STRM,
    SKETCHBOOK_COND_COUNT
} SketchbookCond;

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
 * Six fields, and every face in the family agrees on all six. There is deliberately no seventh:
 * see the file's header for why.
 */
typedef struct SketchbookSky
{
    uint8_t clouds;    ///< A CloudCover
    uint8_t celestial; ///< A CelestialVis
    uint8_t precip;    ///< A PrecipKind
    uint8_t drops;     ///< How many particles fall
    bool    wash;      ///< Lay the face's wash (its fog, haze or mist) over the scene
    bool    bolt;      ///< Fork a bolt out of the cloud
} SketchbookSky;

/**
 * @brief Read the phone's condition token into a row.
 *
 * The trailing "_NIGHT" marker is ignored: night is decided from sunrise and sunset, so the
 * same sky reads the same either way.
 *
 * @param condition The condition abbreviation, e.g. "RAIN" or "PCLDY_NIGHT".
 * @return The row, or SKETCHBOOK_COND_CLEAR for an empty or unrecognised token.
 */
SketchbookCond sketchbook_cond_parse(const char *condition);

/**
 * @brief What that row does to the sky.
 *
 * @param cond The row.
 * @return The sky to draw. Out of range reads as open sky.
 */
SketchbookSky sketchbook_sky_recipe(SketchbookCond cond);

/** @} */
